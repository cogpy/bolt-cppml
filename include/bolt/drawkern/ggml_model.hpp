#ifndef BOLT_DRAWKERN_GGML_MODEL_HPP_
#define BOLT_DRAWKERN_GGML_MODEL_HPP_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "bolt/drawkern/ai_model.hpp"

namespace bolt {
namespace drawkern {

// GGML-based model implementation
class GGMLModel : public AIModel {
 public:
    explicit GGMLModel(const AIModelConfig& config);
    ~GGMLModel();

    bool load() override;
    void unload() override;
    bool is_loaded() const override { return loaded_; }

    AIInferenceResponse generate(const AIInferenceRequest& request) override;
    AIInferenceResponse chat(const std::string& message, const std::string& context = "") override;
    AIInferenceResponse complete_code(const std::string& code, const std::string& language = "cpp") override;

    std::string get_model_info() const override;
    std::vector<std::string> get_capabilities() const override;

 private:
    void* ggml_context_ = nullptr;  // ggml_context pointer
    void* model_data_ = nullptr;    // Model-specific data

    std::string tokenize_and_generate(const std::string& input, const std::map<std::string, float>& params);
    std::vector<int32_t> tokenize(const std::string& text);
    std::string detokenize(const std::vector<int32_t>& tokens);
};

}  // namespace drawkern
}  // namespace bolt

#endif  // BOLT_DRAWKERN_GGML_MODEL_HPP_
