
#include <iostream>
#include <string>

#include "bolt/bolt.hpp"

#ifndef BOLT_VERSION
#define BOLT_VERSION "unknown"
#endif

namespace {

void printUsage(const char* program) {
    std::cout << "Bolt C++ - AI-Powered Development Environment\n\n"
              << "Usage: " << program << " [options]\n\n"
              << "Options:\n"
              << "  -h, --help     Show this help message and exit\n"
              << "  -v, --version  Show version information and exit\n\n"
              << "With no options, bolt starts an interactive session.\n"
              << "Type /help inside the session for commands and /exit to quit.\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            printUsage(argv[0]);
            return 0;
        }
        if (arg == "-v" || arg == "--version") {
            std::cout << "bolt " << BOLT_VERSION << "\n";
            return 0;
        }
        std::cerr << "bolt: unknown option '" << arg << "'\n\n";
        printUsage(argv[0]);
        return 2;
    }

    try {
        auto& app = bolt::BoltApp::getInstance();
        app.initialize();
        app.run();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
