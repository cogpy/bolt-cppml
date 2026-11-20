/**
 * @file demo_external_lsp.cpp
 * @brief Demo application for testing external language server integration.
 *
 * This demo tests the integration with real language servers like clangd and pyright.
 * It demonstrates:
 * - Auto-configuration of available language servers
 * - Connecting to external language servers via stdio
 * - Sending didOpen/didClose notifications
 * - Requesting code completion
 * - Requesting hover information
 */

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include "bolt/editor/lsp_client.hpp"
#include "bolt/editor/lsp_server_configs.hpp"

using namespace bolt::lsp;  // NOLINT(build/namespaces)

// Test file content
// clang-format off
const char* CPP_TEST_CODE = R"(
#include <iostream>
#include <vector>
#include <string>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    for (const auto& num : numbers) {
        std::cout << num << std::endl;
    }
    
    std::string message = "Hello, World!";
    std::cout << message << std::endl;
    
    return 0;
}
)";
// clang-format on

const char* PYTHON_TEST_CODE = R"(
import os
import sys
from typing import List, Optional

def greet(name: str) -> str:
    """Return a greeting message."""
    return f"Hello, {name}!"

def main() -> None:
    names: List[str] = ["Alice", "Bob", "Charlie"]
    
    for name in names:
        print(greet(name))
    
    path: Optional[str] = os.getenv("PATH")
    if path:
        print(f"PATH has {len(path.split(':'))} entries")

if __name__ == "__main__":
    main()
)";

void printSeparator(const std::string& title) {
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  " << title << "\n";
    std::cout << std::string(60, '=') << "\n\n";
}

void printSubsection(const std::string& title) {
    std::cout << "\n--- " << title << " ---\n\n";
}

// Describes one external language server exercised by the demo.
struct ServerDemoSpec {
    std::string title;        // Heading printed before the test
    std::string languageKey;  // Key used for the availability check
    std::string name;         // Server name used in messages
    std::string installHint;  // Command that installs the server
    std::string fileName;     // Name of the test file inside the test directory
    const char* code;         // Content of the test file
    LspClientConfig (*makeConfig)(const std::string&);
    Position completionPosition;
    std::optional<Position> hoverPosition;  // No hover request when empty
};

// Prints the command line the server is started with.
void printServerCommand(const LspClientConfig& config) {
    std::cout << "   Command: " << config.serverCommand << "\n";
    std::cout << "   Args: ";
    for (const auto& arg : config.serverArgs) {
        std::cout << arg << " ";
    }
    std::cout << "\n";
}

// Polls until the flag is set by a response callback or the timeout expires.
void waitForFlag(const bool& flag, std::chrono::seconds timeout) {
    auto startTime = std::chrono::steady_clock::now();
    while (!flag && std::chrono::steady_clock::now() - startTime < timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// Requests completion at the given position and prints the first few items.
void requestCompletion(LspClient& client, const std::string& uri, const Position& position) {
    printSubsection("Requesting code completion");

    TextDocumentPositionParams completionParams;
    completionParams.textDocument.uri = uri;
    completionParams.position = position;

    CompletionList completions;
    bool completionReceived = false;

    client.completion(completionParams, [&completions, &completionReceived](const CompletionList& result) {
        completions = result;
        completionReceived = true;
    });

    // Wait for completion response
    waitForFlag(completionReceived, std::chrono::seconds(5));

    if (completionReceived) {
        std::cout << "✅ Received " << completions.items.size() << " completion items\n";
        int shown = 0;
        for (const auto& item : completions.items) {
            if (shown++ < 5) {
                std::cout << "   - " << item.label << "\n";
            }
        }
        if (completions.items.size() > 5) {
            std::cout << "   ... and " << (completions.items.size() - 5) << " more\n";
        }
    } else {
        std::cout << "⚠️  Completion request timed out\n";
    }
}

// Requests hover information at the given position and prints it.
void requestHover(LspClient& client, const std::string& uri, const Position& position) {
    printSubsection("Requesting hover information");

    TextDocumentPositionParams hoverParams;
    hoverParams.textDocument.uri = uri;
    hoverParams.position = position;

    std::optional<Hover> hoverResult;
    bool hoverReceived = false;

    client.hover(hoverParams, [&hoverResult, &hoverReceived](const std::optional<Hover>& result) {
        hoverResult = result;
        hoverReceived = true;
    });

    // Wait for hover response
    waitForFlag(hoverReceived, std::chrono::seconds(5));

    if (hoverReceived && hoverResult.has_value()) {
        std::cout << "✅ Received hover information:\n";
        std::string content = hoverResult->contents;
        // Truncate if too long
        if (content.length() > 200) {
            content = content.substr(0, 200) + "...";
        }
        std::cout << "   " << content << "\n";
    } else {
        std::cout << "⚠️  Hover request timed out or returned empty\n";
    }
}

// Runs the whole demo session (connect, open, query, shut down) against one server.
bool testLanguageServer(const ServerDemoSpec& spec, const std::string& testDir) {
    printSeparator(spec.title);

    if (!LspServerConfigs::isServerAvailable(spec.languageKey)) {
        std::cout << "❌ " << spec.name << " is not available on this system.\n";
        std::cout << "   Install with: " << spec.installHint << "\n";
        return false;
    }

    std::cout << "✅ " << spec.name << " is available\n";

    // Get configuration
    auto config = spec.makeConfig(testDir);
    printServerCommand(config);

    // Create test file
    std::string testFile = testDir + "/" + spec.fileName;
    {
        std::ofstream file(testFile);
        file << spec.code;
    }
    std::cout << "   Created test file: " << testFile << "\n\n";

    // Create client
    printSubsection("Connecting to " + spec.name);

    auto connection = std::make_unique<ProcessLspConnection>();
    LspClient client(std::move(connection));

    if (!client.connect(config)) {
        std::cout << "❌ Failed to connect to " << spec.name << "\n";
        return false;
    }
    std::cout << "✅ Connected to " << spec.name << "\n";

    // Initialize
    printSubsection("Initializing LSP session");

    bool initSuccess = false;
    client.initialize([&initSuccess](bool success) { initSuccess = success; });

    // Wait for initialization (with timeout)
    waitForFlag(initSuccess, std::chrono::seconds(10));

    if (!initSuccess) {
        std::cout << "❌ Failed to initialize LSP session (timeout)\n";
        client.disconnect();
        return false;
    }
    std::cout << "✅ LSP session initialized\n";

    // Open document
    printSubsection("Opening document");

    TextDocumentItem doc;
    doc.uri = "file://" + testFile;
    doc.languageId = spec.languageKey;
    doc.version = 1;
    doc.text = spec.code;

    client.didOpenTextDocument(doc);
    std::cout << "✅ Sent didOpen notification\n";

    // Wait for the server to process the file
    std::this_thread::sleep_for(std::chrono::seconds(2));

    requestCompletion(client, doc.uri, spec.completionPosition);

    if (spec.hoverPosition.has_value()) {
        requestHover(client, doc.uri, *spec.hoverPosition);
    }

    // Close document
    printSubsection("Closing document");

    TextDocumentIdentifier closeDoc;
    closeDoc.uri = doc.uri;
    client.didCloseTextDocument(closeDoc);
    std::cout << "✅ Sent didClose notification\n";

    // Shutdown
    printSubsection("Shutting down");

    client.shutdown([](bool success) {
        std::cout << (success ? "✅" : "❌") << " Shutdown " << (success ? "successful" : "failed") << "\n";
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    client.disconnect();
    std::cout << "✅ Disconnected from " << spec.name << "\n";

    return true;
}

bool testClangd(const std::string& testDir) {
    ServerDemoSpec spec;
    spec.title = "Testing clangd (C/C++ Language Server)";
    spec.languageKey = "cpp";
    spec.name = "clangd";
    spec.installHint = "sudo apt-get install clangd";
    spec.fileName = "test.cpp";
    spec.code = CPP_TEST_CODE;
    spec.makeConfig = [](const std::string& rootPath) { return LspServerConfigs::clangd(rootPath); };
    spec.completionPosition = Position(8, 9);  // After "std::" on line 8
    spec.hoverPosition = Position(6, 10);      // On "vector"
    return testLanguageServer(spec, testDir);
}

bool testPyright(const std::string& testDir) {
    ServerDemoSpec spec;
    spec.title = "Testing pyright (Python Language Server)";
    spec.languageKey = "python";
    spec.name = "pyright";
    spec.installHint = "npm install -g pyright";
    spec.fileName = "test.py";
    spec.code = PYTHON_TEST_CODE;
    spec.makeConfig = [](const std::string& rootPath) { return LspServerConfigs::pyright(rootPath); };
    spec.completionPosition = Position(1, 7);  // After "import "
    return testLanguageServer(spec, testDir);
}

void testAutoConfiguration(const std::string& testDir) {
    printSeparator("Testing Auto-Configuration");

    // Print availability report
    std::cout << LspAutoConfigurator::getAvailabilityReport();

    // Create a client manager
    LspClientManager manager;

    // Auto-configure
    int configured = LspAutoConfigurator::autoConfigureAll(manager, testDir);
    std::cout << "Auto-configured " << configured << " language server(s)\n";

    // List registered languages
    auto languages = manager.getRegisteredLanguages();
    std::cout << "Registered languages:\n";
    for (const auto& lang : languages) {
        std::cout << "   - " << lang << "\n";
    }

    manager.shutdown();
}

int main() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║     Bolt C++ External LSP Integration Demo               ║\n";
    std::cout << "║     Testing Real Language Server Connections             ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n";

    // Create test directory
    std::string testDir = "/tmp/bolt_lsp_test";
    std::filesystem::create_directories(testDir);
    std::cout << "\nTest directory: " << testDir << "\n";

    // Test auto-configuration
    testAutoConfiguration(testDir);

    // Test clangd
    bool clangdSuccess = testClangd(testDir);

    // Test pyright
    bool pyrightSuccess = testPyright(testDir);

    // Summary
    printSeparator("Test Summary");

    std::cout << "clangd (C/C++):  " << (clangdSuccess ? "✅ PASSED" : "❌ FAILED") << "\n";
    std::cout << "pyright (Python): " << (pyrightSuccess ? "✅ PASSED" : "❌ FAILED") << "\n";

    // Cleanup
    std::filesystem::remove_all(testDir);
    std::cout << "\nCleaned up test directory.\n";

    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "  External LSP Integration Demo Complete\n";
    std::cout << std::string(60, '=') << "\n\n";

    return (clangdSuccess && pyrightSuccess) ? 0 : 1;
}
