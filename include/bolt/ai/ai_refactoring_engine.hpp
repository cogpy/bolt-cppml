#ifndef BOLT_AI_AI_REFACTORING_ENGINE_HPP_
#define BOLT_AI_AI_REFACTORING_ENGINE_HPP_

#include <functional>
#include <memory>
#include <regex>
#include <string>
#include <vector>

#include "bolt/ai/ai_completion_provider.hpp"
#include "bolt/ai/refactoring_engine_interface.hpp"
#include "bolt/ai/template_refactoring_engine.hpp"
#include "bolt/core/code_analyzer.hpp"

namespace bolt {

/**
 * AI-enhanced refactoring engine using AI completion providers.
 *
 * generateSuggestions returns the AI provider's suggestions when it is ready, followed by the template engine's
 * when fallback is enabled. The find* queries are answered by the template engine when fallback is enabled, even
 * with an AI provider. applyRefactoring and validateRefactoring are done by the template engine; without one the
 * code comes back unchanged and validation is false.
 */
class AIEnhancedRefactoringEngine : public AIRefactoringEngine {
 private:
    std::unique_ptr<AICompletionProvider> aiProvider_;
    std::unique_ptr<TemplateRefactoringEngine> templateEngine_;
    bool initialized_ = false;
    bool fallbackToTemplate_ = true;

    std::string createRefactoringPrompt(const std::string& code, const std::string& refactoringType,
                                        const std::string& context = "") const;

    std::vector<AIRefactoringSuggestion> processAIRefactoringResponse(const std::vector<CompletionItem>& completions,
                                                                      const std::string& originalCode,
                                                                      const std::string& filePath,
                                                                      const std::string& refactoringType) const;

    AIRefactoringSuggestion parseRefactoringSuggestion(const std::string& response, const std::string& originalCode,
                                                       const std::string& filePath,
                                                       const std::string& refactoringType) const;

 public:
    AIEnhancedRefactoringEngine();
    explicit AIEnhancedRefactoringEngine(std::unique_ptr<AICompletionProvider> provider);

    void setAIProvider(std::unique_ptr<AICompletionProvider> provider);

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

    bool isReady() const override;
    bool initialize() override;
    void shutdown() override;

    // Configuration
    void setFallbackToTemplate(bool enabled) { fallbackToTemplate_ = enabled; }
    bool isFallbackToTemplateEnabled() const { return fallbackToTemplate_; }
};

/**
 * Refactoring service manager that integrates with the existing code analyzer
 */
class RefactoringService {
 private:
    std::unique_ptr<AIRefactoringEngine> engine_;
    static std::unique_ptr<RefactoringService> instance_;

    RefactoringService() = default;

 public:
    static RefactoringService& getInstance();

    void setEngine(std::unique_ptr<AIRefactoringEngine> engine);
    AIRefactoringEngine* getEngine() const { return engine_.get(); }

    // Convenience methods
    std::vector<AIRefactoringSuggestion> analyzeCode(const std::string& code, const std::string& filePath = "");
    std::vector<AIRefactoringSuggestion> findAllOpportunities(const std::string& code,
                                                              const std::string& filePath = "");

    std::string applyRefactoring(const std::string& code, const AIRefactoringSuggestion& suggestion);
    bool validateRefactoring(const std::string& originalCode, const std::string& refactoredCode,
                             const AIRefactoringSuggestion& suggestion);

    bool isReady() const;

    // Statistics and reporting
    struct RefactoringStats {
        size_t totalSuggestionsGenerated = 0;
        size_t suggestionsApplied = 0;
        size_t successfulRefactorings = 0;
        double averageConfidenceScore = 0.0;
    };

    RefactoringStats getStatistics() const;
    void resetStatistics();

 private:
    mutable RefactoringStats stats_;
};

}  // namespace bolt

#endif  // BOLT_AI_AI_REFACTORING_ENGINE_HPP_
