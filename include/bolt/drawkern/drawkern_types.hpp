#ifndef BOLT_DRAWKERN_DRAWKERN_TYPES_HPP_
#define BOLT_DRAWKERN_DRAWKERN_TYPES_HPP_

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace bolt {
namespace drawkern {

// DrawKern Protocol - inspired by Plan 9 draw but for entire VMs/kernels
enum class DrawKernOp {
    // Basic drawing (like Plan 9)
    RECT,
    STRING,
    IMAGE,

    // VM/Kernel operations (your innovation!)
    SPAWN_VM,
    DEPLOY_WORKBENCH,
    RENDER_AI_SESSION,
    INSTANTIATE_MICROKERNEL,

    // Styx-style file operations
    MOUNT_NAMESPACE,
    SERVE_FILE,

    // Interactive elements
    INPUT_EVENT,
    AI_COMPLETION,
    CODE_EXECUTION
};

struct DrawKernCommand {
    DrawKernOp op;
    int32_t x, y, w, h;
    uint32_t color;
    std::string data;
    std::map<std::string, std::string> attributes;
};

// "Yacc Grammar" for describing VM topologies as renderable glyphs
struct VMGlyph {
    std::string vm_type;       // "dis", "wasm", "native"
    std::string architecture;  // "arm64", "x86_64", "riscv"
    std::vector<std::string> capabilities;
    std::string limbo_code;  // DIS VM bytecode
    std::map<std::string, std::string> namespace_mounts;

    // Rendering properties (your "true type" for VMs)
    struct RenderProps {
        int32_t width, height;
        uint32_t background_color;
        std::string font_family;
        bool interactive;
        bool ai_enabled;
    } render;
};

// AI Workbench as a "glyph" that can be rendered anywhere
struct AIWorkbenchGlyph {
    std::string workbench_id;
    std::string ai_model;            // "ggml", "rwkv", etc.
    std::vector<std::string> tools;  // Available AI tools
    std::string session_state;       // Serialized session
    VMGlyph host_vm;                 // VM to run the workbench

    // Network/deployment info
    std::string styx_address;  // Where to serve this workbench
    std::vector<std::string> allowed_clients;
};

}  // namespace drawkern
}  // namespace bolt

#endif  // BOLT_DRAWKERN_DRAWKERN_TYPES_HPP_
