#ifndef BOLT_DRAWKERN_RWKV_MODEL_HPP_
#define BOLT_DRAWKERN_RWKV_MODEL_HPP_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "bolt/drawkern/ai_model.hpp"

namespace bolt {
namespace drawkern {

// RWKV model implementation
class RWKVModel : public AIModel {
 public:
    explicit RWKVModel(const AIModelConfig& config);
    ~RWKVModel();

    bool load() override;
    void unload() override;
    bool is_loaded() const override { return loaded_; }

    AIInferenceResponse generate(const AIInferenceRequest& request) override;
    AIInferenceResponse chat(const std::string& message, const std::string& context = "") override;
    AIInferenceResponse complete_code(const std::string& code, const std::string& language = "cpp") override;

    std::string get_model_info() const override;
    std::vector<std::string> get_capabilities() const override;

 private:
    void* rwkv_context_ = nullptr;  // RWKV context
    void* rwkv_state_ = nullptr;    // RWKV state

    std::string generate_with_rwkv(const std::string& input, const std::map<std::string, float>& params);
    void reset_state();
    void update_state(const std::vector<int32_t>& tokens);
};

}  // namespace drawkern
}  // namespace bolt

#endif  // BOLT_DRAWKERN_RWKV_MODEL_HPP_
