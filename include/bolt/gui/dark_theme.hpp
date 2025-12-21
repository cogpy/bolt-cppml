#ifndef BOLT_GUI_DARK_THEME_HPP
#define BOLT_GUI_DARK_THEME_HPP

namespace bolt {
namespace gui {

/**
 * Apply bolt.diy inspired dark theme to ImGui
 * This theme features:
 * - Dark slate/charcoal backgrounds (#1a1a1f, #1e1e23)
 * - Blue accent colors for interactive elements
 * - Rounded corners for modern appearance
 * - Proper contrast for readability
 */
void ApplyBoltDarkTheme();

/**
 * Apply bolt.diy inspired light theme to ImGui (optional)
 */
void ApplyBoltLightTheme();

/**
 * Apply enhanced bolt.new/bolt.diy inspired dark theme
 * Features:
 * - Deep blue-gray charcoal backgrounds matching bolt.new
 * - Signature bright blue accent color (#4a9eff)
 * - Generous rounded corners and modern spacing
 * - Professional, clean aesthetic
 * - Optimized for AI IDE interface
 */
void ApplyEnhancedBoltDarkTheme();

/**
 * Apply bolt theme optimized for code editor
 * Tighter spacing, less rounding, monospace-friendly
 */
void ApplyBoltCodeEditorTheme();

/**
 * Apply high contrast theme for accessibility
 * Maximum contrast with black backgrounds and white text
 */
void ApplyBoltHighContrastTheme();

} // namespace gui
} // namespace bolt

#endif // BOLT_GUI_DARK_THEME_HPP
