#ifndef BOLT_AI_CODE_GENERATOR_INTERFACE_HPP_
#define BOLT_AI_CODE_GENERATOR_INTERFACE_HPP_

#include <string>
#include <unordered_map>
#include <vector>

namespace bolt {

/**
 * Code generation context for AI-powered code creation
 */
struct CodeGenerationContext {
    std::string language = "cpp";
    std::string fileType;
    std::string projectContext;
    std::string existingCode;
    std::vector<std::string> requirements;
    std::vector<std::string> dependencies;
    std::unordered_map<std::string, std::string> metadata;
};

/**
 * Generated code result with metadata
 */
struct GeneratedCode {
    std::string code;
    std::string description;
    std::vector<std::string> dependencies;
    std::vector<std::string> suggestions;
    double confidence = 0.0;
    std::string generatorType;
};

/**
 * AI-powered code generator interface
 */
class AICodeGenerator {
 public:
    virtual ~AICodeGenerator() = default;

    // Function generation
    virtual GeneratedCode generateFunction(const std::string& description,
                                           const CodeGenerationContext& context = {}) = 0;

    // Class generation
    virtual GeneratedCode generateClass(const std::string& className, const std::string& description,
                                        const CodeGenerationContext& context = {}) = 0;

    // Method generation
    virtual GeneratedCode generateMethod(const std::string& methodName, const std::string& description,
                                         const std::string& className, const CodeGenerationContext& context = {}) = 0;

    // Template/boilerplate generation
    virtual GeneratedCode generateTemplate(const std::string& templateType,
                                           const CodeGenerationContext& context = {}) = 0;

    // Documentation generation
    virtual std::string generateDocumentation(const std::string& code, const CodeGenerationContext& context = {}) = 0;

    // Code explanation
    virtual std::string explainCode(const std::string& code, const CodeGenerationContext& context = {}) = 0;

    // Utility methods
    virtual bool isReady() const = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
};

}  // namespace bolt

#endif  // BOLT_AI_CODE_GENERATOR_INTERFACE_HPP_
