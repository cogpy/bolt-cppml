/**
 * @file lsp_editor_bridge.hpp
 * @brief Bridge component connecting LSP clients to the ImGui-based code editor
 * 
 * This component handles:
 * - Document synchronization between editor and LSP servers
 * - Code completion requests and response handling
 * - Hover information requests
 * - Diagnostics collection and display
 */

#ifndef BOLT_LSP_EDITOR_BRIDGE_HPP
#define BOLT_LSP_EDITOR_BRIDGE_HPP

#include "bolt/editor/lsp_client.hpp"
#include "bolt/editor/lsp_server_configs.hpp"
#include "bolt/editor/lsp_protocol.hpp"
#include "bolt/editor/code_completion.hpp"
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <map>
#include <chrono>

namespace bolt {
namespace lsp {

// Use DiagnosticSeverity from lsp_protocol.hpp

/**
 * @brief A diagnostic message from the LSP server
 */
struct EditorDiagnostic {
    size_t startLine;
    size_t startColumn;
    size_t endLine;
    size_t endColumn;
    DiagnosticSeverity severity;
    std::string message;
    std::string source;
    std::string code;
};

/**
 * @brief Completion item for display in the editor
 */
struct EditorCompletionItem {
    std::string label;
    std::string detail;
    std::string documentation;
    std::string insertText;
    std::string kind;  // "function", "variable", "class", etc.
    int sortOrder;
};

/**
 * @brief Hover information for display
 */
struct EditorHoverInfo {
    std::string content;
    std::string language;  // For syntax highlighting in tooltip
    size_t startLine;
    size_t startColumn;
    size_t endLine;
    size_t endColumn;
    bool hasRange;
};

/**
 * @brief State of the completion popup
 */
struct CompletionPopupState {
    bool isVisible = false;
    std::vector<EditorCompletionItem> items;
    int selectedIndex = 0;
    float posX = 0.0f;
    float posY = 0.0f;
    std::string filterText;
    std::chrono::steady_clock::time_point lastRequestTime;
};

/**
 * @brief State of the hover tooltip
 */
struct HoverTooltipState {
    bool isVisible = false;
    EditorHoverInfo info;
    float posX = 0.0f;
    float posY = 0.0f;
    std::chrono::steady_clock::time_point showTime;
};

/**
 * @brief Bridge connecting LSP clients to the code editor
 */
// Forward declaration for config manager
class LspServerConfigManager {
public:
    void autoConfigureServers();
    std::map<std::string, LspClientConfig> getAllConfigs() const;
    bool isServerAvailable(const std::string& languageId) const;
    std::optional<LspClientConfig> getConfig(const std::string& languageId) const;
};

class LspEditorBridge {
public:
    using DiagnosticsCallback = std::function<void(const std::string& uri, const std::vector<EditorDiagnostic>& diagnostics)>;
    
    LspEditorBridge();
    ~LspEditorBridge();
    
    /**
     * @brief Initialize the bridge with auto-detected language servers
     * @return true if at least one language server is available
     */
    bool initialize();
    
    /**
     * @brief Shutdown all LSP connections
     */
    void shutdown();
    
    /**
     * @brief Check if LSP is available for a given language
     * @param languageId The language identifier (e.g., "cpp", "python")
     */
    bool isLanguageSupported(const std::string& languageId) const;
    
    // Document lifecycle
    
    /**
     * @brief Notify LSP that a document was opened
     * @param filePath Full path to the file
     * @param content File content
     * @param languageId Language identifier
     */
    void documentOpened(const std::string& filePath, const std::string& content, const std::string& languageId);
    
    /**
     * @brief Notify LSP that document content changed
     * @param filePath Full path to the file
     * @param content New content
     * @param version Document version number
     */
    void documentChanged(const std::string& filePath, const std::string& content, int version);
    
    /**
     * @brief Notify LSP that a document was closed
     * @param filePath Full path to the file
     */
    void documentClosed(const std::string& filePath);
    
    // Code completion
    
    /**
     * @brief Request code completion at a position
     * @param filePath Full path to the file
     * @param line Line number (0-indexed)
     * @param column Column number (0-indexed)
     * @param triggerChar Optional trigger character
     */
    void requestCompletion(const std::string& filePath, size_t line, size_t column, 
                          const std::string& triggerChar = "");
    
    /**
     * @brief Get current completion popup state
     */
    CompletionPopupState& getCompletionState() { return completionState_; }
    const CompletionPopupState& getCompletionState() const { return completionState_; }
    
    /**
     * @brief Accept the currently selected completion
     * @return The text to insert
     */
    std::string acceptCompletion();
    
    /**
     * @brief Dismiss the completion popup
     */
    void dismissCompletion();
    
    /**
     * @brief Navigate completion selection
     */
    void selectNextCompletion();
    void selectPreviousCompletion();
    
    // Hover information
    
    /**
     * @brief Request hover information at a position
     * @param filePath Full path to the file
     * @param line Line number (0-indexed)
     * @param column Column number (0-indexed)
     */
    void requestHover(const std::string& filePath, size_t line, size_t column);
    
    /**
     * @brief Get current hover tooltip state
     */
    HoverTooltipState& getHoverState() { return hoverState_; }
    const HoverTooltipState& getHoverState() const { return hoverState_; }
    
    /**
     * @brief Dismiss the hover tooltip
     */
    void dismissHover();
    
    // Diagnostics
    
    /**
     * @brief Get diagnostics for a file
     * @param filePath Full path to the file
     */
    std::vector<EditorDiagnostic> getDiagnostics(const std::string& filePath) const;
    
    /**
     * @brief Set callback for diagnostics updates
     */
    void setDiagnosticsCallback(DiagnosticsCallback callback);
    
    /**
     * @brief Get total diagnostic count for a file
     */
    size_t getDiagnosticCount(const std::string& filePath) const;
    
    /**
     * @brief Get error count for a file
     */
    size_t getErrorCount(const std::string& filePath) const;
    
    /**
     * @brief Get warning count for a file
     */
    size_t getWarningCount(const std::string& filePath) const;
    
    // Utility
    
    /**
     * @brief Convert file path to LSP URI format
     */
    static std::string pathToUri(const std::string& path);
    
    /**
     * @brief Convert LSP URI to file path
     */
    static std::string uriToPath(const std::string& uri);
    
    /**
     * @brief Detect language ID from file extension
     */
    static std::string detectLanguageId(const std::string& filePath);
    
    /**
     * @brief Update the bridge state (call each frame)
     */
    void update();
    
private:
    // LSP client manager
    std::unique_ptr<LspClientManager> clientManager_;
    
    // Server configuration manager
    std::unique_ptr<LspServerConfigManager> configManager_;
    
    // Document state tracking
    struct DocumentState {
        std::string languageId;
        int version = 0;
        std::string content;
        std::vector<EditorDiagnostic> diagnostics;
    };
    std::map<std::string, DocumentState> openDocuments_;
    mutable std::mutex documentsMutex_;
    
    // Completion state
    CompletionPopupState completionState_;
    std::mutex completionMutex_;
    std::atomic<bool> completionPending_{false};
    
    // Hover state
    HoverTooltipState hoverState_;
    std::mutex hoverMutex_;
    std::atomic<bool> hoverPending_{false};
    
    // Diagnostics callback
    DiagnosticsCallback diagnosticsCallback_;
    
    // Initialization state
    std::atomic<bool> initialized_{false};
    
    // Internal helpers
    LspClient* getClientForLanguage(const std::string& languageId);
    void handleCompletionResponse(std::shared_ptr<JsonValue> result);
    void handleHoverResponse(std::shared_ptr<JsonValue> result);
    void handleDiagnostics(const std::string& uri, std::shared_ptr<JsonValue> diagnostics);
    EditorCompletionItem convertCompletionItem(std::shared_ptr<JsonValue> item);
    EditorDiagnostic convertDiagnostic(std::shared_ptr<JsonValue> diagnostic);
};

} // namespace lsp
} // namespace bolt

#endif // BOLT_LSP_EDITOR_BRIDGE_HPP
