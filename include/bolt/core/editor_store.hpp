#ifndef BOLT_CORE_EDITOR_STORE_HPP_
#define BOLT_CORE_EDITOR_STORE_HPP_

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "bolt/core/error_handling.hpp"
#include "bolt/editor/code_folding.hpp"
#include "bolt/gui_components.hpp"

namespace bolt {

struct EditorDocument {
    std::string value;
    std::string filePath;
    struct ScrollPosition {
        int line = 0;
        int character = 0;

        void validate() const {
            if (line < 0 || character < 0) {
                throw EditorException(ErrorCode::INVALID_PARAMETER,
                                      "Scroll position cannot be negative: line=" + std::to_string(line) +
                                          ", character=" + std::to_string(character));
            }
        }
    } scroll;
    struct Cursor {
        size_t position = 0;
        std::optional<BracketMatch> bracketMatch;
    } cursor;
    // Code folding state
    std::vector<FoldRange> foldingRanges;

    void validateDocument() const {
        if (filePath.empty()) {
            throw EditorException(ErrorCode::INVALID_PARAMETER, "Document file path cannot be empty");
        }
        if (filePath.length() > MAX_FILE_PATH_LENGTH) {
            throw EditorException(ErrorCode::INVALID_PARAMETER,
                                  "File path too long: " + std::to_string(filePath.length()) + " > " +
                                      std::to_string(MAX_FILE_PATH_LENGTH));
        }
        if (value.length() > MAX_DOCUMENT_SIZE) {
            throw EditorException(
                ErrorCode::INVALID_PARAMETER,
                "Document too large: " + std::to_string(value.length()) + " > " + std::to_string(MAX_DOCUMENT_SIZE));
        }
        scroll.validate();
    }

    static constexpr size_t MAX_FILE_PATH_LENGTH = 2048;
    static constexpr size_t MAX_DOCUMENT_SIZE = 50 * 1024 * 1024;  // 50MB
};

class EditorStore {
 public:
    static EditorStore& getInstance() {
        static EditorStore instance;
        return instance;
    }

    void setDocument(const std::string& path, const EditorDocument& doc) {
        ErrorHandler::validateParameter(!path.empty(), "Document path cannot be empty");
        doc.validateDocument();

        {
            std::lock_guard<std::mutex> lock(mutex_);

            if (documents_.size() >= MAX_OPEN_DOCUMENTS && documents_.find(path) == documents_.end()) {
                throw EditorException(ErrorCode::EDITOR_OPERATION_FAILED,
                                      "Too many open documents: " + std::to_string(documents_.size()) +
                                          " >= " + std::to_string(MAX_OPEN_DOCUMENTS));
            }

            documents_[path] = doc;
        }
        notifyListeners();
    }

    void updateScrollPosition(const std::string& path, int line, int character) {
        ErrorHandler::validateParameter(!path.empty(), "Document path cannot be empty");

        {
            std::lock_guard<std::mutex> lock(mutex_);

            auto it = findDocumentOrThrowLocked(path, "Document not found: ");

            EditorDocument::ScrollPosition pos{line, character};
            pos.validate();

            it->second.scroll = pos;
        }
        notifyListeners();
    }

    void updateFoldingRanges(const std::string& path, const std::vector<FoldRange>& ranges) {
        ErrorHandler::validateParameter(!path.empty(), "Document path cannot be empty");

        {
            std::lock_guard<std::mutex> lock(mutex_);

            auto it = findDocumentOrThrowLocked(path, "Document not found: ");

            if (ranges.size() > MAX_FOLDING_RANGES) {
                throw EditorException(ErrorCode::EDITOR_FOLDING_ERROR,
                                      "Too many folding ranges: " + std::to_string(ranges.size()) + " > " +
                                          std::to_string(MAX_FOLDING_RANGES));
            }

            it->second.foldingRanges = ranges;
        }
        notifyListeners();
    }

    void toggleFold(const std::string& path, size_t line) {
        ErrorHandler::validateParameter(!path.empty(), "Document path cannot be empty");

        {
            std::lock_guard<std::mutex> lock(mutex_);

            auto it = findDocumentOrThrowLocked(path, "Document not found: ");

            auto& ranges = it->second.foldingRanges;
            bool found = false;
            for (auto& range : ranges) {
                if (line >= range.startLine && line <= range.endLine) {
                    range.isFolded = !range.isFolded;
                    found = true;
                    break;
                }
            }

            if (!found) {
                throw EditorException(ErrorCode::EDITOR_FOLDING_ERROR,
                                      "No foldable range found at line " + std::to_string(line) + " in " + path);
            }
        }
        notifyListeners();
    }

    std::vector<FoldRange> getFoldingRanges(const std::string& path) const {
        ErrorHandler::validateParameter(!path.empty(), "Document path cannot be empty");

        std::lock_guard<std::mutex> lock(mutex_);

        auto it = documents_.find(path);
        if (it != documents_.end()) {
            return it->second.foldingRanges;
        }
        return {};
    }

    // The returned pointer refers to an entry of the documents map and is not
    // protected by the lock once this returns: it is only valid until that
    // document is closed, and reading or writing through it is not
    // synchronised with other threads using the store.
    EditorDocument* getCurrentDocument() {
        std::lock_guard<std::mutex> lock(mutex_);

        if (!selectedFile_.empty() && documents_.count(selectedFile_)) {
            return &documents_[selectedFile_];
        }
        return nullptr;
    }

    void setSelectedFile(const std::string& path) {
        if (!path.empty()) {
            ErrorHandler::validateParameter(path.length() <= EditorDocument::MAX_FILE_PATH_LENGTH,
                                            "File path too long: " + std::to_string(path.length()));
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            selectedFile_ = path;
        }
        notifyListeners();
    }

    std::string getSelectedFile() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return selectedFile_;
    }

    void addListener(std::function<void()> listener) {
        ErrorHandler::validateParameter(listener != nullptr, "Listener cannot be null");

        std::lock_guard<std::mutex> lock(mutex_);

        if (listeners_.size() >= MAX_LISTENERS) {
            throw EditorException(
                ErrorCode::STORE_STATE_INVALID,
                "Too many listeners: " + std::to_string(listeners_.size()) + " >= " + std::to_string(MAX_LISTENERS));
        }

        listeners_.push_back(listener);
    }

    size_t getDocumentCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return documents_.size();
    }

    bool hasDocument(const std::string& path) const {
        ErrorHandler::validateParameter(!path.empty(), "Document path cannot be empty");
        std::lock_guard<std::mutex> lock(mutex_);
        return documents_.find(path) != documents_.end();
    }

    void closeDocument(const std::string& path) {
        ErrorHandler::validateParameter(!path.empty(), "Document path cannot be empty");

        {
            std::lock_guard<std::mutex> lock(mutex_);

            auto it = findDocumentOrThrowLocked(path, "Cannot close document that is not open: ");

            documents_.erase(it);

            // Clear selection if this was the selected file
            if (selectedFile_ == path) {
                selectedFile_.clear();
            }
        }

        notifyListeners();
    }

 private:
    EditorStore() {}

    // Finds the open document at path, or throws EDITOR_DOCUMENT_INVALID with notFoundPrefix + path.
    // The caller must already hold mutex_.
    std::map<std::string, EditorDocument>::iterator findDocumentOrThrowLocked(const std::string& path,
                                                                              const char* notFoundPrefix) {
        auto it = documents_.find(path);
        if (it == documents_.end()) {
            throw EditorException(ErrorCode::EDITOR_DOCUMENT_INVALID, notFoundPrefix + path);
        }
        return it;
    }

    // Must be called WITHOUT mutex_ held. The listeners are copied under the
    // lock and invoked after it is released, so a listener may call back into
    // the store (mutex_ is not recursive).
    void notifyListeners() {
        std::vector<std::function<void()>> listeners;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            listeners = listeners_;
        }
        for (auto& listener : listeners) {
            try {
                if (listener) {
                    listener();
                }
            } catch (const std::exception& e) {
                // Don't let listener failures break the store
                // In a real implementation, this would go to a logging system
            }
        }
    }

    std::string selectedFile_;
    std::map<std::string, EditorDocument> documents_;
    std::vector<std::function<void()>> listeners_;
    mutable std::mutex mutex_;

    static constexpr size_t MAX_OPEN_DOCUMENTS = 50;
    static constexpr size_t MAX_FOLDING_RANGES = 10000;
    static constexpr size_t MAX_LISTENERS = 100;
};

}  // namespace bolt

#endif  // BOLT_CORE_EDITOR_STORE_HPP_
