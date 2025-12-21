/**
 * @file lsp_editor_bridge.cpp
 * @brief Implementation of the LSP-Editor bridge component
 */

#include "bolt/editor/lsp_editor_bridge.hpp"
#include <algorithm>
#include <filesystem>

namespace bolt {
namespace lsp {

// Simple LspServerConfigManager implementation
void LspServerConfigManager::autoConfigureServers() {
    // Auto-detection is handled by LspServerConfigs
}

std::map<std::string, LspClientConfig> LspServerConfigManager::getAllConfigs() const {
    std::map<std::string, LspClientConfig> configs;
    auto available = LspServerConfigs::getAvailableConfigs();
    for (const auto& lang : available) {
        if (LspServerConfigs::isServerAvailable(lang)) {
            configs[lang] = LspServerConfigs::getConfigForLanguage(lang);
        }
    }
    return configs;
}

bool LspServerConfigManager::isServerAvailable(const std::string& languageId) const {
    return LspServerConfigs::isServerAvailable(languageId);
}

std::optional<LspClientConfig> LspServerConfigManager::getConfig(const std::string& languageId) const {
    if (LspServerConfigs::isServerAvailable(languageId)) {
        return LspServerConfigs::getConfigForLanguage(languageId);
    }
    return std::nullopt;
}

LspEditorBridge::LspEditorBridge() 
    : clientManager_(std::make_unique<LspClientManager>())
    , configManager_(std::make_unique<LspServerConfigManager>()) {
}

LspEditorBridge::~LspEditorBridge() {
    shutdown();
}

bool LspEditorBridge::initialize() {
    if (initialized_) return true;
    
    // Auto-configure available language servers
    configManager_->autoConfigureServers();
    
    // Get available servers and register them with the client manager
    auto configs = configManager_->getAllConfigs();
    
    for (const auto& [language, config] : configs) {
        if (configManager_->isServerAvailable(language)) {
            clientManager_->registerLanguageServer(language, config);
        }
    }
    
    initialized_ = true;
    return !configs.empty();
}

void LspEditorBridge::shutdown() {
    if (!initialized_) return;
    
    // Close all open documents
    {
        std::lock_guard<std::mutex> lock(documentsMutex_);
        for (const auto& [path, state] : openDocuments_) {
            auto* client = getClientForLanguage(state.languageId);
            if (client && client->isConnected()) {
                TextDocumentIdentifier docId;
                docId.uri = pathToUri(path);
                client->didCloseTextDocument(docId);
            }
        }
        openDocuments_.clear();
    }
    
    // Shutdown client manager
    clientManager_->shutdown();
    
    initialized_ = false;
}

bool LspEditorBridge::isLanguageSupported(const std::string& languageId) const {
    return configManager_->isServerAvailable(languageId);
}

void LspEditorBridge::documentOpened(const std::string& filePath, const std::string& content, 
                                     const std::string& languageId) {
    if (!initialized_) return;
    
    std::string langId = languageId.empty() ? detectLanguageId(filePath) : languageId;
    
    // Store document state
    {
        std::lock_guard<std::mutex> lock(documentsMutex_);
        DocumentState state;
        state.languageId = langId;
        state.version = 1;
        state.content = content;
        openDocuments_[filePath] = state;
    }
    
    // Notify LSP client
    auto* client = getClientForLanguage(langId);
    if (client) {
        // Connect if not already connected
        if (!client->isConnected()) {
            auto config = configManager_->getConfig(langId);
            if (config) {
                // Set root path to file's directory
                config->rootPath = std::filesystem::path(filePath).parent_path().string();
                client->connect(*config);
                
                // Initialize the session
                client->initialize([](bool success) {
                    // Initialization callback - could log or handle errors
                });
                
                // Wait a bit for initialization
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        
        if (client->isConnected()) {
            TextDocumentItem doc;
            doc.uri = pathToUri(filePath);
            doc.languageId = langId;
            doc.version = 1;
            doc.text = content;
            client->didOpenTextDocument(doc);
        }
    }
}

void LspEditorBridge::documentChanged(const std::string& filePath, const std::string& content, int version) {
    if (!initialized_) return;
    
    std::string langId;
    {
        std::lock_guard<std::mutex> lock(documentsMutex_);
        auto it = openDocuments_.find(filePath);
        if (it == openDocuments_.end()) return;
        
        it->second.content = content;
        it->second.version = version;
        langId = it->second.languageId;
    }
    
    auto* client = getClientForLanguage(langId);
    if (client && client->isConnected()) {
        VersionedTextDocumentIdentifier docId;
        docId.uri = pathToUri(filePath);
        docId.version = version;
        
        TextDocumentContentChangeEvent change;
        change.text = content;
        
        client->didChangeTextDocument(docId, {change});
    }
}

void LspEditorBridge::documentClosed(const std::string& filePath) {
    if (!initialized_) return;
    
    std::string langId;
    {
        std::lock_guard<std::mutex> lock(documentsMutex_);
        auto it = openDocuments_.find(filePath);
        if (it == openDocuments_.end()) return;
        
        langId = it->second.languageId;
        openDocuments_.erase(it);
    }
    
    auto* client = getClientForLanguage(langId);
    if (client && client->isConnected()) {
        TextDocumentIdentifier docId;
        docId.uri = pathToUri(filePath);
        client->didCloseTextDocument(docId);
    }
}

void LspEditorBridge::requestCompletion(const std::string& filePath, size_t line, size_t column,
                                        const std::string& triggerChar) {
    if (!initialized_) return;
    
    std::string langId;
    {
        std::lock_guard<std::mutex> lock(documentsMutex_);
        auto it = openDocuments_.find(filePath);
        if (it == openDocuments_.end()) return;
        langId = it->second.languageId;
    }
    
    auto* client = getClientForLanguage(langId);
    if (!client || !client->isConnected()) return;
    
    completionPending_ = true;
    
    TextDocumentPositionParams params;
    params.textDocument.uri = pathToUri(filePath);
    params.position.line = static_cast<int>(line);
    params.position.character = static_cast<int>(column);
    
    client->completion(params, [this](const CompletionList& completions) {
        std::lock_guard<std::mutex> lock(completionMutex_);
        
        completionState_.items.clear();
        for (const auto& item : completions.items) {
            EditorCompletionItem editorItem;
            editorItem.label = item.label;
            editorItem.detail = item.detail.value_or("");
            editorItem.documentation = item.documentation.value_or("");
            editorItem.insertText = item.insertText.value_or(item.label);
            
            // Convert kind to string
            int kindValue = item.kind.has_value() ? static_cast<int>(item.kind.value()) : 0;
            switch (kindValue) {
                case 1: editorItem.kind = "text"; break;
                case 2: editorItem.kind = "method"; break;
                case 3: editorItem.kind = "function"; break;
                case 4: editorItem.kind = "constructor"; break;
                case 5: editorItem.kind = "field"; break;
                case 6: editorItem.kind = "variable"; break;
                case 7: editorItem.kind = "class"; break;
                case 8: editorItem.kind = "interface"; break;
                case 9: editorItem.kind = "module"; break;
                case 10: editorItem.kind = "property"; break;
                case 11: editorItem.kind = "unit"; break;
                case 12: editorItem.kind = "value"; break;
                case 13: editorItem.kind = "enum"; break;
                case 14: editorItem.kind = "keyword"; break;
                case 15: editorItem.kind = "snippet"; break;
                case 16: editorItem.kind = "color"; break;
                case 17: editorItem.kind = "file"; break;
                case 18: editorItem.kind = "reference"; break;
                case 19: editorItem.kind = "folder"; break;
                case 20: editorItem.kind = "enum_member"; break;
                case 21: editorItem.kind = "constant"; break;
                case 22: editorItem.kind = "struct"; break;
                case 23: editorItem.kind = "event"; break;
                case 24: editorItem.kind = "operator"; break;
                case 25: editorItem.kind = "type_parameter"; break;
                default: editorItem.kind = "unknown"; break;
            }
            
            editorItem.sortOrder = 0;  // Default sort order
            completionState_.items.push_back(editorItem);
        }
        
        completionState_.isVisible = !completionState_.items.empty();
        completionState_.selectedIndex = 0;
        completionState_.lastRequestTime = std::chrono::steady_clock::now();
        completionPending_ = false;
    });
}

std::string LspEditorBridge::acceptCompletion() {
    std::lock_guard<std::mutex> lock(completionMutex_);
    
    if (!completionState_.isVisible || completionState_.items.empty()) {
        return "";
    }
    
    std::string insertText = completionState_.items[completionState_.selectedIndex].insertText;
    dismissCompletion();
    return insertText;
}

void LspEditorBridge::dismissCompletion() {
    std::lock_guard<std::mutex> lock(completionMutex_);
    completionState_.isVisible = false;
    completionState_.items.clear();
    completionState_.selectedIndex = 0;
}

void LspEditorBridge::selectNextCompletion() {
    std::lock_guard<std::mutex> lock(completionMutex_);
    if (!completionState_.items.empty()) {
        completionState_.selectedIndex = (completionState_.selectedIndex + 1) % completionState_.items.size();
    }
}

void LspEditorBridge::selectPreviousCompletion() {
    std::lock_guard<std::mutex> lock(completionMutex_);
    if (!completionState_.items.empty()) {
        completionState_.selectedIndex = (completionState_.selectedIndex + completionState_.items.size() - 1) 
                                         % completionState_.items.size();
    }
}

void LspEditorBridge::requestHover(const std::string& filePath, size_t line, size_t column) {
    if (!initialized_) return;
    
    std::string langId;
    {
        std::lock_guard<std::mutex> lock(documentsMutex_);
        auto it = openDocuments_.find(filePath);
        if (it == openDocuments_.end()) return;
        langId = it->second.languageId;
    }
    
    auto* client = getClientForLanguage(langId);
    if (!client || !client->isConnected()) return;
    
    hoverPending_ = true;
    
    TextDocumentPositionParams params;
    params.textDocument.uri = pathToUri(filePath);
    params.position.line = static_cast<int>(line);
    params.position.character = static_cast<int>(column);
    
    client->hover(params, [this](const std::optional<Hover>& hover) {
        std::lock_guard<std::mutex> lock(hoverMutex_);
        
        if (hover.has_value() && !hover->contents.empty()) {
            hoverState_.info.content = hover->contents;
            hoverState_.info.language = ""; // Could parse from markdown code blocks
            hoverState_.info.hasRange = false; // Simplified
            hoverState_.isVisible = true;
            hoverState_.showTime = std::chrono::steady_clock::now();
        } else {
            hoverState_.isVisible = false;
        }
        
        hoverPending_ = false;
    });
}

void LspEditorBridge::dismissHover() {
    std::lock_guard<std::mutex> lock(hoverMutex_);
    hoverState_.isVisible = false;
}

std::vector<EditorDiagnostic> LspEditorBridge::getDiagnostics(const std::string& filePath) const {
    std::lock_guard<std::mutex> lock(documentsMutex_);
    auto it = openDocuments_.find(filePath);
    if (it != openDocuments_.end()) {
        return it->second.diagnostics;
    }
    return {};
}

void LspEditorBridge::setDiagnosticsCallback(DiagnosticsCallback callback) {
    diagnosticsCallback_ = callback;
}

size_t LspEditorBridge::getDiagnosticCount(const std::string& filePath) const {
    return getDiagnostics(filePath).size();
}

size_t LspEditorBridge::getErrorCount(const std::string& filePath) const {
    auto diagnostics = getDiagnostics(filePath);
    return std::count_if(diagnostics.begin(), diagnostics.end(),
        [](const EditorDiagnostic& d) { return d.severity == DiagnosticSeverity::Error; });
}

size_t LspEditorBridge::getWarningCount(const std::string& filePath) const {
    auto diagnostics = getDiagnostics(filePath);
    return std::count_if(diagnostics.begin(), diagnostics.end(),
        [](const EditorDiagnostic& d) { return d.severity == DiagnosticSeverity::Warning; });
}

std::string LspEditorBridge::pathToUri(const std::string& path) {
    return "file://" + path;
}

std::string LspEditorBridge::uriToPath(const std::string& uri) {
    if (uri.substr(0, 7) == "file://") {
        return uri.substr(7);
    }
    return uri;
}

std::string LspEditorBridge::detectLanguageId(const std::string& filePath) {
    std::filesystem::path p(filePath);
    std::string ext = p.extension().string();
    
    // Convert to lowercase
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == ".cpp" || ext == ".cxx" || ext == ".cc" || ext == ".c++" ||
        ext == ".hpp" || ext == ".hxx" || ext == ".hh" || ext == ".h") {
        return "cpp";
    } else if (ext == ".c") {
        return "c";
    } else if (ext == ".py" || ext == ".pyw") {
        return "python";
    } else if (ext == ".js" || ext == ".mjs") {
        return "javascript";
    } else if (ext == ".ts" || ext == ".tsx") {
        return "typescript";
    } else if (ext == ".rs") {
        return "rust";
    } else if (ext == ".go") {
        return "go";
    } else if (ext == ".java") {
        return "java";
    } else if (ext == ".json") {
        return "json";
    } else if (ext == ".xml") {
        return "xml";
    } else if (ext == ".html" || ext == ".htm") {
        return "html";
    } else if (ext == ".css") {
        return "css";
    } else if (ext == ".md" || ext == ".markdown") {
        return "markdown";
    }
    
    return "plaintext";
}

void LspEditorBridge::update() {
    // Process any pending responses or state updates
    // This is called each frame from the main loop
    
    // Auto-hide hover after timeout
    {
        std::lock_guard<std::mutex> lock(hoverMutex_);
        if (hoverState_.isVisible) {
            auto elapsed = std::chrono::steady_clock::now() - hoverState_.showTime;
            if (elapsed > std::chrono::seconds(10)) {
                hoverState_.isVisible = false;
            }
        }
    }
}

LspClient* LspEditorBridge::getClientForLanguage(const std::string& languageId) {
    return clientManager_->getClientForLanguage(languageId);
}

void LspEditorBridge::handleCompletionResponse(std::shared_ptr<JsonValue> result) {
    // This is handled in the lambda callback in requestCompletion
}

void LspEditorBridge::handleHoverResponse(std::shared_ptr<JsonValue> result) {
    // This is handled in the lambda callback in requestHover
}

void LspEditorBridge::handleDiagnostics(const std::string& uri, std::shared_ptr<JsonValue> diagnostics) {
    std::string filePath = uriToPath(uri);
    
    std::vector<EditorDiagnostic> editorDiagnostics;
    
    if (diagnostics && diagnostics->getType() == JsonValue::Array) {
        for (size_t i = 0; i < diagnostics->getArraySize(); ++i) {
            auto diag = diagnostics->getArrayElement(i);
            if (diag) {
                editorDiagnostics.push_back(convertDiagnostic(diag));
            }
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(documentsMutex_);
        auto it = openDocuments_.find(filePath);
        if (it != openDocuments_.end()) {
            it->second.diagnostics = editorDiagnostics;
        }
    }
    
    if (diagnosticsCallback_) {
        diagnosticsCallback_(uri, editorDiagnostics);
    }
}

EditorCompletionItem LspEditorBridge::convertCompletionItem(std::shared_ptr<JsonValue> item) {
    EditorCompletionItem result;
    
    if (!item || item->getType() != JsonValue::Object) {
        return result;
    }
    
    auto label = item->getProperty("label");
    if (label && label->getType() == JsonValue::String) {
        result.label = label->asString();
    }
    
    auto detail = item->getProperty("detail");
    if (detail && detail->getType() == JsonValue::String) {
        result.detail = detail->asString();
    }
    
    auto insertText = item->getProperty("insertText");
    if (insertText && insertText->getType() == JsonValue::String) {
        result.insertText = insertText->asString();
    } else {
        result.insertText = result.label;
    }
    
    return result;
}

EditorDiagnostic LspEditorBridge::convertDiagnostic(std::shared_ptr<JsonValue> diagnostic) {
    EditorDiagnostic result;
    
    if (!diagnostic || diagnostic->getType() != JsonValue::Object) {
        return result;
    }
    
    auto range = diagnostic->getProperty("range");
    if (range && range->getType() == JsonValue::Object) {
        auto start = range->getProperty("start");
        auto end = range->getProperty("end");
        
        if (start && start->getType() == JsonValue::Object) {
            auto line = start->getProperty("line");
            auto character = start->getProperty("character");
            if (line) result.startLine = static_cast<size_t>(line->asNumber());
            if (character) result.startColumn = static_cast<size_t>(character->asNumber());
        }
        
        if (end && end->getType() == JsonValue::Object) {
            auto line = end->getProperty("line");
            auto character = end->getProperty("character");
            if (line) result.endLine = static_cast<size_t>(line->asNumber());
            if (character) result.endColumn = static_cast<size_t>(character->asNumber());
        }
    }
    
    auto severity = diagnostic->getProperty("severity");
    if (severity && severity->getType() == JsonValue::Number) {
        result.severity = static_cast<DiagnosticSeverity>(static_cast<int>(severity->asNumber()));
    }
    
    auto message = diagnostic->getProperty("message");
    if (message && message->getType() == JsonValue::String) {
        result.message = message->asString();
    }
    
    auto source = diagnostic->getProperty("source");
    if (source && source->getType() == JsonValue::String) {
        result.source = source->asString();
    }
    
    return result;
}

} // namespace lsp
} // namespace bolt
