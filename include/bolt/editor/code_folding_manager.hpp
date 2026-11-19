#ifndef BOLT_EDITOR_CODE_FOLDING_MANAGER_HPP_
#define BOLT_EDITOR_CODE_FOLDING_MANAGER_HPP_

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "bolt/editor/code_folding.hpp"
#include "bolt/editor/code_folding_detector.hpp"
#include "bolt/editor/code_folding_visibility.hpp"

namespace bolt {

class CodeFoldingManager {
 private:
    std::map<std::string, std::vector<FoldRange>> foldingRanges_;
    ThreadSafe<bool> foldingEnabled_{true};
    CodeFoldingVisibilityManager visibilityManager_;
    std::function<void()> onFoldingChanged_;

 public:
    static CodeFoldingManager& getInstance() {
        static CodeFoldingManager instance;
        return instance;
    }

    void updateFoldingRanges(const std::string& filePath, const std::string& content);
    void toggleFold(const std::string& filePath, size_t line);
    std::vector<FoldRange> getFoldingRanges(const std::string& filePath) const;
    void setFoldingEnabled(bool enabled);
    bool isFoldingEnabled() const;
    void handleClick(const std::string& filePath, size_t lineNumber);
    void setOnFoldingChanged(std::function<void()> callback);

 private:
    void refreshFoldingRanges();
};

}  // namespace bolt

#endif  // BOLT_EDITOR_CODE_FOLDING_MANAGER_HPP_
