#ifndef BOLT_DRAWKERN_DRAWKERN_CLIENT_HPP_
#define BOLT_DRAWKERN_DRAWKERN_CLIENT_HPP_

#include <cstdint>
#include <string>

#include "bolt/drawkern/drawkern_types.hpp"

namespace bolt {
namespace drawkern {

// Base of the DrawKern clients. The platform implementations are in terminal_client.hpp, web_client.hpp and
// imgui_client.hpp.
class DrawKernClient {
 public:
    explicit DrawKernClient(const std::string& server_address);
    ~DrawKernClient();

    // Connect to DrawKern server
    bool connect();
    void disconnect();

    // Receive and render "glyphs" (VMs, workbenches, etc.)
    void handle_glyph(const VMGlyph& glyph);
    void instantiate_workbench(const AIWorkbenchGlyph& workbench);

    // Local rendering (could be terminal, web, native, anything!)
    virtual void render_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) = 0;
    virtual void render_text(int32_t x, int32_t y, const std::string& text) = 0;
    virtual void render_ai_interface(const AIWorkbenchGlyph& workbench) = 0;

    // Input handling
    void send_input(const DrawKernCommand& input);

 private:
    std::string server_address_;
    bool connected_;
};

}  // namespace drawkern
}  // namespace bolt

#endif  // BOLT_DRAWKERN_DRAWKERN_CLIENT_HPP_
