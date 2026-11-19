
#ifndef BOLT_EDITOR_CODE_FOLDING_HPP_
#define BOLT_EDITOR_CODE_FOLDING_HPP_

#include <map>
#include <string>
#include <vector>

#include "../core/thread_safety.hpp"

namespace bolt {

struct FoldRange {
    size_t startLine;
    size_t endLine;
    bool isFolded;
    std::string placeholder;
};

class CodeFolding {
 private:
    ThreadSafe<std::map<std::string, std::vector<FoldRange>>> foldingRanges_;

 public:
    static CodeFolding& getInstance() {
        static CodeFolding instance;
        return instance;
    }

    void addFoldRange(const std::string& filePath, size_t startLine, size_t endLine) {
        foldingRanges_.write([&](auto& ranges) { ranges[filePath].push_back({startLine, endLine, false, "..."}); });
    }

    void toggleFold(const std::string& filePath, size_t line) {
        foldingRanges_.write([&](auto& ranges) {
            auto& fileRanges = ranges[filePath];
            for (auto& range : fileRanges) {
                if (line >= range.startLine && line <= range.endLine) {
                    range.isFolded = !range.isFolded;
                    break;
                }
            }
        });
    }

    std::vector<FoldRange> getFoldingRanges(const std::string& filePath) const {
        return foldingRanges_.read([&](const auto& ranges) {
            auto it = ranges.find(filePath);
            return it != ranges.end() ? it->second : std::vector<FoldRange>{};
        });
    }
};

}  // namespace bolt

#endif  // BOLT_EDITOR_CODE_FOLDING_HPP_
