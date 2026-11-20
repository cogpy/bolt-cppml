
#ifndef BOLT_NETWORK_NETWORK_COMMANDS_HPP_
#define BOLT_NETWORK_NETWORK_COMMANDS_HPP_

#include <string>

#ifdef _WIN32
// winsock2.h must come before windows.h, which otherwise pulls in the
// conflicting Winsock 1 declarations. Keep this order.
// clang-format off
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
// clang-format on
#define close closesocket
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include "bolt/core/bolt_commands.hpp"

namespace bolt {

class NetworkCommand : public Command {
 public:
    virtual ~NetworkCommand() = default;
    virtual bool connect(const std::string& host, int port) = 0;
    virtual void disconnect() = 0;
};

class TCPCommand : public NetworkCommand {
 public:
    TCPCommand() : sockfd_(-1) {}

    bool connect(const std::string& host, int port) override {
#ifdef _WIN32
        sockfd_ = static_cast<int>(socket(AF_INET, SOCK_STREAM, 0));
#else
        sockfd_ = socket(AF_INET, SOCK_STREAM, 0);
#endif
        if (sockfd_ < 0)
            return false;

        struct sockaddr_in serv_addr;
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);
        inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr);

        return ::connect(sockfd_, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) >= 0;
    }

    void disconnect() override {
        if (sockfd_ >= 0) {
            close(sockfd_);
            sockfd_ = -1;
        }
    }

    void execute() override {
        if (connect("0.0.0.0", 8080)) {
            // Example connection
            disconnect();
        }
    }

    std::string getDescription() const override { return "TCP Network Command"; }

 private:
    int sockfd_;
};

}  // namespace bolt

#endif  // BOLT_NETWORK_NETWORK_COMMANDS_HPP_
