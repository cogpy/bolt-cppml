#ifndef BOLT_DRAWKERN_AI_MODEL_HPP_
#define BOLT_DRAWKERN_AI_MODEL_HPP_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace bolt {
namespace drawkern {

// The configuration, request and response types shared by every DrawKern AI model, and the interface the
// models implement. The implementations are in ggml_model.hpp and rwkv_model.hpp.

struct AIModelConfig {
    std::string model_type;  // "ggml", "rwkv", "llama"
    std::string model_path;  // Path to model file
    std::string model_name;  // Human-readable name

    // Model parameters
    int32_t context_length = 2048;
    int32_t vocab_size = 50277;
    int32_t n_layers = 12;
    int32_t n_head = 12;
    int32_t n_embd = 768;

    // Inference parameters
    float temperature = 0.8f;
    float top_p = 0.95f;
    int32_t top_k = 40;
    int32_t max_tokens = 256;

    // Hardware settings
    int32_t n_threads = 4;
    bool use_mmap = true;
    bool use_mlock = false;
    bool use_gpu = false;

    // DrawKern specific
    bool enable_chat = true;
    bool enable_completion = true;
    bool enable_analysis = true;
};

struct AIInferenceRequest {
    std::string prompt;
    std::string context;
    std::string task_type;  // "chat", "completion", "analysis"

    // Override parameters for this request
    std::map<std::string, float> parameters;

    // DrawKern integration
    std::string vm_id;       // Requesting VM
    std::string session_id;  // Session for context
};

struct AIInferenceResponse {
    std::string response;
    std::string error;
    bool success = false;

    // Metadata
    float inference_time_ms = 0.0f;
    int32_t tokens_generated = 0;
    int32_t tokens_processed = 0;

    // DrawKern integration
    std::string vm_id;
    std::string session_id;
};

// Base class for AI models
class AIModel {
 public:
    explicit AIModel(const AIModelConfig& config);
    virtual ~AIModel();

    // Model lifecycle
    virtual bool load() = 0;
    virtual void unload() = 0;
    virtual bool is_loaded() const = 0;

    // Inference
    virtual AIInferenceResponse generate(const AIInferenceRequest& request) = 0;
    virtual AIInferenceResponse chat(const std::string& message, const std::string& context = "") = 0;
    virtual AIInferenceResponse complete_code(const std::string& code, const std::string& language = "cpp") = 0;

    // Model info
    virtual std::string get_model_info() const = 0;
    virtual std::vector<std::string> get_capabilities() const = 0;

    const AIModelConfig& get_config() const { return config_; }

 protected:
    AIModelConfig config_;
    bool loaded_ = false;
};

}  // namespace drawkern
}  // namespace bolt

#endif  // BOLT_DRAWKERN_AI_MODEL_HPP_
