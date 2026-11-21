#ifndef BOLT_COLLABORATION_COLLABORATION_PROTOCOL_HPP_
#define BOLT_COLLABORATION_COLLABORATION_PROTOCOL_HPP_

#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>

#include "bolt/collaboration/collaborative_session.hpp"
#include "bolt/collaboration/document_operation.hpp"
#include "bolt/network/websocket_server.hpp"

namespace bolt {
namespace collaboration {

enum class MessageType {
    JOIN_DOCUMENT,
    LEAVE_DOCUMENT,
    DOCUMENT_OPERATION,
    CURSOR_UPDATE,
    USER_JOINED,
    USER_LEFT,
    DOCUMENT_STATE,
    ERROR_MESSAGE
};

namespace detail {

// Escapes quotes, backslashes and control characters for embedding in a JSON string value.
inline std::string escapeJsonString(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (c == '"')
            result += "\\\"";
        else if (c == '\\')
            result += "\\\\";
        else if (c == '\n')
            result += "\\n";
        else if (c == '\r')
            result += "\\r";
        else if (c == '\t')
            result += "\\t";
        else
            result += c;
    }
    return result;
}

// Reads the integer value of "key" from a flat JSON object; returns 0 when the key is absent.
inline int extractJsonInt(const std::string& data, const std::string& key) {
    auto pos = data.find("\"" + key + "\":");
    if (pos == std::string::npos)
        return 0;
    pos = data.find(":", pos) + 1;
    auto end = data.find_first_of(",}", pos);
    return std::stoi(data.substr(pos, end - pos));
}

}  // namespace detail

struct ProtocolMessage {
    MessageType type;
    std::string documentId;
    std::string userId;
    std::string data;

    std::string serialize() const {
        std::string result = "{";
        result += "\"type\":" + std::to_string(static_cast<int>(type)) + ",";
        result += "\"documentId\":\"" + documentId + "\",";
        result += "\"userId\":\"" + userId + "\",";
        result += "\"data\":\"" + detail::escapeJsonString(data) + "\"";
        result += "}";
        return result;
    }

    static ProtocolMessage deserialize(const std::string& json) {
        ProtocolMessage msg;
        msg.type = static_cast<MessageType>(detail::extractJsonInt(json, "type"));
        msg.documentId = extractString(json, "documentId");
        msg.userId = extractString(json, "userId");
        msg.data = extractString(json, "data");
        return msg;
    }

 private:
    static std::string extractString(const std::string& data, const std::string& key) {
        auto pos = data.find("\"" + key + "\":\"");
        if (pos == std::string::npos)
            return "";
        pos = data.find("\":\"", pos) + 3;
        auto end = data.find("\"", pos);
        return data.substr(pos, end - pos);
    }
};

class CollaborationProtocol {
 public:
    static CollaborationProtocol& getInstance() {
        static CollaborationProtocol instance;
        return instance;
    }

    void initialize(int port = 8081) {
        auto& wsServer = WebSocketServer::getInstance();

        // Set up WebSocket callbacks
        wsServer.onConnect([this](WebSocketConnection* conn) { handleConnection(conn); });

        wsServer.onDisconnect([this](WebSocketConnection* conn) { handleDisconnection(conn); });

        wsServer.onMessage([this](const std::string& message, WebSocketConnection* conn, bool binary) {
            if (!binary) {
                handleMessage(message, conn);
            }
        });

        // Set up collaborative session callbacks
        auto& session = CollaborativeSession::getInstance();

        session.onOperation(
            [this](const DocumentOperation& op, const std::string& docId) { broadcastOperation(op, docId); });

        session.onUserJoin(
            [this](const UserSession& user, const std::string& docId) { broadcastUserJoined(user, docId); });

        session.onUserLeave(
            [this](const std::string& userId, const std::string& docId) { broadcastUserLeft(userId, docId); });

        session.onCursorUpdate([this](const std::string& userId, const Position& pos, const std::string& docId) {
            broadcastCursorUpdate(userId, pos, docId);
        });

        // Start WebSocket server
        wsServer.start(port);
    }

    void shutdown() {
        auto& wsServer = WebSocketServer::getInstance();
        wsServer.stop();

        std::lock_guard<std::mutex> lock(connectionsMutex_);
        connections_.clear();
        userConnections_.clear();
    }

    // Send document state to specific user
    void sendDocumentState(const std::string& userId, const std::string& documentId) {
        auto& session = CollaborativeSession::getInstance();
        std::string content = session.getDocumentContent(documentId);
        auto users = session.getActiveUsers(documentId);

        ProtocolMessage msg;
        msg.type = MessageType::DOCUMENT_STATE;
        msg.documentId = documentId;
        msg.userId = userId;

        // Create document state data
        std::string stateData = "{";
        stateData += "\"content\":\"" + detail::escapeJsonString(content) + "\",";
        stateData += "\"users\":[";
        for (size_t i = 0; i < users.size(); ++i) {
            if (i > 0)
                stateData += ",";
            stateData += "{\"userId\":\"" + users[i].userId + "\",";
            stateData += "\"userName\":\"" + users[i].userName + "\",";
            stateData += "\"cursorLine\":" + std::to_string(users[i].cursorPosition.line) + ",";
            stateData += "\"cursorChar\":" + std::to_string(users[i].cursorPosition.character) + "}";
        }
        stateData += "]}";
        msg.data = stateData;

        sendToUser(userId, msg.serialize());
    }

 private:
    CollaborationProtocol() = default;

    void handleConnection(WebSocketConnection* conn) {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        connections_.insert(conn);
    }

    void handleDisconnection(WebSocketConnection* conn) {
        std::lock_guard<std::mutex> lock(connectionsMutex_);

        // Find and remove user from all documents
        std::string disconnectedUserId;
        for (auto it = userConnections_.begin(); it != userConnections_.end(); ++it) {
            if (it->second == conn) {
                disconnectedUserId = it->first;
                userConnections_.erase(it);
                break;
            }
        }

        connections_.erase(conn);

        // Remove user from all collaborative sessions
        if (!disconnectedUserId.empty()) {
            auto& session = CollaborativeSession::getInstance();
            // Use the session reference to avoid unused variable warning
            static_cast<void>(session);
            // Note: In a real implementation, we'd track which documents the user is in
            // For now, we'll let the cleanup routine handle inactive users
        }
    }

    void handleMessage(const std::string& message, WebSocketConnection* conn) {
        try {
            ProtocolMessage msg = ProtocolMessage::deserialize(message);

            switch (msg.type) {
                case MessageType::JOIN_DOCUMENT:
                    handleJoinDocument(msg, conn);
                    break;
                case MessageType::LEAVE_DOCUMENT:
                    handleLeaveDocument(msg, conn);
                    break;
                case MessageType::DOCUMENT_OPERATION:
                    handleDocumentOperation(msg, conn);
                    break;
                case MessageType::CURSOR_UPDATE:
                    handleCursorUpdate(msg, conn);
                    break;
                default:
                    sendError(conn, "Unknown message type");
                    break;
            }
        } catch (const std::exception& e) {
            sendError(conn, "Failed to parse message: " + std::string(e.what()));
        }
    }

    void handleJoinDocument(const ProtocolMessage& msg, WebSocketConnection* conn) {
        auto& session = CollaborativeSession::getInstance();

        // Associate user with connection
        {
            std::lock_guard<std::mutex> lock(connectionsMutex_);
            userConnections_[msg.userId] = conn;
        }

        // Create document if it doesn't exist
        if (session.getDocumentLines(msg.documentId).empty()) {
            session.createDocument(msg.documentId);
        }

        // Add user to document
        bool success = session.joinDocument(msg.userId, msg.userId, msg.documentId);
        if (success) {
            // Send current document state to the user
            sendDocumentState(msg.userId, msg.documentId);
        } else {
            sendError(conn, "Failed to join document");
        }
    }

    void handleLeaveDocument(const ProtocolMessage& msg, WebSocketConnection* conn) {
        auto& session = CollaborativeSession::getInstance();
        session.leaveDocument(msg.userId, msg.documentId);
    }

    void handleDocumentOperation(const ProtocolMessage& msg, WebSocketConnection* conn) {
        try {
            auto operation = DocumentOperation::deserialize(msg.data);
            auto& session = CollaborativeSession::getInstance();

            bool success = session.applyOperation(*operation, msg.documentId);
            if (!success) {
                sendError(conn, "Failed to apply operation");
            }
        } catch (const std::exception& e) {
            sendError(conn, "Invalid operation: " + std::string(e.what()));
        }
    }

    void handleCursorUpdate(const ProtocolMessage& msg, WebSocketConnection* conn) {
        try {
            // Parse cursor position from data
            Position pos = parseCursorPosition(msg.data);

            DocumentOperation cursorOp(OperationType::CURSOR_MOVE, msg.userId, pos);
            auto& session = CollaborativeSession::getInstance();
            session.applyOperation(cursorOp, msg.documentId);
        } catch (const std::exception& e) {
            sendError(conn, "Invalid cursor update: " + std::string(e.what()));
        }
    }

    void broadcastOperation(const DocumentOperation& op, const std::string& documentId) {
        ProtocolMessage msg;
        msg.type = MessageType::DOCUMENT_OPERATION;
        msg.documentId = documentId;
        msg.userId = op.getUserId();
        msg.data = op.serialize();

        broadcastToDocument(documentId, msg.serialize(), op.getUserId());
    }

    void broadcastUserJoined(const UserSession& user, const std::string& documentId) {
        ProtocolMessage msg;
        msg.type = MessageType::USER_JOINED;
        msg.documentId = documentId;
        msg.userId = user.userId;
        msg.data = user.userName;

        broadcastToDocument(documentId, msg.serialize(), user.userId);
    }

    void broadcastUserLeft(const std::string& userId, const std::string& documentId) {
        ProtocolMessage msg;
        msg.type = MessageType::USER_LEFT;
        msg.documentId = documentId;
        msg.userId = userId;

        broadcastToDocument(documentId, msg.serialize(), userId);
    }

    void broadcastCursorUpdate(const std::string& userId, const Position& pos, const std::string& documentId) {
        ProtocolMessage msg;
        msg.type = MessageType::CURSOR_UPDATE;
        msg.documentId = documentId;
        msg.userId = userId;
        msg.data = "{\"line\":" + std::to_string(pos.line) + ",\"character\":" + std::to_string(pos.character) + "}";

        broadcastToDocument(documentId, msg.serialize(), userId);
    }

    void broadcastToDocument(const std::string& documentId, const std::string& message,
                             const std::string& excludeUserId = "") {
        auto& session = CollaborativeSession::getInstance();
        auto users = session.getActiveUsers(documentId);

        std::lock_guard<std::mutex> lock(connectionsMutex_);
        for (const auto& user : users) {
            if (user.userId != excludeUserId) {
                auto connIt = userConnections_.find(user.userId);
                if (connIt != userConnections_.end()) {
                    connIt->second->send(message);
                }
            }
        }
    }

    void sendToUser(const std::string& userId, const std::string& message) {
        std::lock_guard<std::mutex> lock(connectionsMutex_);
        auto it = userConnections_.find(userId);
        if (it != userConnections_.end()) {
            it->second->send(message);
        }
    }

    void sendError(WebSocketConnection* conn, const std::string& error) {
        ProtocolMessage msg;
        msg.type = MessageType::ERROR_MESSAGE;
        msg.data = error;
        conn->send(msg.serialize());
    }

    Position parseCursorPosition(const std::string& data) {
        int line = detail::extractJsonInt(data, "line");
        int character = detail::extractJsonInt(data, "character");
        return Position(line, character);
    }

    std::mutex connectionsMutex_;
    std::set<WebSocketConnection*> connections_;
    std::map<std::string, WebSocketConnection*> userConnections_;
};

}  // namespace collaboration
}  // namespace bolt

#endif  // BOLT_COLLABORATION_COLLABORATION_PROTOCOL_HPP_
