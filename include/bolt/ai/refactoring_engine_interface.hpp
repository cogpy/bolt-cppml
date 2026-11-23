#ifndef BOLT_AI_REFACTORING_ENGINE_INTERFACE_HPP_
#define BOLT_AI_REFACTORING_ENGINE_INTERFACE_HPP_

#include <string>
#include <vector>

namespace bolt {

/**
 * Enhanced refactoring suggestion with AI insights
 */
struct AIRefactoringSuggestion {
    std::string id;
    std::string title;
    std::string description;
    std::string filePath;
    size_t startLine;
    size_t endLine;
    std::string originalCode;
    std::string suggestedCode;
    std::vector<std::string> benefits;
    std::vector<std::string> risks;
    double confidenceScore;
    std::string refactoringType;
    std::string aiReasoning;
    std::vector<std::string> alternativeApproaches;

    // Priority and impact assessment
    enum class Priority { LOW, MEDIUM, HIGH, CRITICAL };
    enum class Impact { MINIMAL, MODERATE, SIGNIFICANT, MAJOR };

    Priority priority = Priority::MEDIUM;
    Impact impact = Impact::MODERATE;

    // Difficulty estimation
    enum class Difficulty { EASY, MODERATE, HARD, EXPERT };
    Difficulty difficulty = Difficulty::MODERATE;

    // Estimated time to implement (in minutes)
    int estimatedTimeMinutes = 30;
};

/**
 * AI-powered refactoring suggestion engine
 */
class AIRefactoringEngine {
 public:
    virtual ~AIRefactoringEngine() = default;

    // Generate AI-enhanced refactoring suggestions
    virtual std::vector<AIRefactoringSuggestion> generateSuggestions(const std::string& code,
                                                                     const std::string& filePath = "",
                                                                     const std::string& language = "cpp") = 0;

    // Analyze specific refactoring patterns
    virtual std::vector<AIRefactoringSuggestion> findExtractMethodOpportunities(const std::string& code,
                                                                                const std::string& filePath = "") = 0;

    virtual std::vector<AIRefactoringSuggestion> findComplexityReductions(const std::string& code,
                                                                          const std::string& filePath = "") = 0;

    virtual std::vector<AIRefactoringSuggestion> findDuplicationRemoval(const std::string& code,
                                                                        const std::string& filePath = "") = 0;

    virtual std::vector<AIRefactoringSuggestion> findDesignPatternOpportunities(const std::string& code,
                                                                                const std::string& filePath = "") = 0;

    // Performance-focused refactoring
    virtual std::vector<AIRefactoringSuggestion> findPerformanceOptimizations(const std::string& code,
                                                                              const std::string& filePath = "") = 0;

    // Security-focused refactoring
    virtual std::vector<AIRefactoringSuggestion> findSecurityImprovements(const std::string& code,
                                                                          const std::string& filePath = "") = 0;

    // Apply refactoring suggestion
    virtual std::string applyRefactoring(const std::string& code, const AIRefactoringSuggestion& suggestion) = 0;

    // Validate refactoring result
    virtual bool validateRefactoring(const std::string& originalCode, const std::string& refactoredCode,
                                     const AIRefactoringSuggestion& suggestion) = 0;

    virtual bool isReady() const = 0;
    virtual bool initialize() = 0;
    virtual void shutdown() = 0;
};

}  // namespace bolt

#endif  // BOLT_AI_REFACTORING_ENGINE_INTERFACE_HPP_
