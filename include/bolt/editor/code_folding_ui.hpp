#ifndef BOLT_EDITOR_CODE_FOLDING_UI_HPP_
#define BOLT_EDITOR_CODE_FOLDING_UI_HPP_

#include <memory>
#include <string>

#include "bolt/editor/code_folding_manager.hpp"

namespace bolt {

class CodeFoldingUI {
 private:
    bool visible_;
    std::shared_ptr<CodeFoldingManager> foldingManager_;

 public:
    struct FoldingGutter {
        bool isVisible;
        int width;
        std::string iconCollapsed;
        std::string iconExpanded;
    };

    CodeFoldingUI();

    void toggleVisibility();
    void setVisibility(bool visible);
    bool isVisible() const;
    void renderFoldingIndicator(size_t lineNumber, bool isFolded);
    void handleFoldingClick(size_t lineNumber);
    void renderFoldingMarkers(const std::string& filePath);
    FoldingGutter createGutter();

 private:
    void renderMarker(size_t startLine, size_t endLine, bool isCollapsed);
    void drawIcon(int x, int y, const std::string& icon);
    static constexpr int lineHeight = 20;  // Default line height in pixels
};

}  // namespace bolt

#endif  // BOLT_EDITOR_CODE_FOLDING_UI_HPP_
