#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

// Include all DrawKern subsystems
#include "bolt/drawkern/ai_integration.hpp"
#include "bolt/drawkern/dis_vm.hpp"
#include "bolt/drawkern/styx_protocol.hpp"
#include "bolt/drawkern/yacc_grammar.hpp"

// The protocol and glyph types, the client base class and the client for each platform
#include "bolt/drawkern/drawkern_client.hpp"
#include "bolt/drawkern/drawkern_types.hpp"
#include "bolt/drawkern/imgui_client.hpp"
#include "bolt/drawkern/terminal_client.hpp"
#include "bolt/drawkern/web_client.hpp"

namespace bolt {
namespace drawkern {

class DrawKernServer {
 public:
    explicit DrawKernServer(const std::string& listen_address);
    ~DrawKernServer();

    // Core DrawKern operations
    void render_glyph(const VMGlyph& glyph);
    void deploy_workbench(const AIWorkbenchGlyph& workbench);
    void send_command(const DrawKernCommand& cmd);

    // Styx protocol integration
    void serve_namespace(const std::string& path);
    void mount_remote(const std::string& address, const std::string& local_path);

    // AI integration
    void spawn_ai_session(const std::string& model, const AIWorkbenchGlyph& workbench);
    void handle_ai_completion(const std::string& code, const std::string& context);

    // Event handling
    void handle_client_input(const DrawKernCommand& input);
    void broadcast_to_clients(const DrawKernCommand& cmd);

    void run();

 private:
    std::string listen_address_;
    std::vector<std::unique_ptr<class DrawKernClient>> clients_;
    std::map<std::string, AIWorkbenchGlyph> active_workbenches_;

    // DIS VM management
    void start_dis_vm(const VMGlyph& glyph);
    void execute_limbo_code(const std::string& code);

    // Rendering engine
    void render_to_clients(const DrawKernCommand& cmd);
};

// Factory function to create AI workbench glyphs
AIWorkbenchGlyph create_bolt_ai_glyph();

}  // namespace drawkern
}  // namespace bolt
