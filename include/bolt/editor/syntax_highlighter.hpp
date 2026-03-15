
#ifndef SYNTAX_HIGHLIGHTER_HPP
#define SYNTAX_HIGHLIGHTER_HPP

#include <string>
#include <vector>
#include <regex>
#include <map>
#include "bolt/core/thread_safety.hpp"

namespace bolt {

struct Token {
    std::string text;
    std::string type;
    size_t position;
};

class SyntaxHighlighter {
private:
    ThreadSafe<std::map<std::string, std::vector<std::pair<std::regex, std::string>>>> rules_;
    bool enabled_ = true;

public:
    static SyntaxHighlighter& getInstance() {
        static SyntaxHighlighter instance;
        return instance;
    }

    void addLanguageRules(const std::string& language, 
                         const std::vector<std::pair<std::string, std::string>>& patterns) {
        std::vector<std::pair<std::regex, std::string>> compiledRules;
        for (const auto& pattern : patterns) {
            compiledRules.emplace_back(std::regex(pattern.first), pattern.second);
        }
        
        rules_.write([&](auto& rulesMap) {
            rulesMap[language] = compiledRules;
        });
    }

    void addLanguageRule(const std::string& language, const std::string& pattern, const std::string& tokenType) {
        rules_.write([&](auto& rulesMap) {
            rulesMap[language].emplace_back(std::regex(pattern), tokenType);
        });
    }

    std::vector<Token> highlight(const std::string& code, const std::string& language) {
        std::vector<Token> tokens;
        
        auto rules = rules_.read([&](const auto& rulesMap) {
            auto it = rulesMap.find(language);
            return it != rulesMap.end() ? it->second : std::vector<std::pair<std::regex, std::string>>();
        });

        size_t pos = 0;
        std::string remaining = code;

        while (!remaining.empty()) {
            bool matched = false;
            size_t bestMatch = remaining.length();
            std::string bestType;
            std::string matchedText;

            for (const auto& rule : rules) {
                std::smatch match;
                if (std::regex_search(remaining, match, rule.first, std::regex_constants::match_continuous)) {
                    if (static_cast<size_t>(match.position(0)) < bestMatch) {
                        bestMatch = match.position(0);
                        bestType = rule.second;
                        matchedText = match.str(0);
                        matched = true;
                    }
                }
            }

            if (matched) {
                tokens.push_back({matchedText, bestType, pos});
                pos += matchedText.length();
                remaining = remaining.substr(matchedText.length());
            } else {
                tokens.push_back({remaining.substr(0, 1), "text", pos});
                pos += 1;
                remaining = remaining.substr(1);
            }
        }

        return tokens;
    }

    // Alias for highlight
    std::vector<Token> highlightCode(const std::string& code, const std::string& language) {
        return highlight(code, language);
    }

    void setEnabled(bool enable) { enabled_ = enable; }
    bool isEnabled() const { return enabled_; }

    void clearRules() {
        rules_.write([](auto& rulesMap) { rulesMap.clear(); });
    }

    void loadDefaultRules() {
        // Load C++ default rules
        addLanguageRules("cpp", {
            {"\\b(int|void|return|if|else|for|while|class|struct|namespace|using|include|template|typename|const|static|virtual|override|public|private|protected)\\b", "keyword"},
            {"\\d+\\.?\\d*", "number"},
            {"\"[^\"]*\"", "string"},
            {"//.*$", "comment"},
            {"/\\*[\\s\\S]*?\\*/", "comment"}
        });
    }
};

} // namespace bolt

#endif
