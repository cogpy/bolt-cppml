#ifndef BOLT_AI_TEMPLATE_REFACTORING_ENGINE_HPP_
#define BOLT_AI_TEMPLATE_REFACTORING_ENGINE_HPP_

#include <functional>
#include <regex>
#include <string>
#include <vector>

#include "bolt/ai/refactoring_engine_interface.hpp"

namespace bolt {

/**
 * Template-based refactoring engine with pattern recognition
 */
class TemplateRefactoringEngine : public AIRefactoringEngine {
 private:
    struct RefactoringPattern {
        std::string name;
        std::string description;
        std::regex pattern;
        std::function<AIRefactoringSuggestion(const std::smatch&, const std::string&)> suggestionGenerator;
    };

    std::vector<RefactoringPattern> patterns_;
    bool initialized_ = false;

    void loadDefaultPatterns();
    AIRefactoringSuggestion createExtractMethodSuggestion(const std::smatch& match, const std::string& filePath) const;
    AIRefactoringSuggestion createComplexityReductionSuggestion(const std::smatch& match,
                                                                const std::string& filePath) const;
    AIRefactoringSuggestion createDuplicationRemovalSuggestion(const std::smatch& match,
                                                               const std::string& filePath) const;

 public:
    TemplateRefactoringEngine();

    std::vector<AIRefactoringSuggestion> generateSuggestions(const std::string& code, const std::string& filePath = "",
                                                             const std::string& language = "cpp") override;

    std::vector<AIRefactoringSuggestion> findExtractMethodOpportunities(const std::string& code,
                                                                        const std::string& filePath = "") override;

    std::vector<AIRefactoringSuggestion> findComplexityReductions(const std::string& code,
                                                                  const std::string& filePath = "") override;

    std::vector<AIRefactoringSuggestion> findDuplicationRemoval(const std::string& code,
                                                                const std::string& filePath = "") override;

    std::vector<AIRefactoringSuggestion> findDesignPatternOpportunities(const std::string& code,
                                                                        const std::string& filePath = "") override;

    std::vector<AIRefactoringSuggestion> findPerformanceOptimizations(const std::string& code,
                                                                      const std::string& filePath = "") override;

    std::vector<AIRefactoringSuggestion> findSecurityImprovements(const std::string& code,
                                                                  const std::string& filePath = "") override;

    std::string applyRefactoring(const std::string& code, const AIRefactoringSuggestion& suggestion) override;

    bool validateRefactoring(const std::string& originalCode, const std::string& refactoredCode,
                             const AIRefactoringSuggestion& suggestion) override;

    bool isReady() const override { return initialized_; }
    bool initialize() override;
    void shutdown() override { initialized_ = false; }
};

}  // namespace bolt

#endif  // BOLT_AI_TEMPLATE_REFACTORING_ENGINE_HPP_
