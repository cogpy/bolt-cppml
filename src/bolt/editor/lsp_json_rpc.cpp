#include "bolt/editor/lsp_json_rpc.hpp"
#include <sstream>
#include <iostream>
#include <json/json.h>

namespace bolt {
namespace lsp {

// Basic JSON Value implementation
std::string JsonValue::toString() const {
    switch (type_) {
        case Null:
            return "null";
        case Bool:
            return boolValue_ ? "true" : "false";
        case Number:
            return std::to_string(numberValue_);
        case String: {
            std::ostringstream oss;
            oss << "\"";
            // Basic string escaping
            for (char c : stringValue_) {
                switch (c) {
                    case '"': oss << "\\\""; break;
                    case '\\': oss << "\\\\"; break;
                    case '\n': oss << "\\n"; break;
                    case '\r': oss << "\\r"; break;
                    case '\t': oss << "\\t"; break;
                    default: oss << c; break;
                }
            }
            oss << "\"";
            return oss.str();
        }
        case Array: {
            std::ostringstream oss;
            oss << "[";
            for (size_t i = 0; i < arrayValue_.size(); ++i) {
                if (i > 0) oss << ",";
                oss << arrayValue_[i]->toString();
            }
            oss << "]";
            return oss.str();
        }
        case Object: {
            std::ostringstream oss;
            oss << "{";
            bool first = true;
            for (const auto& pair : objectValue_) {
                if (!first) oss << ",";
                first = false;
                oss << "\"" << pair.first << "\":" << pair.second->toString();
            }
            oss << "}";
            return oss.str();
        }
    }
    return "null";
}

// Helper function to convert Json::Value to JsonValue
static std::shared_ptr<JsonValue> convertFromJsonCpp(const Json::Value& jval) {
    auto value = std::make_shared<JsonValue>();
    
    if (jval.isNull()) {
        return value; // null
    }
    
    if (jval.isBool()) {
        value->setBool(jval.asBool());
        return value;
    }
    
    if (jval.isInt() || jval.isInt64() || jval.isUInt() || jval.isUInt64() || jval.isDouble()) {
        value->setNumber(jval.asDouble());
        return value;
    }
    
    if (jval.isString()) {
        value->setString(jval.asString());
        return value;
    }
    
    if (jval.isArray()) {
        value->setArray();
        for (Json::ArrayIndex i = 0; i < jval.size(); ++i) {
            value->addArrayElement(convertFromJsonCpp(jval[i]));
        }
        return value;
    }
    
    if (jval.isObject()) {
        value->setObject();
        for (const auto& key : jval.getMemberNames()) {
            value->setProperty(key, convertFromJsonCpp(jval[key]));
        }
        return value;
    }
    
    return value; // null for unknown types
}

std::shared_ptr<JsonValue> JsonValue::fromString(const std::string& json) {
    if (json.empty()) {
        return std::make_shared<JsonValue>();
    }
    
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    std::istringstream stream(json);
    
    if (!Json::parseFromStream(builder, stream, &root, &errors)) {
        // Parse error - return null
        return std::make_shared<JsonValue>();
    }
    
    return convertFromJsonCpp(root);
}

// JSON-RPC Handler implementation
void JsonRpcHandler::registerRequestHandler(const std::string& method, RequestHandler handler) {
    requestHandlers_[method] = handler;
}

void JsonRpcHandler::registerNotificationHandler(const std::string& method, NotificationHandler handler) {
    notificationHandlers_[method] = handler;
}

std::string JsonRpcHandler::processMessage(const std::string& message) {
    try {
        auto json = JsonValue::fromString(message);
        if (!json || json->getType() != JsonValue::Object) {
            return createErrorResponse("", -32700, "Parse error");
        }
        
        auto methodProperty = json->getProperty("method");
        auto idProperty = json->getProperty("id");
        
        if (!methodProperty || methodProperty->getType() != JsonValue::String) {
            return createErrorResponse(idProperty ? idProperty->asString() : "", -32600, "Invalid Request");
        }
        
        std::string method = methodProperty->asString();
        auto paramsProperty = json->getProperty("params");
        
        // Check if it's a notification (no id) or request (has id)
        if (!idProperty) {
            // Notification
            auto it = notificationHandlers_.find(method);
            if (it != notificationHandlers_.end()) {
                it->second(method, paramsProperty);
            }
            return ""; // No response for notifications
        } else {
            // Request
            std::string id = idProperty->asString();
            auto it = requestHandlers_.find(method);
            if (it != requestHandlers_.end()) {
                auto result = it->second(method, paramsProperty);
                return createResponse(id, result);
            } else {
                return createErrorResponse(id, -32601, "Method not found");
            }
        }
    } catch (const std::exception& e) {
        return createErrorResponse("", -32603, "Internal error");
    }
}

std::string JsonRpcHandler::createRequest(const std::string& method, std::shared_ptr<JsonValue> params, const std::string& id) {
    auto request = std::make_shared<JsonValue>();
    request->setObject();
    request->setProperty("jsonrpc", std::make_shared<JsonValue>("2.0"));
    request->setProperty("method", std::make_shared<JsonValue>(method));
    if (!id.empty()) {
        request->setProperty("id", std::make_shared<JsonValue>(id));
    }
    if (params) {
        request->setProperty("params", params);
    }
    return request->toString();
}

std::string JsonRpcHandler::createNotification(const std::string& method, std::shared_ptr<JsonValue> params) {
    return createRequest(method, params, ""); // No id for notifications
}

std::string JsonRpcHandler::createResponse(const std::string& id, std::shared_ptr<JsonValue> result) {
    auto response = std::make_shared<JsonValue>();
    response->setObject();
    response->setProperty("jsonrpc", std::make_shared<JsonValue>("2.0"));
    response->setProperty("id", std::make_shared<JsonValue>(id));
    if (result) {
        response->setProperty("result", result);
    } else {
        response->setProperty("result", std::make_shared<JsonValue>()); // null
    }
    return response->toString();
}

std::string JsonRpcHandler::createErrorResponse(const std::string& id, int code, const std::string& message) {
    auto error = std::make_shared<JsonValue>();
    error->setObject();
    error->setProperty("code", std::make_shared<JsonValue>(static_cast<double>(code)));
    error->setProperty("message", std::make_shared<JsonValue>(message));
    
    auto response = std::make_shared<JsonValue>();
    response->setObject();
    response->setProperty("jsonrpc", std::make_shared<JsonValue>("2.0"));
    response->setProperty("id", std::make_shared<JsonValue>(id));
    response->setProperty("error", error);
    
    return response->toString();
}

} // namespace lsp
} // namespace bolt