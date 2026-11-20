#ifndef BOLT_EDITOR_CODE_FOLDING_VISIBILITY_HPP_
#define BOLT_EDITOR_CODE_FOLDING_VISIBILITY_HPP_

#include <map>
#include <string>
#include <vector>

#include "bolt/editor/code_folding.hpp"

namespace bolt {

class CodeFoldingVisibilityManager {
 private:
    std::map<std::string, std::map<size_t, bool>> visibilityMap_;

 public:
    void setLineVisibility(const std::string& filePath, size_t line, bool visible);
    bool isLineVisible(const std::string& filePath, size_t line) const;
    void toggleFoldedSection(const std::string& filePath, size_t startLine, size_t endLine);
    void showAllLines(const std::string& filePath);
    void updateFoldedSections(const std::string& filePath, const std::vector<FoldRange>& foldRanges);
};

}  // namespace bolt

#endif  // BOLT_EDITOR_CODE_FOLDING_VISIBILITY_HPP_
