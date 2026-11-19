#ifndef BOLT_UTILS_FILESYSTEM_UTILS_HPP_
#define BOLT_UTILS_FILESYSTEM_UTILS_HPP_

#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

class FileSystemUtils {
 public:
    static std::string readFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }

        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        return content;
    }

    static void writeFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file for writing: " + filename);
        }
        file << content;
    }

    static void appendToFile(const std::string& filename, const std::string& content) {
        std::ofstream file(filename, std::ios::app);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file for appending: " + filename);
        }
        file << content;
    }
};

#endif  // BOLT_UTILS_FILESYSTEM_UTILS_HPP_
