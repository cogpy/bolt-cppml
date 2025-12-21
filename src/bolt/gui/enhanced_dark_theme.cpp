#include "bolt/gui/dark_theme.hpp"

#ifdef BOLT_HAVE_IMGUI
#include <imgui.h>
#endif

namespace bolt {
namespace gui {

// Enhanced dark theme inspired by bolt.diy/bolt.new interface
void ApplyEnhancedBoltDarkTheme() {
#ifdef BOLT_HAVE_IMGUI
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    
    // ============================================================================
    // COLOR PALETTE - Bolt.diy/Bolt.new Inspired
    // ============================================================================
    
    // Primary dark backgrounds (deep blue-gray charcoal)
    const ImVec4 bg_primary       = ImVec4(0.10f, 0.11f, 0.14f, 1.00f);  // #1a1c23
    const ImVec4 bg_secondary     = ImVec4(0.12f, 0.13f, 0.16f, 1.00f);  // #1e2129
    const ImVec4 bg_tertiary      = ImVec4(0.14f, 0.15f, 0.18f, 1.00f);  // #24262e
    const ImVec4 bg_elevated      = ImVec4(0.16f, 0.17f, 0.20f, 1.00f);  // #292b33
    
    // Accent colors (bright blue - bolt.new signature color)
    const ImVec4 accent_blue      = ImVec4(0.29f, 0.62f, 1.00f, 1.00f);  // #4a9eff
    const ImVec4 accent_blue_hover= ImVec4(0.35f, 0.68f, 1.00f, 1.00f);  // #59adff
    const ImVec4 accent_blue_active=ImVec4(0.24f, 0.56f, 0.95f, 1.00f); // #3d8ff2
    const ImVec4 accent_blue_dim  = ImVec4(0.29f, 0.62f, 1.00f, 0.30f); // Semi-transparent
    
    // Text colors
    const ImVec4 text_primary     = ImVec4(0.92f, 0.93f, 0.95f, 1.00f);  // #ebecf2
    const ImVec4 text_secondary   = ImVec4(0.70f, 0.72f, 0.76f, 1.00f);  // #b3b8c2
    const ImVec4 text_disabled    = ImVec4(0.50f, 0.52f, 0.56f, 1.00f);  // #80858f
    
    // Border and separator colors
    const ImVec4 border_default   = ImVec4(0.22f, 0.24f, 0.28f, 1.00f);  // #383d47
    const ImVec4 border_subtle    = ImVec4(0.18f, 0.20f, 0.24f, 1.00f);  // #2e333d
    
    // Success, warning, error colors
    const ImVec4 success_green    = ImVec4(0.26f, 0.78f, 0.50f, 1.00f);  // #43c780
    const ImVec4 warning_yellow   = ImVec4(0.95f, 0.77f, 0.26f, 1.00f);  // #f2c443
    const ImVec4 error_red        = ImVec4(0.95f, 0.38f, 0.38f, 1.00f);  // #f26161
    
    // ============================================================================
    // APPLY COLORS TO IMGUI ELEMENTS
    // ============================================================================
    
    // Window backgrounds
    colors[ImGuiCol_WindowBg]               = bg_primary;
    colors[ImGuiCol_ChildBg]                = bg_secondary;
    colors[ImGuiCol_PopupBg]                = bg_elevated;
    
    // Borders
    colors[ImGuiCol_Border]                 = border_default;
    colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    
    // Frame backgrounds (input fields, etc.)
    colors[ImGuiCol_FrameBg]                = bg_tertiary;
    colors[ImGuiCol_FrameBgHovered]         = bg_elevated;
    colors[ImGuiCol_FrameBgActive]          = ImVec4(0.18f, 0.19f, 0.22f, 1.00f);
    
    // Title bar
    colors[ImGuiCol_TitleBg]                = bg_primary;
    colors[ImGuiCol_TitleBgActive]          = bg_secondary;
    colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.08f, 0.09f, 0.12f, 0.75f);
    
    // Menu bar
    colors[ImGuiCol_MenuBarBg]              = bg_primary;
    
    // Scrollbar
    colors[ImGuiCol_ScrollbarBg]            = bg_primary;
    colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.30f, 0.32f, 0.36f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.38f, 0.40f, 0.44f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.46f, 0.48f, 0.52f, 1.00f);
    
    // Checkboxes and radio buttons
    colors[ImGuiCol_CheckMark]              = accent_blue;
    
    // Sliders
    colors[ImGuiCol_SliderGrab]             = accent_blue;
    colors[ImGuiCol_SliderGrabActive]       = accent_blue_hover;
    
    // Buttons - PRIMARY ACTION (bolt.new style)
    colors[ImGuiCol_Button]                 = accent_blue;
    colors[ImGuiCol_ButtonHovered]          = accent_blue_hover;
    colors[ImGuiCol_ButtonActive]           = accent_blue_active;
    
    // Headers (for tree nodes, collapsing headers, etc.)
    colors[ImGuiCol_Header]                 = accent_blue_dim;
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.29f, 0.62f, 1.00f, 0.50f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.29f, 0.62f, 1.00f, 0.70f);
    
    // Separator
    colors[ImGuiCol_Separator]              = border_subtle;
    colors[ImGuiCol_SeparatorHovered]       = accent_blue_dim;
    colors[ImGuiCol_SeparatorActive]        = accent_blue;
    
    // Resize grip
    colors[ImGuiCol_ResizeGrip]             = accent_blue_dim;
    colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.29f, 0.62f, 1.00f, 0.50f);
    colors[ImGuiCol_ResizeGripActive]       = accent_blue;
    
    // Tabs
    colors[ImGuiCol_Tab]                    = bg_secondary;
    colors[ImGuiCol_TabHovered]             = accent_blue_dim;
    colors[ImGuiCol_TabActive]              = accent_blue;
    colors[ImGuiCol_TabUnfocused]           = bg_primary;
    colors[ImGuiCol_TabUnfocusedActive]     = bg_tertiary;
    
    // Docking
    #ifdef IMGUI_HAS_DOCK
    colors[ImGuiCol_DockingPreview]         = ImVec4(0.29f, 0.62f, 1.00f, 0.60f);
    colors[ImGuiCol_DockingEmptyBg]         = bg_primary;
    #endif
    
    // Plot colors
    colors[ImGuiCol_PlotLines]              = accent_blue;
    colors[ImGuiCol_PlotLinesHovered]       = accent_blue_hover;
    colors[ImGuiCol_PlotHistogram]          = accent_blue;
    colors[ImGuiCol_PlotHistogramHovered]   = accent_blue_hover;
    
    // Table colors
    colors[ImGuiCol_TableHeaderBg]          = bg_tertiary;
    colors[ImGuiCol_TableBorderStrong]      = border_default;
    colors[ImGuiCol_TableBorderLight]       = border_subtle;
    colors[ImGuiCol_TableRowBg]             = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImGuiCol_TableRowBgAlt]          = ImVec4(1.00f, 1.00f, 1.00f, 0.03f);
    
    // Text colors
    colors[ImGuiCol_Text]                   = text_primary;
    colors[ImGuiCol_TextDisabled]           = text_disabled;
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.29f, 0.62f, 1.00f, 0.35f);
    
    // Drag and drop
    colors[ImGuiCol_DragDropTarget]         = accent_blue;
    
    // Navigation
    colors[ImGuiCol_NavHighlight]           = accent_blue;
    colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
    
    // Modal window dimming
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.65f);
    
    // ============================================================================
    // STYLE SETTINGS - Modern, Professional Look
    // ============================================================================
    
    // Rounding (generous, modern look like bolt.new)
    style.WindowRounding    = 12.0f;
    style.ChildRounding     = 10.0f;
    style.FrameRounding     = 8.0f;
    style.PopupRounding     = 10.0f;
    style.ScrollbarRounding = 12.0f;
    style.GrabRounding      = 8.0f;
    style.TabRounding       = 8.0f;
    
    // Border sizes
    style.WindowBorderSize  = 1.0f;
    style.ChildBorderSize   = 1.0f;
    style.PopupBorderSize   = 1.0f;
    style.FrameBorderSize   = 0.0f;
    style.TabBorderSize     = 0.0f;
    
    // Padding and spacing (generous, comfortable)
    style.WindowPadding     = ImVec2(16.0f, 16.0f);
    style.FramePadding      = ImVec2(12.0f, 8.0f);
    style.ItemSpacing       = ImVec2(12.0f, 8.0f);
    style.ItemInnerSpacing  = ImVec2(8.0f, 6.0f);
    style.IndentSpacing     = 24.0f;
    style.ScrollbarSize     = 16.0f;
    style.GrabMinSize       = 12.0f;
    
    // Window title alignment
    style.WindowTitleAlign  = ImVec2(0.5f, 0.5f);  // Center title
    
    // Button text alignment
    style.ButtonTextAlign   = ImVec2(0.5f, 0.5f);  // Center button text
    
    // Additional modern touches
    style.WindowMenuButtonPosition = ImGuiDir_None;  // No collapse button
    style.ColorButtonPosition = ImGuiDir_Right;
    
    // Anti-aliasing
    style.AntiAliasedLines = true;
    style.AntiAliasedLinesUseTex = true;
    style.AntiAliasedFill = true;
    
#endif
}

// Additional theme variants for different contexts

void ApplyBoltCodeEditorTheme() {
#ifdef BOLT_HAVE_IMGUI
    // Start with enhanced dark theme
    ApplyEnhancedBoltDarkTheme();
    
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Adjust for code editor - tighter spacing, monospace-friendly
    style.ItemSpacing       = ImVec2(8.0f, 4.0f);
    style.FramePadding      = ImVec2(8.0f, 4.0f);
    style.WindowPadding     = ImVec2(8.0f, 8.0f);
    
    // Less rounding for code editor (more traditional IDE feel)
    style.FrameRounding     = 4.0f;
    style.WindowRounding    = 6.0f;
#endif
}

void ApplyBoltHighContrastTheme() {
#ifdef BOLT_HAVE_IMGUI
    ImGuiStyle& style = ImGui::GetStyle();
    ImVec4* colors = style.Colors;
    
    // High contrast theme for accessibility
    const ImVec4 bg_black         = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    const ImVec4 bg_dark_gray     = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    const ImVec4 text_white       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    const ImVec4 accent_cyan      = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
    const ImVec4 border_white     = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    
    colors[ImGuiCol_WindowBg]               = bg_black;
    colors[ImGuiCol_ChildBg]                = bg_dark_gray;
    colors[ImGuiCol_PopupBg]                = bg_black;
    colors[ImGuiCol_Border]                 = border_white;
    colors[ImGuiCol_FrameBg]                = bg_dark_gray;
    colors[ImGuiCol_Button]                 = accent_cyan;
    colors[ImGuiCol_ButtonHovered]          = ImVec4(0.20f, 1.00f, 1.00f, 1.00f);
    colors[ImGuiCol_ButtonActive]           = ImVec4(0.00f, 0.80f, 0.80f, 1.00f);
    colors[ImGuiCol_Text]                   = text_white;
    colors[ImGuiCol_Header]                 = ImVec4(0.00f, 1.00f, 1.00f, 0.40f);
    colors[ImGuiCol_HeaderHovered]          = ImVec4(0.00f, 1.00f, 1.00f, 0.60f);
    colors[ImGuiCol_HeaderActive]           = ImVec4(0.00f, 1.00f, 1.00f, 0.80f);
    
    // Apply same style settings as enhanced theme
    style.WindowRounding    = 12.0f;
    style.FrameRounding     = 8.0f;
    style.WindowBorderSize  = 2.0f;  // Thicker borders for high contrast
    style.FrameBorderSize   = 1.0f;
#endif
}

} // namespace gui
} // namespace bolt
