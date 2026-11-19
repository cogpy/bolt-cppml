
#ifndef BOLT_UTILS_STRING_UTILS_HPP_
#define BOLT_UTILS_STRING_UTILS_HPP_

#include <algorithm>
#include <string>

class StringUtils {
 public:
    static std::string reverseString(const std::string& input) {
        std::string result = input;
        std::reverse(result.begin(), result.end());
        return result;
    }

    static std::string capitalizeString(const std::string& input) {
        std::string result = input;
        if (!result.empty()) {
            result[0] = std::toupper(result[0]);
        }
        return result;
    }

    static int countVowels(const std::string& input) {
        const std::string vowels = "aeiouAEIOU";
        return std::count_if(input.begin(), input.end(),
                             [&vowels](char c) { return vowels.find(c) != std::string::npos; });
    }
};

#endif  // BOLT_UTILS_STRING_UTILS_HPP_
