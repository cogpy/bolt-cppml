#ifndef BOLT_EDITOR_CODE_FOLDING_DETECTOR_HPP_
#define BOLT_EDITOR_CODE_FOLDING_DETECTOR_HPP_

#include <string>
#include <vector>

#include "bolt/editor/code_folding.hpp"

namespace bolt {

class CodeFoldingDetector {
 public:
    static std::vector<FoldRange> detectFoldableRanges(const std::string& content);
    static bool isFoldableRegion(const std::string& line);
    static int findMatchingEnd(const std::vector<std::string>& lines, int startLine);
};

}  // namespace bolt

#endif  // BOLT_EDITOR_CODE_FOLDING_DETECTOR_HPP_
