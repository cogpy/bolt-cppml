#ifndef BOLT_DRAWKERN_WEB_CLIENT_HPP_
#define BOLT_DRAWKERN_WEB_CLIENT_HPP_

#include <cstdint>
#include <string>

#include "bolt/drawkern/drawkern_client.hpp"

namespace bolt {
namespace drawkern {

// DrawKern client that renders as HTML/JavaScript for a browser.
class WebDrawKernClient : public DrawKernClient {
 public:
    explicit WebDrawKernClient(const std::string& server_address) : DrawKernClient(server_address) {}

    void render_rect(int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color) override;
    void render_text(int32_t x, int32_t y, const std::string& text) override;
    void render_ai_interface(const AIWorkbenchGlyph& workbench) override;

    std::string generate_html() const;
    std::string generate_javascript() const;
};

}  // namespace drawkern
}  // namespace bolt

#endif  // BOLT_DRAWKERN_WEB_CLIENT_HPP_
