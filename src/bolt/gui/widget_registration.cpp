#include "bolt/gui/widget_registration.hpp"

#include <optional>

#ifdef BOLT_HAVE_IMGUI
#include <imgui.h>
#endif

namespace bolt {
namespace gui {

#ifdef BOLT_HAVE_IMGUI
namespace {

// The per-widget colors and metrics in which a built-in theme differs from its base style.
struct BuiltInWidgetStyles {
    ImVec4 buttonBackground;
    ImVec4 buttonHover;
    ImVec4 buttonActive;
    float buttonCornerRadius = 0.0f;
    ImVec2 buttonPadding;
    ImVec4 inputBackground;
    ImVec4 inputBorder;
    float inputCornerRadius = 0.0f;
    ImVec2 inputPadding;
    std::optional<float> controlBorderWidth;  // Button and text input border; unset keeps the base style's width
    ImVec4 panelBackground;
    ImVec4 panelBorder;
    float panelBorderWidth = 0.0f;
    float panelCornerRadius = 0.0f;
    ImVec2 panelPadding;
    ImVec2 labelPadding;
};

// The accent-colored button states, corner radii and paddings that the Dark and Light themes have in common;
// the surface colors are left for the theme to fill in.
BuiltInWidgetStyles createStandardWidgetStyles() {
    BuiltInWidgetStyles styles;
    styles.buttonHover = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
    styles.buttonActive = ImVec4(0.06f, 0.53f, 0.98f, 1.00f);
    styles.buttonCornerRadius = 3.0f;
    styles.buttonPadding = ImVec2(8.0f, 4.0f);
    styles.inputCornerRadius = 2.0f;
    styles.inputPadding = ImVec2(6.0f, 3.0f);
    styles.panelBorderWidth = 1.0f;
    styles.panelCornerRadius = 4.0f;
    styles.panelPadding = ImVec2(10.0f, 10.0f);
    styles.labelPadding = ImVec2(2.0f, 2.0f);
    return styles;
}

// Makes baseStyle the default style of a built-in theme, then registers its Button, TextInput/TextArea, Panel and
// Label styles, each derived from baseStyle.
void applyBuiltInThemeStyles(WidgetTheme& theme, const WidgetStyle& baseStyle, const BuiltInWidgetStyles& styles) {
    theme.setDefaultStyle(baseStyle);

    // Button specific styling
    WidgetStyle buttonStyle = baseStyle;
    buttonStyle.backgroundColor = styles.buttonBackground;
    buttonStyle.hoverColor = styles.buttonHover;
    buttonStyle.activeColor = styles.buttonActive;
    buttonStyle.cornerRadius = styles.buttonCornerRadius;
    buttonStyle.padding = styles.buttonPadding;
    if (styles.controlBorderWidth) {
        buttonStyle.borderWidth = styles.controlBorderWidth;
    }
    theme.setWidgetStyle("Button", buttonStyle);

    // TextInput specific styling
    WidgetStyle textInputStyle = baseStyle;
    textInputStyle.backgroundColor = styles.inputBackground;
    textInputStyle.borderColor = styles.inputBorder;
    textInputStyle.cornerRadius = styles.inputCornerRadius;
    textInputStyle.padding = styles.inputPadding;
    if (styles.controlBorderWidth) {
        textInputStyle.borderWidth = styles.controlBorderWidth;
    }
    theme.setWidgetStyle("TextInput", textInputStyle);
    theme.setWidgetStyle("TextArea", textInputStyle);

    // Panel specific styling
    WidgetStyle panelStyle = baseStyle;
    panelStyle.backgroundColor = styles.panelBackground;
    panelStyle.borderColor = styles.panelBorder;
    panelStyle.borderWidth = styles.panelBorderWidth;
    panelStyle.cornerRadius = styles.panelCornerRadius;
    panelStyle.padding = styles.panelPadding;
    theme.setWidgetStyle("Panel", panelStyle);

    // Label styling
    WidgetStyle labelStyle = baseStyle;
    labelStyle.padding = styles.labelPadding;
    theme.setWidgetStyle("Label", labelStyle);
}

}  // namespace
#endif

// Static member initialization
bool WidgetRegistration::registered_ = false;
bool WidgetFrameworkInit::initialized_ = false;

// WidgetRegistration implementation
void WidgetRegistration::registerAllWidgets() {
    if (registered_)
        return;

    registerCoreWidgets();
    registerInputWidgets();
    registerDisplayWidgets();
    registerContainerWidgets();

    registered_ = true;
}

void WidgetRegistration::registerCoreWidgets() {
    auto& factory = WidgetFactory::getInstance();

    factory.registerWidget<Widget>("Widget");
    factory.registerWidget<WidgetContainer>("WidgetContainer");
}

void WidgetRegistration::registerInputWidgets() {
    auto& factory = WidgetFactory::getInstance();

    factory.registerWidget<Button>("Button");
    factory.registerWidget<TextInput>("TextInput");
    factory.registerWidget<TextArea>("TextArea");
    factory.registerWidget<Checkbox>("Checkbox");
    factory.registerWidget<RadioButton>("RadioButton");
    factory.registerWidget<ComboBox>("ComboBox");
    factory.registerWidget<Slider>("Slider");
}

void WidgetRegistration::registerDisplayWidgets() {
    auto& factory = WidgetFactory::getInstance();

    factory.registerWidget<Label>("Label");
    factory.registerWidget<ProgressBar>("ProgressBar");
}

void WidgetRegistration::registerContainerWidgets() {
    auto& factory = WidgetFactory::getInstance();

    factory.registerWidget<Panel>("Panel");
    factory.registerWidget<TabContainer>("TabContainer");
    factory.registerWidget<Splitter>("Splitter");
}

std::vector<std::string> WidgetRegistration::getRegisteredTypes() {
    ensureRegistered();
    return WidgetFactory::getInstance().getRegisteredTypes();
}

std::shared_ptr<Widget> WidgetRegistration::createWidget(const std::string& typeName, const std::string& id) {
    ensureRegistered();
    return WidgetFactory::getInstance().createWidget(typeName, id);
}

void WidgetRegistration::ensureRegistered() {
    if (!registered_) {
        registerAllWidgets();
    }
}

// DefaultThemes implementation
void DefaultThemes::registerAllThemes() {
    auto& themeManager = ThemeManager::getInstance();

    themeManager.addTheme(createDarkTheme());
    themeManager.addTheme(createLightTheme());
    themeManager.addTheme(createHighContrastTheme());

    // Set dark theme as default
    themeManager.setCurrentTheme("Dark");
}

std::shared_ptr<WidgetTheme> DefaultThemes::createDarkTheme() {
    auto theme = std::make_shared<WidgetTheme>("Dark");

#ifdef BOLT_HAVE_IMGUI
    // Base colors for dark theme
    ImVec4 backgroundColor(0.12f, 0.12f, 0.14f, 1.00f);
    ImVec4 textColor(0.90f, 0.90f, 0.90f, 1.00f);
    ImVec4 borderColor(0.25f, 0.25f, 0.27f, 0.50f);
    ImVec4 accentColor(0.26f, 0.59f, 0.98f, 1.00f);

    WidgetStyle baseStyle = createBaseStyle(backgroundColor, textColor, borderColor, accentColor);

    // Widget specific styling: the standard metrics with this theme's surface colors
    BuiltInWidgetStyles styles = createStandardWidgetStyles();
    styles.buttonBackground = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
    styles.inputBackground = ImVec4(0.15f, 0.15f, 0.17f, 1.00f);
    styles.inputBorder = ImVec4(0.30f, 0.30f, 0.32f, 1.00f);
    styles.panelBackground = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
    styles.panelBorder = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
    applyBuiltInThemeStyles(*theme, baseStyle, styles);

#endif

    return theme;
}

std::shared_ptr<WidgetTheme> DefaultThemes::createLightTheme() {
    auto theme = std::make_shared<WidgetTheme>("Light");

#ifdef BOLT_HAVE_IMGUI
    // Base colors for light theme
    ImVec4 backgroundColor(0.95f, 0.95f, 0.95f, 1.00f);
    ImVec4 textColor(0.10f, 0.10f, 0.10f, 1.00f);
    ImVec4 borderColor(0.70f, 0.70f, 0.70f, 0.50f);
    ImVec4 accentColor(0.26f, 0.59f, 0.98f, 1.00f);

    WidgetStyle baseStyle = createBaseStyle(backgroundColor, textColor, borderColor, accentColor);

    // Widget specific styling: the standard metrics with this theme's surface colors
    BuiltInWidgetStyles styles = createStandardWidgetStyles();
    styles.buttonBackground = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
    styles.inputBackground = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    styles.inputBorder = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    styles.panelBackground = ImVec4(0.98f, 0.98f, 0.98f, 1.00f);
    styles.panelBorder = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    applyBuiltInThemeStyles(*theme, baseStyle, styles);

#endif

    return theme;
}

std::shared_ptr<WidgetTheme> DefaultThemes::createHighContrastTheme() {
    auto theme = std::make_shared<WidgetTheme>("HighContrast");

#ifdef BOLT_HAVE_IMGUI
    // Base colors for high contrast theme
    ImVec4 backgroundColor(0.00f, 0.00f, 0.00f, 1.00f);
    ImVec4 textColor(1.00f, 1.00f, 1.00f, 1.00f);
    ImVec4 borderColor(1.00f, 1.00f, 1.00f, 1.00f);
    ImVec4 accentColor(1.00f, 1.00f, 0.00f, 1.00f);

    WidgetStyle baseStyle = createBaseStyle(backgroundColor, textColor, borderColor, accentColor);

    // Widget specific styling
    BuiltInWidgetStyles styles;
    styles.buttonBackground = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    styles.buttonHover = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    styles.buttonActive = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    styles.buttonCornerRadius = 0.0f;  // Sharp edges for high contrast
    styles.buttonPadding = ImVec2(10.0f, 6.0f);
    styles.inputBackground = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    styles.inputBorder = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    styles.inputCornerRadius = 0.0f;
    styles.inputPadding = ImVec2(8.0f, 4.0f);
    styles.panelBackground = ImVec4(0.05f, 0.05f, 0.05f, 1.00f);
    styles.panelBorder = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    styles.panelBorderWidth = 2.0f;
    styles.panelCornerRadius = 0.0f;
    styles.panelPadding = ImVec2(15.0f, 15.0f);
    styles.labelPadding = ImVec2(4.0f, 4.0f);
    styles.controlBorderWidth = 2.0f;
    applyBuiltInThemeStyles(*theme, baseStyle, styles);

#endif

    return theme;
}

std::shared_ptr<WidgetTheme> DefaultThemes::createCustomTheme(const std::string& name, const ImVec4& backgroundColor,
                                                              const ImVec4& textColor, const ImVec4& accentColor) {
    auto theme = std::make_shared<WidgetTheme>(name);

#ifdef BOLT_HAVE_IMGUI
    ImVec4 borderColor((backgroundColor.x + textColor.x) * 0.5f, (backgroundColor.y + textColor.y) * 0.5f,
                       (backgroundColor.z + textColor.z) * 0.5f, 0.7f);

    WidgetStyle baseStyle = createBaseStyle(backgroundColor, textColor, borderColor, accentColor);
    theme->setDefaultStyle(baseStyle);

    // Create variations for different widget types
    WidgetStyle buttonStyle = baseStyle;
    buttonStyle.hoverColor = accentColor;
    buttonStyle.activeColor = ImVec4(accentColor.x * 0.8f, accentColor.y * 0.8f, accentColor.z * 0.8f, accentColor.w);
    theme->setWidgetStyle("Button", buttonStyle);

    WidgetStyle inputStyle = baseStyle;
    inputStyle.backgroundColor =
        ImVec4(backgroundColor.x * 0.9f, backgroundColor.y * 0.9f, backgroundColor.z * 0.9f, backgroundColor.w);
    theme->setWidgetStyle("TextInput", inputStyle);
    theme->setWidgetStyle("TextArea", inputStyle);

#endif

    return theme;
}

WidgetStyle DefaultThemes::createBaseStyle(const ImVec4& backgroundColor, const ImVec4& textColor,
                                           const ImVec4& borderColor, const ImVec4& accentColor) {
    WidgetStyle style;

#ifdef BOLT_HAVE_IMGUI
    style.backgroundColor = backgroundColor;
    style.textColor = textColor;
    style.borderColor = borderColor;
    style.hoverColor = accentColor;
    style.activeColor = ImVec4(accentColor.x * 0.7f, accentColor.y * 0.7f, accentColor.z * 0.7f, accentColor.w);

    style.borderWidth = 1.0f;
    style.cornerRadius = 3.0f;
    style.padding = ImVec2(8.0f, 4.0f);
    style.margin = ImVec2(2.0f, 2.0f);

    style.fontSize = 14.0f;
    style.animationDuration = 0.2f;
#endif

    return style;
}

// WidgetFrameworkInit implementation
void WidgetFrameworkInit::initialize() {
    if (initialized_)
        return;

    // Register all widget types
    WidgetRegistration::registerAllWidgets();

    // Register all default themes
    DefaultThemes::registerAllThemes();

    initialized_ = true;
}

void WidgetFrameworkInit::shutdown() {
    if (!initialized_)
        return;

    // Clean up theme manager
    auto& themeManager = ThemeManager::getInstance();
    for (const auto& themeName : themeManager.getThemeNames()) {
        themeManager.removeTheme(themeName);
    }

    initialized_ = false;
    WidgetRegistration::registered_ = false;
}

bool WidgetFrameworkInit::isInitialized() {
    return initialized_;
}

}  // namespace gui
}  // namespace bolt
