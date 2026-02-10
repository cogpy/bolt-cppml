#include <cassert>
#include <iostream>
#include <vector>
#include "bolt/editor/keyboard_shortcuts.hpp"

using namespace bolt;

void test_key_combination_parsing() {
    std::cout << "[KeyboardShortcuts] KeyCombinationParsing ... ";
    
    // Test basic key parsing
    auto combo1 = KeyCombination::fromString("Ctrl+S");
    assert(combo1.ctrl == true);
    assert(combo1.shift == false);
    assert(combo1.alt == false);
    assert(combo1.meta == false);
    assert(combo1.key == "S");
    assert(combo1.toString() == "Ctrl+S");
    
    // Test complex combination
    auto combo2 = KeyCombination::fromString("Ctrl+Shift+Alt+F1");
    assert(combo2.ctrl == true);
    assert(combo2.shift == true);
    assert(combo2.alt == true);
    assert(combo2.meta == false);
    assert(combo2.key == "F1");
    assert(combo2.toString() == "Ctrl+Shift+Alt+F1");
    
    // Test single key
    auto combo3 = KeyCombination::fromString("Escape");
    assert(combo3.ctrl == false);
    assert(combo3.shift == false);
    assert(combo3.alt == false);
    assert(combo3.meta == false);
    assert(combo3.key == "Escape");
    assert(combo3.toString() == "Escape");
    
    // Test validity
    assert(combo1.isValid());
    assert(combo2.isValid());
    assert(combo3.isValid());
    
    KeyCombination empty;
    assert(!empty.isValid());
    
    std::cout << "PASS" << std::endl;
}

void test_shortcut_registration() {
    std::cout << "[KeyboardShortcuts] ShortcutRegistration ... ";
    
    KeyboardShortcuts& mgr = KeyboardShortcuts::getInstance();
    
    bool executed = false;
    
    // Register a shortcut
    bool registered = mgr.registerShortcut("Ctrl+T", "test", [&executed]() { executed = true; }, 
                                           ShortcutContext::Editor, "Test command");
    assert(registered); (void)registered;
    
    // Check if shortcut exists
    KeyCombination combo = KeyCombination::fromString("Ctrl+T");
    assert(mgr.hasShortcut(combo, ShortcutContext::Editor));
    
    // Execute shortcut
    bool result = mgr.executeShortcut("Ctrl+T", ShortcutContext::Editor);
    assert(result); (void)result;
    assert(executed); (void)executed;
    
    // Unregister shortcut
    bool unreg = mgr.unregisterShortcut("Ctrl+T", "test", ShortcutContext::Editor);
    assert(unreg); (void)unreg;
    assert(!mgr.hasShortcut(combo, ShortcutContext::Editor));
    
    std::cout << "PASS" << std::endl;
}

void test_context_management() {
    std::cout << "[KeyboardShortcuts] ContextManagement ... ";
    
    auto& mgr = KeyboardShortcuts::getInstance();
    
    // Test initial context
    assert(mgr.getActiveContext() == ShortcutContext::Global);
    
    // Set context
    mgr.setActiveContext(ShortcutContext::Editor);
    assert(mgr.getActiveContext() == ShortcutContext::Editor);
    
    // Test context stack
    mgr.pushContext(ShortcutContext::Search);
    assert(mgr.getActiveContext() == ShortcutContext::Search);
    
    mgr.popContext();
    assert(mgr.getActiveContext() == ShortcutContext::Editor);
    
    // Register shortcuts in different contexts
    bool executed1 = false, executed2 = false;
    mgr.registerShortcut("F1", "help1", [&executed1]() { executed1 = true; }, 
                         ShortcutContext::Editor, "Editor help");
    mgr.registerShortcut("F1", "help2", [&executed2]() { executed2 = true; }, 
                         ShortcutContext::Search, "Search help");
    
    // Execute in editor context
    mgr.setActiveContext(ShortcutContext::Editor);
    assert(mgr.executeShortcut("F1", ShortcutContext::Editor));
    assert(executed1 && !executed2);
    
    // Execute in search context
    executed1 = executed2 = false;
    mgr.setActiveContext(ShortcutContext::Search);
    assert(mgr.executeShortcut("F1", ShortcutContext::Search));
    assert(!executed1 && executed2);
    
    std::cout << "PASS" << std::endl;
}

void test_shortcut_discovery() {
    std::cout << "[KeyboardShortcuts] ShortcutDiscovery ... ";
    
    auto& mgr = KeyboardShortcuts::getInstance();
    
    // Register test shortcuts
    mgr.registerShortcut("Ctrl+K", "command1", []() {}, 
                         ShortcutContext::Editor, "Command 1");
    mgr.registerShortcut("Ctrl+L", "command2", []() {}, 
                         ShortcutContext::Editor, "Command 2");
    mgr.registerShortcut("Ctrl+K", "command3", []() {}, 
                         ShortcutContext::Search, "Command 3");
    
    // Test finding by context
    auto editorShortcuts = mgr.getShortcutsForContext(ShortcutContext::Editor);
    size_t editorCount = 0;
    for (const auto& info : editorShortcuts) {
        if (info.command == "command1" || info.command == "command2") {
            editorCount++;
        }
    }
    assert(editorCount >= 2);
    
    // Test finding by command
    auto command1Shortcuts = mgr.findShortcutsByCommand("command1");
    assert(!command1Shortcuts.empty());
    assert(command1Shortcuts[0].command == "command1");
    
    // Test finding by key
    auto ctrlKShortcuts = mgr.findShortcutsByKey("Ctrl+K");
    size_t ctrlKCount = 0;
    for (const auto& info : ctrlKShortcuts) {
        if (info.command == "command1" || info.command == "command3") {
            ctrlKCount++;
        }
    }
    assert(ctrlKCount >= 2);
    
    std::cout << "PASS" << std::endl;
}

void test_help_system() {
    std::cout << "[KeyboardShortcuts] HelpSystem ... ";
    
    auto& mgr = KeyboardShortcuts::getInstance();
    
    // Register test shortcut with description
    mgr.registerShortcut("Ctrl+H", "testHelp", []() {}, 
                         ShortcutContext::Editor, "Test help command");
    
    // Get help text
    std::string helpText = mgr.getHelpText(ShortcutContext::Editor);
    assert(!helpText.empty());
    assert(helpText.find("Test help command") != std::string::npos);
    
    // Get shortcut description
    std::string description = mgr.getShortcutDescription("Ctrl+H", ShortcutContext::Editor);
    assert(description == "Test help command");
    
    std::cout << "PASS" << std::endl;
}

void test_enable_disable() {
    std::cout << "[KeyboardShortcuts] EnableDisable ... ";
    
    auto& mgr = KeyboardShortcuts::getInstance();
    
    bool executed = false;
    mgr.registerShortcut("Ctrl+E", "testEnable", [&executed]() { executed = true; }, 
                         ShortcutContext::Global, "Test enable");
    
    // Test enabled state
    assert(mgr.isEnabled());
    assert(mgr.executeShortcut("Ctrl+E", ShortcutContext::Global));
    assert(executed);
    
    // Test disabled state
    executed = false;
    mgr.setEnabled(false);
    assert(!mgr.isEnabled());
    assert(!mgr.executeShortcut("Ctrl+E", ShortcutContext::Global));
    assert(!executed);
    
    // Re-enable
    mgr.setEnabled(true);
    assert(mgr.executeShortcut("Ctrl+E", ShortcutContext::Global));
    assert(executed);
    
    std::cout << "PASS" << std::endl;
}

void test_conflict_detection() {
    std::cout << "[KeyboardShortcuts] ConflictDetection ... ";
    
    KeyboardShortcuts& mgr = KeyboardShortcuts::getInstance();
    (void)mgr;
    
    // Register initial shortcut
    assert(mgr.registerShortcut("Ctrl+R", "command1", []() {}, 
                                ShortcutContext::Editor, "Command 1"));
    
    // Try to register conflicting shortcut (same key combination and context, different command)
    assert(!mgr.registerShortcut("Ctrl+R", "command2", []() {}, 
                                 ShortcutContext::Editor, "Command 2"));
    
    // Register same key combination in different context should succeed
    assert(mgr.registerShortcut("Ctrl+R", "command3", []() {}, 
                                ShortcutContext::Search, "Command 3"));
    
    // Register same command with same key should succeed (overwrite)
    assert(mgr.registerShortcut("Ctrl+R", "command1", []() {}, 
                                ShortcutContext::Editor, "Updated Command 1"));
    
    std::cout << "PASS" << std::endl;
}

void test_default_shortcuts() {
    std::cout << "[KeyboardShortcuts] DefaultShortcuts ... ";
    
    auto& mgr = KeyboardShortcuts::getInstance();
    
    // Initialize default shortcuts
    mgr.initDefaultShortcuts();
    
    // Check if some default shortcuts exist
    auto globalShortcuts = mgr.getShortcutsForContext(ShortcutContext::Global);
    auto editorShortcuts = mgr.getShortcutsForContext(ShortcutContext::Editor);
    
    assert(!globalShortcuts.empty());
    assert(!editorShortcuts.empty());
    
    // Check for specific default shortcuts
    auto ctrlSShortcuts = mgr.findShortcutsByKey("Ctrl+S");
    bool hasSave = false;
    for (const auto& info : ctrlSShortcuts) {
        if (info.command == "save") {
            hasSave = true;
            break;
        }
    }
    assert(hasSave); (void)hasSave;
    
    auto ctrlZShortcuts = mgr.findShortcutsByKey("Ctrl+Z");
    bool hasUndo = false;
    for (const auto& info : ctrlZShortcuts) {
        if (info.command == "undo") {
            hasUndo = true;
            break;
        }
    }
    assert(hasUndo); (void)hasUndo;
    
    std::cout << "PASS" << std::endl;
}

int main() {
    std::cout << "Running Keyboard Shortcuts Tests" << std::endl;
    std::cout << "=================================" << std::endl;
    
    try {
        test_key_combination_parsing();
        test_shortcut_registration();
        test_context_management();
        test_shortcut_discovery();
        test_help_system();
        test_enable_disable();
        test_conflict_detection();
        test_default_shortcuts();
        
        std::cout << "\n=== All Keyboard Shortcuts Tests Passed! ===" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
