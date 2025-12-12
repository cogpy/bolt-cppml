#include "bolt/ai/rwkv_wrapper.hpp"
#include <iostream>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <cstring>

namespace bolt {

void RWKVWrapper::initialize(const std::string& model_path, int ctx_size) {
    model_path_ = model_path;
    
    std::cout << "Initializing RWKV model from: " << model_path << std::endl;
    
    // Create GGUF loader
    gguf_loader_ = std::make_unique<GGUFLoader>(model_path);
    
    // Load the GGUF file
    if (!gguf_loader_->load()) {
        throw std::runtime_error("Failed to load GGUF file: " + gguf_loader_->getError());
    }
    
    std::cout << "GGUF file loaded successfully" << std::endl;
    std::cout << "  Version: " << gguf_loader_->getVersion() << std::endl;
    std::cout << "  Architecture: " << gguf_loader_->getArchitecture() << std::endl;
    
    // Extract model parameters
    n_layers_ = gguf_loader_->getNumLayers();
    n_embd_ = gguf_loader_->getEmbedDim();
    n_vocab_ = gguf_loader_->getVocabSize();
    
    if (n_layers_ == 0 || n_embd_ == 0 || n_vocab_ == 0) {
        throw std::runtime_error("Invalid model parameters: n_layers=" + std::to_string(n_layers_) +
                               ", n_embd=" + std::to_string(n_embd_) +
                               ", n_vocab=" + std::to_string(n_vocab_));
    }
    
    std::cout << "Model parameters:" << std::endl;
    std::cout << "  Layers: " << n_layers_ << std::endl;
    std::cout << "  Embedding dim: " << n_embd_ << std::endl;
    std::cout << "  Vocabulary size: " << n_vocab_ << std::endl;
    
    // Initialize tokenizer
    tokenizer_ = std::make_unique<BPETokenizer>();
    auto tokens = gguf_loader_->getMetadataStringArray("tokenizer.ggml.tokens");
    auto scores = gguf_loader_->getMetadataFloatArray("tokenizer.ggml.scores");
    
    if (tokens.empty()) {
        throw std::runtime_error("No tokenizer vocabulary found in model file");
    }
    
    if (!tokenizer_->loadVocabulary(tokens, scores)) {
        throw std::runtime_error("Failed to load tokenizer vocabulary");
    }
    
    // Calculate required memory
    size_t mem_size = ctx_size * 1024 * 1024; // Convert MB to bytes
    
    // Create GGML context
    context_ = std::make_unique<GGMLContext>(mem_size);
    
    // Load model weights
    loadModel(model_path);
    
    // Initialize state
    state_ = std::make_unique<RWKVState>(context_->get(), n_layers_, n_embd_);
    
    // Initialize random number generator
    std::random_device rd;
    rng_ = std::mt19937(rd());
    
    model_loaded_ = true;
    std::cout << "RWKV model initialized successfully" << std::endl;
}

void RWKVWrapper::loadModel(const std::string& path) {
    std::cout << "Loading model weights..." << std::endl;
    
    // Get list of all tensors
    auto tensor_names = gguf_loader_->getTensorNames();
    std::cout << "Found " << tensor_names.size() << " tensors in model file" << std::endl;
    
    // Load each tensor
    int loaded_count = 0;
    for (const auto& name : tensor_names) {
        ggml_tensor* tensor = gguf_loader_->loadTensor(context_->get(), name);
        if (tensor) {
            weights_[name] = tensor;
            loaded_count++;
        } else {
            std::cerr << "Warning: Failed to load tensor: " << name << std::endl;
        }
    }
    
    std::cout << "Loaded " << loaded_count << " / " << tensor_names.size() << " tensors" << std::endl;
    
    // Verify essential weights are present
    std::vector<std::string> essential_weights = {
        "token_embd.weight",
        "output.weight"
    };
    
    for (const auto& weight_name : essential_weights) {
        if (weights_.find(weight_name) == weights_.end()) {
            // Try alternative names
            if (weight_name == "token_embd.weight" && weights_.find("emb.weight") != weights_.end()) {
                weights_["token_embd.weight"] = weights_["emb.weight"];
            } else if (weight_name == "output.weight" && weights_.find("head.weight") != weights_.end()) {
                weights_["output.weight"] = weights_["head.weight"];
            } else {
                std::cerr << "Warning: Essential weight not found: " << weight_name << std::endl;
            }
        }
    }
}

std::vector<int> RWKVWrapper::tokenize(const std::string& text) const {
    if (tokenizer_ && tokenizer_->isLoaded()) {
        return tokenizer_->encode(text);
    }
    
    // Fallback to character-level tokenization
    std::vector<int> tokens;
    for (char c : text) {
        tokens.push_back(static_cast<int>(static_cast<unsigned char>(c)));
    }
    return tokens;
}

std::string RWKVWrapper::detokenize(const std::vector<int>& tokens) const {
    if (tokenizer_ && tokenizer_->isLoaded()) {
        return tokenizer_->decode(tokens);
    }
    
    // Fallback to character-level detokenization
    std::string text;
    for (int token : tokens) {
        if (token >= 0 && token < 256) {
            text += static_cast<char>(token);
        }
    }
    return text;
}

std::string RWKVWrapper::generate(const std::string& prompt, size_t max_tokens) {
    return generateImproved(prompt, max_tokens, 1.0f, 0.95f);
}

std::string RWKVWrapper::generateImproved(const std::string& prompt, size_t max_tokens,
                                         float temperature, float top_p) {
    if (!model_loaded_) {
        throw std::runtime_error("RWKV model not initialized");
    }
    
    // Tokenize the prompt
    auto prompt_tokens = tokenize(prompt);
    
    if (prompt_tokens.empty()) {
        throw std::runtime_error("Empty prompt after tokenization");
    }
    
    std::cout << "Generating " << max_tokens << " tokens (prompt: " 
              << prompt_tokens.size() << " tokens)" << std::endl;
    
    std::vector<int> generated_tokens = prompt_tokens;
    
    // Generation loop
    for (size_t i = 0; i < max_tokens; ++i) {
        // Create input tensor from last token
        int last_token = generated_tokens.back();
        auto* input = ggml_new_tensor_1d(context_->get(), GGML_TYPE_I32, 1);
        memcpy(input->data, &last_token, sizeof(int));
        
        // Forward pass through the model
        auto* logits = forwardWithLogits(input);
        
        if (!logits) {
            std::cerr << "Forward pass failed at token " << i << std::endl;
            break;
        }
        
        // Sample next token
        int next_token = sampleToken(logits, temperature, top_p);
        
        // Check for end of sequence
        if (tokenizer_ && (next_token == tokenizer_->getEOSToken() || 
                          next_token == tokenizer_->getPADToken())) {
            std::cout << "EOS token generated at position " << i << std::endl;
            break;
        }
        
        generated_tokens.push_back(next_token);
        
        // Progress indicator
        if ((i + 1) % 10 == 0) {
            std::cout << "." << std::flush;
        }
    }
    
    std::cout << std::endl;
    
    // Remove prompt tokens and return only generated text
    std::vector<int> output_tokens(generated_tokens.begin() + prompt_tokens.size(), 
                                   generated_tokens.end());
    return detokenize(output_tokens);
}

ggml_tensor* RWKVWrapper::forwardWithLogits(ggml_tensor* input) {
    if (!model_loaded_) {
        throw std::runtime_error("RWKV model not initialized");
    }
    
    // Get input embeddings
    auto* current = getEmbeddings(input);
    
    if (!current) {
        std::cerr << "Failed to get embeddings" << std::endl;
        return nullptr;
    }
    
    // Process through RWKV layers
    for (int i = 0; i < n_layers_; i++) {
        current = rwkvLayer(current, i);
        if (!current) {
            std::cerr << "Failed at layer " << i << std::endl;
            return nullptr;
        }
    }
    
    // Final layer norm
    current = layerNorm(current, "ln_out.weight", "ln_out.bias");
    
    // Project to vocabulary logits
    auto* logits = projectToVocab(current);
    
    return logits;
}

ggml_tensor* RWKVWrapper::getEmbeddings(ggml_tensor* input) {
    // Look for embedding weight
    ggml_tensor* emb_weight = nullptr;
    
    if (weights_.find("token_embd.weight") != weights_.end()) {
        emb_weight = weights_["token_embd.weight"];
    } else if (weights_.find("emb.weight") != weights_.end()) {
        emb_weight = weights_["emb.weight"];
    }
    
    if (!emb_weight) {
        std::cerr << "Embedding weight not found" << std::endl;
        return nullptr;
    }
    
    // Use GGML's get_rows operation to extract embeddings
    return ggml_get_rows(context_->get(), emb_weight, input);
}

ggml_tensor* RWKVWrapper::projectToVocab(ggml_tensor* hidden) {
    // Look for output weight
    ggml_tensor* output_weight = nullptr;
    
    if (weights_.find("output.weight") != weights_.end()) {
        output_weight = weights_["output.weight"];
    } else if (weights_.find("head.weight") != weights_.end()) {
        output_weight = weights_["head.weight"];
    } else if (weights_.find("token_embd.weight") != weights_.end()) {
        // Weight tying: use embedding weights
        output_weight = weights_["token_embd.weight"];
    }
    
    if (!output_weight) {
        std::cerr << "Output weight not found" << std::endl;
        return nullptr;
    }
    
    // Matrix multiplication: hidden @ output_weight^T
    return ggml_mul_mat(context_->get(), output_weight, hidden);
}

ggml_tensor* RWKVWrapper::rwkvLayer(ggml_tensor* x, int layer_idx) {
    // Time-mixing (attention-like mechanism)
    std::string ln1_weight = "blk." + std::to_string(layer_idx) + ".ln1.weight";
    std::string ln1_bias = "blk." + std::to_string(layer_idx) + ".ln1.bias";
    
    auto* x_norm = layerNorm(x, ln1_weight, ln1_bias);
    auto* tm_out = timeMixing(x_norm, layer_idx);
    
    // Residual connection
    auto* x1 = ggml_add(context_->get(), x, tm_out);
    
    // Channel-mixing (FFN-like mechanism)
    std::string ln2_weight = "blk." + std::to_string(layer_idx) + ".ln2.weight";
    std::string ln2_bias = "blk." + std::to_string(layer_idx) + ".ln2.bias";
    
    auto* x1_norm = layerNorm(x1, ln2_weight, ln2_bias);
    auto* cm_out = channelMixing(x1_norm, layer_idx);
    
    // Residual connection
    auto* output = ggml_add(context_->get(), x1, cm_out);
    
    return output;
}

ggml_tensor* RWKVWrapper::timeMixing(ggml_tensor* x, int layer_idx) {
    // This is a simplified placeholder
    // Real RWKV time-mixing involves complex operations with state
    
    std::string prefix = "blk." + std::to_string(layer_idx) + ".att.";
    
    // For now, just return the input (placeholder)
    // TODO: Implement proper RWKV time-mixing with WKV operation
    return x;
}

ggml_tensor* RWKVWrapper::channelMixing(ggml_tensor* x, int layer_idx) {
    // This is a simplified placeholder
    // Real RWKV channel-mixing involves key, value, and receptance operations
    
    std::string prefix = "blk." + std::to_string(layer_idx) + ".ffn.";
    
    // For now, just return the input (placeholder)
    // TODO: Implement proper RWKV channel-mixing
    return x;
}

ggml_tensor* RWKVWrapper::layerNorm(ggml_tensor* x, const std::string& weight_name, 
                                    const std::string& bias_name) {
    ggml_tensor* weight = weights_.find(weight_name) != weights_.end() ? 
                         weights_[weight_name] : nullptr;
    ggml_tensor* bias = weights_.find(bias_name) != weights_.end() ? 
                       weights_[bias_name] : nullptr;
    
    if (!weight) {
        // If no weight, just return input
        return x;
    }
    
    // Use GGML's layer norm operation
    auto* normed = ggml_norm(context_->get(), x, 1e-5f);
    
    // Scale by weight
    normed = ggml_mul(context_->get(), normed, weight);
    
    // Add bias if present
    if (bias) {
        normed = ggml_add(context_->get(), normed, bias);
    }
    
    return normed;
}

int RWKVWrapper::sampleToken(ggml_tensor* logits, float temperature, float top_p) {
    // Get logits data
    float* logits_data = ggml_get_data_f32(logits);
    size_t n_logits = ggml_nelements(logits);
    
    if (n_logits != static_cast<size_t>(n_vocab_)) {
        std::cerr << "Logits size mismatch: " << n_logits << " vs " << n_vocab_ << std::endl;
        return 0;
    }
    
    // Apply temperature
    if (temperature != 1.0f && temperature > 0.0f) {
        for (size_t i = 0; i < n_logits; i++) {
            logits_data[i] /= temperature;
        }
    }
    
    // Compute softmax
    float max_logit = *std::max_element(logits_data, logits_data + n_logits);
    float sum = 0.0f;
    std::vector<float> probs(n_logits);
    
    for (size_t i = 0; i < n_logits; i++) {
        probs[i] = std::exp(logits_data[i] - max_logit);
        sum += probs[i];
    }
    
    for (size_t i = 0; i < n_logits; i++) {
        probs[i] /= sum;
    }
    
    // Top-p sampling
    if (top_p < 1.0f) {
        // Create index-probability pairs
        std::vector<std::pair<float, int>> prob_idx;
        for (size_t i = 0; i < n_logits; i++) {
            prob_idx.push_back({probs[i], static_cast<int>(i)});
        }
        
        // Sort by probability (descending)
        std::sort(prob_idx.begin(), prob_idx.end(), 
                 [](const auto& a, const auto& b) { return a.first > b.first; });
        
        // Compute cumulative probability and find cutoff
        float cumsum = 0.0f;
        size_t cutoff = 0;
        for (size_t i = 0; i < prob_idx.size(); i++) {
            cumsum += prob_idx[i].first;
            cutoff = i + 1;
            if (cumsum >= top_p) {
                break;
            }
        }
        
        // Zero out probabilities below cutoff
        std::fill(probs.begin(), probs.end(), 0.0f);
        for (size_t i = 0; i < cutoff; i++) {
            probs[prob_idx[i].second] = prob_idx[i].first;
        }
        
        // Renormalize
        sum = 0.0f;
        for (float p : probs) sum += p;
        for (float& p : probs) p /= sum;
    }
    
    // Sample from distribution
    std::discrete_distribution<int> dist(probs.begin(), probs.end());
    return dist(rng_);
}

ggml_tensor* RWKVWrapper::forward(ggml_tensor* input) {
    return forwardWithLogits(input);
}

} // namespace bolt
