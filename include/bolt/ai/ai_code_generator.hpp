#ifndef BOLT_AI_AI_CODE_GENERATOR_HPP_
#define BOLT_AI_AI_CODE_GENERATOR_HPP_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "bolt/ai/ai_completion_provider.hpp"
#include "bolt/ai/code_generator_interface.hpp"
#include "bolt/ai/template_code_generator.hpp"

namespace bolt {

/**
 * AI-enhanced code generator that uses AI completion providers.
 *
 * generateFunction, generateClass and generateMethod ask the AI provider first when it is ready; the template
 * generator answers when the provider has nothing and fallback is enabled. generateTemplate never uses the
 * provider: it is always produced by the template generator, whatever the fallback setting.
 * generateDocumentation and explainCode return the provider's first completion.
 */
class AIEnhancedCodeGenerator : public AICodeGenerator {
 private:
    std::unique_ptr<AICompletionProvider> aiProvider_;
    std::unique_ptr<TemplateCodeGenerator> templateGenerator_;
    bool initialized_ = false;

    std::string createGenerationPrompt(const std::string& type, const std::string& description,
                                       const CodeGenerationContext& context) const;

    GeneratedCode processAIResponse(const std::vector<CompletionItem>& completions,
                                    const std::string& generatorType) const;

 public:
    AIEnhancedCodeGenerator();
    explicit AIEnhancedCodeGenerator(std::unique_ptr<AICompletionProvider> provider);

    void setAIProvider(std::unique_ptr<AICompletionProvider> provider);

    GeneratedCode generateFunction(const std::string& description, const CodeGenerationContext& context = {}) override;

    GeneratedCode generateClass(const std::string& className, const std::string& description,
                                const CodeGenerationContext& context = {}) override;

    GeneratedCode generateMethod(const std::string& methodName, const std::string& description,
                                 const std::string& className, const CodeGenerationContext& context = {}) override;

    GeneratedCode generateTemplate(const std::string& templateType, const CodeGenerationContext& context = {}) override;

    std::string generateDocumentation(const std::string& code, const CodeGenerationContext& context = {}) override;

    std::string explainCode(const std::string& code, const CodeGenerationContext& context = {}) override;

    bool isReady() const override;
    bool initialize() override;
    void shutdown() override;

    // Configuration
    void setFallbackToTemplate(bool enabled) { fallbackToTemplate_ = enabled; }
    bool isFallbackToTemplateEnabled() const { return fallbackToTemplate_; }

 private:
    bool fallbackToTemplate_ = true;
};

/**
 * Code generation service manager
 */
class CodeGenerationService {
 private:
    std::unique_ptr<AICodeGenerator> generator_;
    static std::unique_ptr<CodeGenerationService> instance_;

    CodeGenerationService() = default;

 public:
    static CodeGenerationService& getInstance();

    void setGenerator(std::unique_ptr<AICodeGenerator> generator);
    AICodeGenerator* getGenerator() const { return generator_.get(); }

    // Convenience methods
    GeneratedCode generateFunction(const std::string& description, const CodeGenerationContext& context = {});
    GeneratedCode generateClass(const std::string& className, const std::string& description,
                                const CodeGenerationContext& context = {});
    std::string generateDocumentation(const std::string& code, const CodeGenerationContext& context = {});
    std::string explainCode(const std::string& code, const CodeGenerationContext& context = {});

    bool isReady() const;
};

}  // namespace bolt

#endif  // BOLT_AI_AI_CODE_GENERATOR_HPP_
