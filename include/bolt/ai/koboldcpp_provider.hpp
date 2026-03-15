#ifndef BOLT_AI_KOBOLDCPP_PROVIDER_HPP
#define BOLT_AI_KOBOLDCPP_PROVIDER_HPP

#include "bolt/ai/ai_completion_provider.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>

namespace bolt {
namespace ai {

/**
 * KoboldCpp server connection configuration.
 *
 * Supports both the OpenAI-compatible API (/v1/completions, /v1/chat/completions)
 * and the KoboldAI native API (/api/v1/generate).
 */
struct KoboldCppConfig {
    std::string endpoint = "http://localhost:5001";
    
    // Generation parameters
    float temperature = 0.7f;
    float top_p = 0.9f;
    int top_k = 40;
    float rep_pen = 1.1f;
    int rep_pen_range = 512;
    int max_tokens = 200;
    int max_context_length = 4096;
    std::vector<std::string> stop_sequences;
    
    // Connection settings
    int timeout_seconds = 30;
    int max_retries = 3;
    int retry_delay_ms = 500;
    bool use_streaming = false;
    
    // API selection
    enum class APIMode {
        OPENAI_COMPATIBLE,  // /v1/completions, /v1/chat/completions
        KOBOLDAI_NATIVE,    // /api/v1/generate
        AUTO_DETECT         // Try OpenAI first, fall back to KoboldAI
    };
    APIMode api_mode = APIMode::AUTO_DETECT;
};

/**
 * Response from KoboldCpp server.
 */
struct KoboldCppResponse {
    bool success = false;
    std::string text;
    std::string error;
    int tokens_used = 0;
    double latency_ms = 0.0;
    std::string model_name;
};

/**
 * KoboldCpp server information.
 */
struct KoboldCppServerInfo {
    bool connected = false;
    std::string version;
    std::string model_name;
    int max_context_length = 0;
    int max_length = 0;
    bool has_image_gen = false;
    bool has_whisper = false;
    bool has_tts = false;
};

/**
 * Streaming callback type.
 * Called with each token as it arrives. Return false to cancel generation.
 */
using StreamCallback = std::function<bool(const std::string& token)>;

/**
 * KoboldCpp AI completion provider.
 *
 * Connects to a running KoboldCpp server via HTTP API to provide
 * AI-powered code completions, chat, and text generation.
 *
 * Supports:
 * - OpenAI-compatible API (/v1/completions, /v1/chat/completions)
 * - KoboldAI native API (/api/v1/generate)
 * - Auto-detection of available API mode
 * - Streaming responses via SSE
 * - Connection health monitoring
 * - Configurable generation parameters
 */
class KoboldCppProvider : public AICompletionProvider {
public:
    KoboldCppProvider();
    explicit KoboldCppProvider(const KoboldCppConfig& config);
    explicit KoboldCppProvider(const std::string& endpoint);
    ~KoboldCppProvider() override;

    // ── AICompletionProvider interface ──────────────────────────
    std::vector<CompletionItem> generateCompletions(
        const CodeContext& context,
        const std::string& prefix,
        size_t maxSuggestions = 10
    ) override;
    
    bool isReady() const override;
    bool initialize() override;
    void shutdown() override;

    // ── KoboldCpp-specific methods ─────────────────────────────
    
    /// Raw text completion via /v1/completions or /api/v1/generate
    KoboldCppResponse complete(const std::string& prompt);
    KoboldCppResponse complete(const std::string& prompt, int max_tokens);
    
    /// Chat completion via /v1/chat/completions
    KoboldCppResponse chat(const std::string& message);
    KoboldCppResponse chat(const std::string& message, const std::string& system_prompt);
    KoboldCppResponse chat(const std::vector<std::pair<std::string,std::string>>& messages);
    
    /// Streaming completion (calls callback for each token)
    bool complete_streaming(const std::string& prompt, StreamCallback callback);
    bool chat_streaming(const std::string& message, StreamCallback callback);
    
    /// Server health and info
    bool test_connection();
    KoboldCppServerInfo get_server_info();
    bool is_model_loaded();
    
    /// Configuration
    void set_config(const KoboldCppConfig& config);
    const KoboldCppConfig& get_config() const { return config_; }
    void set_endpoint(const std::string& endpoint);
    void set_temperature(float temp) { config_.temperature = temp; }
    void set_max_tokens(int tokens) { config_.max_tokens = tokens; }
    void set_api_mode(KoboldCppConfig::APIMode mode) { config_.api_mode = mode; }

private:
    KoboldCppConfig config_;
    bool initialized_ = false;
    KoboldCppConfig::APIMode detected_mode_ = KoboldCppConfig::APIMode::OPENAI_COMPATIBLE;
    
    // Internal HTTP helpers
    KoboldCppResponse send_openai_completion(const std::string& prompt, int max_tokens);
    KoboldCppResponse send_openai_chat(const std::vector<std::pair<std::string,std::string>>& messages);
    KoboldCppResponse send_koboldai_generate(const std::string& prompt, int max_tokens);
    std::string http_post(const std::string& url, const std::string& json_body);
    std::string http_get(const std::string& url);
    
    // Code completion helpers
    std::string build_code_prompt(const CodeContext& context, const std::string& prefix) const;
    std::vector<CompletionItem> parse_completions(const std::string& response_text, size_t max) const;
    std::string infer_kind(const std::string& text) const;
    
    // Auto-detection
    bool detect_api_mode();
};

} // namespace ai
} // namespace bolt

#endif // BOLT_AI_KOBOLDCPP_PROVIDER_HPP
