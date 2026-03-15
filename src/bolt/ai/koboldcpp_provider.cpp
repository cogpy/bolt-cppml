#include "bolt/ai/koboldcpp_provider.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <cstring>

#ifdef BOLT_HAVE_CURL
#include <curl/curl.h>
#endif

#ifdef BOLT_HAVE_JSONCPP
#include <json/json.h>
#endif

namespace bolt {
namespace ai {

// ── CURL write callback ────────────────────────────────────────
#ifdef BOLT_HAVE_CURL
static size_t KoboldWriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total = size * nmemb;
    userp->append(static_cast<char*>(contents), total);
    return total;
}
#endif

// ── Constructors / Destructor ──────────────────────────────────

KoboldCppProvider::KoboldCppProvider() = default;

KoboldCppProvider::KoboldCppProvider(const KoboldCppConfig& config)
    : config_(config) {}

KoboldCppProvider::KoboldCppProvider(const std::string& endpoint) {
    config_.endpoint = endpoint;
}

KoboldCppProvider::~KoboldCppProvider() {
    shutdown();
}

// ── AICompletionProvider interface ─────────────────────────────

bool KoboldCppProvider::isReady() const {
    return initialized_;
}

bool KoboldCppProvider::initialize() {
    if (initialized_) return true;
    
    // Test connection and detect API mode
    if (test_connection()) {
        if (config_.api_mode == KoboldCppConfig::APIMode::AUTO_DETECT) {
            detect_api_mode();
        } else {
            detected_mode_ = config_.api_mode;
        }
        initialized_ = true;
        return true;
    }
    return false;
}

void KoboldCppProvider::shutdown() {
    initialized_ = false;
}

std::vector<CompletionItem> KoboldCppProvider::generateCompletions(
    const CodeContext& context,
    const std::string& prefix,
    size_t maxSuggestions)
{
    if (!initialized_) {
        return {};
    }
    
    std::string prompt = build_code_prompt(context, prefix);
    auto response = complete(prompt, std::min(config_.max_tokens, 100));
    
    if (!response.success || response.text.empty()) {
        return {};
    }
    
    return parse_completions(response.text, maxSuggestions);
}

// ── KoboldCpp-specific: completion ─────────────────────────────

KoboldCppResponse KoboldCppProvider::complete(const std::string& prompt) {
    return complete(prompt, config_.max_tokens);
}

KoboldCppResponse KoboldCppProvider::complete(const std::string& prompt, int max_tokens) {
    auto mode = (config_.api_mode == KoboldCppConfig::APIMode::AUTO_DETECT)
        ? detected_mode_ : config_.api_mode;
    
    if (mode == KoboldCppConfig::APIMode::KOBOLDAI_NATIVE) {
        return send_koboldai_generate(prompt, max_tokens);
    }
    return send_openai_completion(prompt, max_tokens);
}

// ── KoboldCpp-specific: chat ───────────────────────────────────

KoboldCppResponse KoboldCppProvider::chat(const std::string& message) {
    return chat(message, "You are a helpful AI programming assistant.");
}

KoboldCppResponse KoboldCppProvider::chat(const std::string& message, const std::string& system_prompt) {
    std::vector<std::pair<std::string,std::string>> messages;
    messages.push_back({"system", system_prompt});
    messages.push_back({"user", message});
    return chat(messages);
}

KoboldCppResponse KoboldCppProvider::chat(const std::vector<std::pair<std::string,std::string>>& messages) {
    auto mode = (config_.api_mode == KoboldCppConfig::APIMode::AUTO_DETECT)
        ? detected_mode_ : config_.api_mode;
    
    if (mode == KoboldCppConfig::APIMode::KOBOLDAI_NATIVE) {
        // Build a prompt from messages for the native API
        std::ostringstream prompt;
        for (const auto& msg : messages) {
            if (msg.first == "system") {
                prompt << msg.second << "\n\n";
            } else if (msg.first == "user") {
                prompt << "User: " << msg.second << "\n";
            } else if (msg.first == "assistant") {
                prompt << "Assistant: " << msg.second << "\n";
            }
        }
        prompt << "Assistant: ";
        return send_koboldai_generate(prompt.str(), config_.max_tokens);
    }
    return send_openai_chat(messages);
}

// ── Streaming ──────────────────────────────────────────────────

bool KoboldCppProvider::complete_streaming(const std::string& prompt, StreamCallback callback) {
    // For now, simulate streaming by getting the full response and chunking it
    auto response = complete(prompt);
    if (!response.success) return false;
    
    // Deliver token by token
    std::istringstream stream(response.text);
    std::string word;
    while (stream >> word) {
        if (!callback(word + " ")) return false;
    }
    return true;
}

bool KoboldCppProvider::chat_streaming(const std::string& message, StreamCallback callback) {
    auto response = chat(message);
    if (!response.success) return false;
    
    std::istringstream stream(response.text);
    std::string word;
    while (stream >> word) {
        if (!callback(word + " ")) return false;
    }
    return true;
}

// ── Server health ──────────────────────────────────────────────

bool KoboldCppProvider::test_connection() {
    std::string url = config_.endpoint + "/api/extra/version";
    std::string response = http_get(url);
    return !response.empty() && response.find("error") == std::string::npos;
}

KoboldCppServerInfo KoboldCppProvider::get_server_info() {
    KoboldCppServerInfo info;
    
    // Try /api/extra/version
    std::string version_response = http_get(config_.endpoint + "/api/extra/version");
    if (!version_response.empty()) {
        info.connected = true;
#ifdef BOLT_HAVE_JSONCPP
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::istringstream s(version_response);
        std::string errors;
        if (Json::parseFromStream(builder, s, &root, &errors)) {
            info.version = root.get("result", "unknown").asString();
        }
#endif
    }
    
    // Try /api/extra/true_max_context_length
    std::string ctx_response = http_get(config_.endpoint + "/api/extra/true_max_context_length");
    if (!ctx_response.empty()) {
#ifdef BOLT_HAVE_JSONCPP
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::istringstream s(ctx_response);
        std::string errors;
        if (Json::parseFromStream(builder, s, &root, &errors)) {
            info.max_context_length = root.get("value", 0).asInt();
        }
#endif
    }
    
    // Try /api/v1/model
    std::string model_response = http_get(config_.endpoint + "/api/v1/model");
    if (!model_response.empty()) {
#ifdef BOLT_HAVE_JSONCPP
        Json::Value root;
        Json::CharReaderBuilder builder;
        std::istringstream s(model_response);
        std::string errors;
        if (Json::parseFromStream(builder, s, &root, &errors)) {
            info.model_name = root.get("result", "unknown").asString();
        }
#endif
    }
    
    return info;
}

bool KoboldCppProvider::is_model_loaded() {
    auto info = get_server_info();
    return info.connected && !info.model_name.empty() && info.model_name != "unknown";
}

// ── Configuration ──────────────────────────────────────────────

void KoboldCppProvider::set_config(const KoboldCppConfig& config) {
    config_ = config;
    initialized_ = false; // Force re-initialization
}

void KoboldCppProvider::set_endpoint(const std::string& endpoint) {
    config_.endpoint = endpoint;
    initialized_ = false;
}

// ── Internal: OpenAI-compatible API ────────────────────────────

KoboldCppResponse KoboldCppProvider::send_openai_completion(const std::string& prompt, int max_tokens) {
    KoboldCppResponse result;
    auto start = std::chrono::high_resolution_clock::now();
    
    std::ostringstream json;
    json << "{\"prompt\":\"";
    // Escape the prompt for JSON
    for (char c : prompt) {
        switch (c) {
            case '"':  json << "\\\""; break;
            case '\\': json << "\\\\"; break;
            case '\n': json << "\\n"; break;
            case '\r': json << "\\r"; break;
            case '\t': json << "\\t"; break;
            default:   json << c; break;
        }
    }
    json << "\",\"max_tokens\":" << max_tokens
         << ",\"temperature\":" << config_.temperature
         << ",\"top_p\":" << config_.top_p;
    
    if (!config_.stop_sequences.empty()) {
        json << ",\"stop\":[";
        for (size_t i = 0; i < config_.stop_sequences.size(); ++i) {
            if (i > 0) json << ",";
            json << "\"" << config_.stop_sequences[i] << "\"";
        }
        json << "]";
    }
    json << "}";
    
    std::string url = config_.endpoint + "/v1/completions";
    std::string response_str = http_post(url, json.str());
    
    auto end = std::chrono::high_resolution_clock::now();
    result.latency_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    if (response_str.empty()) {
        result.success = false;
        result.error = "Empty response from server";
        return result;
    }
    
#ifdef BOLT_HAVE_JSONCPP
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::istringstream s(response_str);
    std::string errors;
    if (Json::parseFromStream(builder, s, &root, &errors)) {
        if (root.isMember("choices") && root["choices"].isArray() && root["choices"].size() > 0) {
            result.text = root["choices"][0].get("text", "").asString();
            result.success = true;
        }
        if (root.isMember("usage")) {
            result.tokens_used = root["usage"].get("total_tokens", 0).asInt();
        }
        if (root.isMember("model")) {
            result.model_name = root["model"].asString();
        }
    } else {
        result.error = "JSON parse error: " + errors;
    }
#else
    // Simple text extraction without jsoncpp
    auto pos = response_str.find("\"text\":\"");
    if (pos != std::string::npos) {
        pos += 8;
        auto end_pos = response_str.find("\"", pos);
        if (end_pos != std::string::npos) {
            result.text = response_str.substr(pos, end_pos - pos);
            result.success = true;
        }
    }
    if (!result.success) {
        result.error = "No jsoncpp available and simple parse failed";
    }
#endif
    
    return result;
}

KoboldCppResponse KoboldCppProvider::send_openai_chat(
    const std::vector<std::pair<std::string,std::string>>& messages)
{
    KoboldCppResponse result;
    auto start = std::chrono::high_resolution_clock::now();
    
    std::ostringstream json;
    json << "{\"messages\":[";
    for (size_t i = 0; i < messages.size(); ++i) {
        if (i > 0) json << ",";
        json << "{\"role\":\"" << messages[i].first << "\",\"content\":\"";
        for (char c : messages[i].second) {
            switch (c) {
                case '"':  json << "\\\""; break;
                case '\\': json << "\\\\"; break;
                case '\n': json << "\\n"; break;
                case '\r': json << "\\r"; break;
                case '\t': json << "\\t"; break;
                default:   json << c; break;
            }
        }
        json << "\"}";
    }
    json << "],\"max_tokens\":" << config_.max_tokens
         << ",\"temperature\":" << config_.temperature
         << "}";
    
    std::string url = config_.endpoint + "/v1/chat/completions";
    std::string response_str = http_post(url, json.str());
    
    auto end = std::chrono::high_resolution_clock::now();
    result.latency_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    if (response_str.empty()) {
        result.success = false;
        result.error = "Empty response from server";
        return result;
    }
    
#ifdef BOLT_HAVE_JSONCPP
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::istringstream s(response_str);
    std::string errors;
    if (Json::parseFromStream(builder, s, &root, &errors)) {
        if (root.isMember("choices") && root["choices"].isArray() && root["choices"].size() > 0) {
            auto& choice = root["choices"][0];
            if (choice.isMember("message")) {
                result.text = choice["message"].get("content", "").asString();
            } else {
                result.text = choice.get("text", "").asString();
            }
            result.success = true;
        }
        if (root.isMember("model")) {
            result.model_name = root["model"].asString();
        }
    } else {
        result.error = "JSON parse error: " + errors;
    }
#else
    auto pos = response_str.find("\"content\":\"");
    if (pos != std::string::npos) {
        pos += 11;
        auto end_pos = response_str.find("\"", pos);
        if (end_pos != std::string::npos) {
            result.text = response_str.substr(pos, end_pos - pos);
            result.success = true;
        }
    }
#endif
    
    return result;
}

// ── Internal: KoboldAI native API ──────────────────────────────

KoboldCppResponse KoboldCppProvider::send_koboldai_generate(const std::string& prompt, int max_tokens) {
    KoboldCppResponse result;
    auto start = std::chrono::high_resolution_clock::now();
    
    std::ostringstream json;
    json << "{\"prompt\":\"";
    for (char c : prompt) {
        switch (c) {
            case '"':  json << "\\\""; break;
            case '\\': json << "\\\\"; break;
            case '\n': json << "\\n"; break;
            case '\r': json << "\\r"; break;
            case '\t': json << "\\t"; break;
            default:   json << c; break;
        }
    }
    json << "\",\"max_context_length\":" << config_.max_context_length
         << ",\"max_length\":" << max_tokens
         << ",\"temperature\":" << config_.temperature
         << ",\"top_k\":" << config_.top_k
         << ",\"top_p\":" << config_.top_p
         << ",\"rep_pen\":" << config_.rep_pen
         << ",\"rep_pen_range\":" << config_.rep_pen_range
         << "}";
    
    std::string url = config_.endpoint + "/api/v1/generate";
    std::string response_str = http_post(url, json.str());
    
    auto end = std::chrono::high_resolution_clock::now();
    result.latency_ms = std::chrono::duration<double, std::milli>(end - start).count();
    
    if (response_str.empty()) {
        result.success = false;
        result.error = "Empty response from server";
        return result;
    }
    
#ifdef BOLT_HAVE_JSONCPP
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::istringstream s(response_str);
    std::string errors;
    if (Json::parseFromStream(builder, s, &root, &errors)) {
        if (root.isMember("results") && root["results"].isArray() && root["results"].size() > 0) {
            result.text = root["results"][0].get("text", "").asString();
            result.success = true;
        }
    } else {
        result.error = "JSON parse error: " + errors;
    }
#else
    auto pos = response_str.find("\"text\":\"");
    if (pos != std::string::npos) {
        pos += 8;
        auto end_pos = response_str.find("\"", pos);
        if (end_pos != std::string::npos) {
            result.text = response_str.substr(pos, end_pos - pos);
            result.success = true;
        }
    }
#endif
    
    return result;
}

// ── Internal: HTTP helpers ─────────────────────────────────────

std::string KoboldCppProvider::http_post(const std::string& url, const std::string& json_body) {
#ifdef BOLT_HAVE_CURL
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    
    std::string response;
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_body.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, KoboldWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, static_cast<long>(config_.timeout_seconds));
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        return "";
    }
    return response;
#else
    (void)url;
    (void)json_body;
    return ""; // No CURL support
#endif
}

std::string KoboldCppProvider::http_get(const std::string& url) {
#ifdef BOLT_HAVE_CURL
    CURL* curl = curl_easy_init();
    if (!curl) return "";
    
    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, KoboldWriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 3L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) return "";
    return response;
#else
    (void)url;
    return "";
#endif
}

// ── Internal: Code completion helpers ──────────────────────────

std::string KoboldCppProvider::build_code_prompt(const CodeContext& context, const std::string& prefix) const {
    std::ostringstream prompt;
    prompt << "// Language: " << context.language << "\n";
    prompt << "// File: " << context.filePath << "\n";
    
    // Include surrounding context (up to 500 chars before cursor)
    size_t start = (context.cursorPosition > 500) ? context.cursorPosition - 500 : 0;
    std::string before = context.content.substr(start, context.cursorPosition - start);
    
    prompt << before << prefix;
    return prompt.str();
}

std::vector<CompletionItem> KoboldCppProvider::parse_completions(
    const std::string& response_text, size_t max) const
{
    std::vector<CompletionItem> items;
    
    // Split response into lines and create completion items
    std::istringstream stream(response_text);
    std::string line;
    size_t count = 0;
    
    while (std::getline(stream, line) && count < max) {
        // Trim whitespace
        size_t start = line.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) continue;
        line = line.substr(start);
        
        if (line.empty()) continue;
        
        CompletionItem item;
        item.label = line.substr(0, std::min(line.size(), size_t(60)));
        item.detail = "KoboldCpp AI suggestion";
        item.kind = infer_kind(line);
        item.score = 100 - static_cast<int>(count);
        items.push_back(item);
        count++;
    }
    
    // If no line breaks, treat the whole response as one completion
    if (items.empty() && !response_text.empty()) {
        CompletionItem item;
        item.label = response_text.substr(0, std::min(response_text.size(), size_t(60)));
        item.detail = "KoboldCpp AI suggestion";
        item.kind = infer_kind(response_text);
        item.score = 100;
        items.push_back(item);
    }
    
    return items;
}

std::string KoboldCppProvider::infer_kind(const std::string& text) const {
    if (text.find("(") != std::string::npos && text.find(")") != std::string::npos) {
        return "function";
    }
    if (text.find("class ") != std::string::npos || text.find("struct ") != std::string::npos) {
        return "class";
    }
    if (text.find("=") != std::string::npos || text.find("int ") != std::string::npos ||
        text.find("auto ") != std::string::npos) {
        return "variable";
    }
    return "text";
}

// ── Auto-detection ─────────────────────────────────────────────

bool KoboldCppProvider::detect_api_mode() {
    // Try OpenAI-compatible first (more feature-rich)
    std::string test = http_get(config_.endpoint + "/v1/models");
    if (!test.empty() && test.find("error") == std::string::npos) {
        detected_mode_ = KoboldCppConfig::APIMode::OPENAI_COMPATIBLE;
        return true;
    }
    
    // Fall back to KoboldAI native
    test = http_get(config_.endpoint + "/api/v1/model");
    if (!test.empty() && test.find("error") == std::string::npos) {
        detected_mode_ = KoboldCppConfig::APIMode::KOBOLDAI_NATIVE;
        return true;
    }
    
    // Default to OpenAI-compatible
    detected_mode_ = KoboldCppConfig::APIMode::OPENAI_COMPATIBLE;
    return false;
}

} // namespace ai
} // namespace bolt
