#ifndef BOLT_AI_TEMPLATE_CODE_GENERATOR_HPP_
#define BOLT_AI_TEMPLATE_CODE_GENERATOR_HPP_

#include <string>
#include <unordered_map>
#include <vector>

#include "bolt/ai/code_generator_interface.hpp"

namespace bolt {

/**
 * Template-based code generator for common patterns
 */
class TemplateCodeGenerator : public AICodeGenerator {
 private:
    struct CodeTemplate {
        std::string name;
        std::string pattern;
        std::vector<std::string> placeholders;
        std::string description;
    };

    std::unordered_map<std::string, CodeTemplate> templates_;
    bool initialized_ = false;

    void loadDefaultTemplates();
    std::string processTemplate(const CodeTemplate& tmpl,
                                const std::unordered_map<std::string, std::string>& values) const;

 public:
    TemplateCodeGenerator();

    // AICodeGenerator interface
    GeneratedCode generateFunction(const std::string& description, const CodeGenerationContext& context = {}) override;

    GeneratedCode generateClass(const std::string& className, const std::string& description,
                                const CodeGenerationContext& context = {}) override;

    GeneratedCode generateMethod(const std::string& methodName, const std::string& description,
                                 const std::string& className, const CodeGenerationContext& context = {}) override;

    GeneratedCode generateTemplate(const std::string& templateType, const CodeGenerationContext& context = {}) override;

    std::string generateDocumentation(const std::string& code, const CodeGenerationContext& context = {}) override;

    std::string explainCode(const std::string& code, const CodeGenerationContext& context = {}) override;

    bool isReady() const override { return initialized_; }
    bool initialize() override;
    void shutdown() override { initialized_ = false; }

    // Template management
    void addTemplate(const std::string& name, const CodeTemplate& tmpl);
    std::vector<std::string> getAvailableTemplates() const;
};

}  // namespace bolt

#endif  // BOLT_AI_TEMPLATE_CODE_GENERATOR_HPP_
