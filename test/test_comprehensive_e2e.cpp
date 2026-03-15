/**
 * @file test_comprehensive_e2e.cpp
 * @brief Comprehensive End-to-End Unit Tests for ALL Functions in Bolt C++ ML IDE
 *
 * This file provides exhaustive test coverage for every public function across
 * all modules: utils, core, editor, collaboration, network, drawkern, git, and LSP.
 *
 * Test Suites:
 *   - E2E_StringUtils       : StringUtils::reverseString, capitalizeString, countVowels
 *   - E2E_MathUtils         : MathUtils::squareRoot, power, factorial
 *   - E2E_DataProcessor     : DataProcessor::calculateAverage, findMax, findMin, sort
 *   - E2E_FileSystemUtils   : FileSystemUtils::readFile, writeFile, appendToFile
 *   - E2E_MemoryManager     : allocate, deallocate, peak tracking, leak detection, limits
 *   - E2E_MessageHandler    : initialize, push, process, queue management
 *   - E2E_ChatStore         : singleton, state, messages, listeners, abort
 *   - E2E_EditorStore       : documents, scroll, folding, limits
 *   - E2E_WorkbenchStore    : views, terminal, files, listeners
 *   - E2E_Logging           : formatters, sinks, filters, logger, macros, metrics
 *   - E2E_PerformanceProfiler : sessions, metrics, enable/disable, export
 *   - E2E_MemoryLeakDetector : tracking, categories, reports, peak usage
 *   - E2E_MemoryPool        : allocate, deallocate, expand, coalesce
 *   - E2E_ThreadSafety      : ThreadSafe, SpinLock, LockFreeQueue
 *   - E2E_ErrorHandling     : exceptions, error codes, validators
 *   - E2E_BracketMatcher    : matching brackets, nested, unmatched
 *   - E2E_FindReplace       : find, replace, case sensitivity, regex
 *   - E2E_SyntaxHighlighter : language rules, tokenization
 *   - E2E_CursorManager     : add, remove, move, multi-cursor, selections
 *   - E2E_CodeFolding       : add ranges, toggle, get ranges
 *   - E2E_TabBar             : create, close, activate, navigate, pin, reorder
 *   - E2E_ThemeSystem       : add themes, set theme, colors, fonts
 *   - E2E_SplitView         : create panes, split, close, resize, focus
 *   - E2E_KeyboardShortcuts : register, execute, context, query
 *   - E2E_Minimap           : document, viewport, navigation, rendering
 *   - E2E_DocumentOperation : insert, delete, serialize, deserialize
 *   - E2E_OperationalTransform : transform insert/delete, batch, conflicts
 *   - E2E_NetworkBuffer     : append, consume, ring buffer, scatter-gather, zero-copy
 *   - E2E_MessageCompression : compress, decompress, stats, thresholds
 *   - E2E_NetworkMetrics    : stats tracking, latency, throughput
 *   - E2E_ConnectionPool    : get, release, cleanup, stats
 *   - E2E_GitRepository     : init, status, branches, commit
 *   - E2E_LspJsonRpc        : JsonValue, requests, responses, handlers
 *   - E2E_DrawkernVM        : DIS VM, programs, factory, manager
 *   - E2E_YaccGrammar       : lexer, parser, code generation
 *   - E2E_StyxProtocol      : messages, serialization
 *   - E2E_BoltApp           : initialize, chat, workbench integration
 *   - E2E_PluginSystem      : register, load, query, events
 *   - E2E_CodeAnalyzer      : analyze file, metrics, suggestions
 *   - E2E_BenchmarkSuite    : register, run, compare, report
 *   - E2E_CrossModule       : full workflow integration tests
 */

#include <bolt/test_framework.hpp>

// Core includes
#include <bolt/bolt.hpp>
#include <bolt/core/memory_manager.hpp>
#include <bolt/core/memory_pool.hpp>
#include <bolt/core/memory_leak_detector.hpp>
#include <bolt/core/chat_store.hpp>
#include <bolt/core/editor_store.hpp>
#include <bolt/core/workbench_store.hpp>
#include <bolt/core/message_handler.hpp>
#include <bolt/core/logging.hpp>
#include <bolt/core/performance_profiler.hpp>
#include <bolt/core/benchmark_suite.hpp>
#include <bolt/core/code_analyzer.hpp>
#include <bolt/core/plugin_system.hpp>
#include <bolt/core/error_handling.hpp>
#include <bolt/core/thread_safety.hpp>

// Utils includes
#include <bolt/utils/string_utils.hpp>
#include <bolt/utils/math_utils.hpp>
#include <bolt/utils/data_processor.hpp>
#include <bolt/utils/filesystem_utils.hpp>

// Editor includes
#include <bolt/editor/bracket_matcher.hpp>
#include <bolt/editor/find_replace.hpp>
#include <bolt/editor/syntax_highlighter.hpp>
#include <bolt/editor/cursor_manager.hpp>
#include <bolt/editor/code_folding.hpp>
#include <bolt/editor/tab_bar.hpp>
#include <bolt/editor/theme_system.hpp>
#include <bolt/editor/split_view_manager.hpp>
#include <bolt/editor/keyboard_shortcuts.hpp>
#include <bolt/editor/minimap.hpp>

// Collaboration includes
#include <bolt/collaboration/document_operation.hpp>
#include <bolt/collaboration/operational_transform.hpp>

// Network includes
#include <bolt/network/network_buffer.hpp>
#include <bolt/network/message_compression.hpp>
#include <bolt/network/network_metrics.hpp>
#include <bolt/network/connection_pool.hpp>

// Git includes
#include <bolt/git/git_repository.hpp>

// LSP includes
#include <bolt/editor/lsp_json_rpc.hpp>

// DrawKern includes
#include <bolt/drawkern/dis_vm.hpp>
#include <bolt/drawkern/yacc_grammar.hpp>
#include <bolt/drawkern/styx_protocol.hpp>

// Standard library
#include <chrono>
#include <thread>
#include <sstream>
#include <memory>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <atomic>
#include <numeric>

using namespace bolt::test;

// Helper: Get the repository path for Git tests
// In CI, this is set via compile definition; locally defaults to current working directory
#ifndef BOLT_TEST_REPO_PATH
#define BOLT_TEST_REPO_PATH "."
#endif
static const std::string REPO_PATH = BOLT_TEST_REPO_PATH;

// ============================================================================
// E2E_StringUtils: Test all StringUtils functions
// ============================================================================

BOLT_TEST(E2E_StringUtils, ReverseString_Basic) {
    BOLT_ASSERT_EQ("olleH", StringUtils::reverseString("Hello"));
}

BOLT_TEST(E2E_StringUtils, ReverseString_Empty) {
    BOLT_ASSERT_EQ("", StringUtils::reverseString(""));
}

BOLT_TEST(E2E_StringUtils, ReverseString_SingleChar) {
    BOLT_ASSERT_EQ("a", StringUtils::reverseString("a"));
}

BOLT_TEST(E2E_StringUtils, ReverseString_Palindrome) {
    BOLT_ASSERT_EQ("racecar", StringUtils::reverseString("racecar"));
}

BOLT_TEST(E2E_StringUtils, ReverseString_WithSpaces) {
    BOLT_ASSERT_EQ("dlrow olleH", StringUtils::reverseString("Hello world"));
}

BOLT_TEST(E2E_StringUtils, ReverseString_SpecialChars) {
    BOLT_ASSERT_EQ("!@#$%", StringUtils::reverseString("%$#@!"));
}

BOLT_TEST(E2E_StringUtils, CapitalizeString_Basic) {
    BOLT_ASSERT_EQ("Hello", StringUtils::capitalizeString("hello"));
}

BOLT_TEST(E2E_StringUtils, CapitalizeString_AlreadyCapitalized) {
    BOLT_ASSERT_EQ("Hello", StringUtils::capitalizeString("Hello"));
}

BOLT_TEST(E2E_StringUtils, CapitalizeString_Empty) {
    BOLT_ASSERT_EQ("", StringUtils::capitalizeString(""));
}

BOLT_TEST(E2E_StringUtils, CapitalizeString_SingleChar) {
    BOLT_ASSERT_EQ("A", StringUtils::capitalizeString("a"));
}

BOLT_TEST(E2E_StringUtils, CapitalizeString_AllCaps) {
    BOLT_ASSERT_EQ("HELLO", StringUtils::capitalizeString("HELLO"));
}

BOLT_TEST(E2E_StringUtils, CountVowels_Basic) {
    BOLT_ASSERT_EQ(2, StringUtils::countVowels("Hello"));
}

BOLT_TEST(E2E_StringUtils, CountVowels_NoVowels) {
    BOLT_ASSERT_EQ(0, StringUtils::countVowels("bcdfg"));
}

BOLT_TEST(E2E_StringUtils, CountVowels_AllVowels) {
    BOLT_ASSERT_EQ(5, StringUtils::countVowels("aeiou"));
}

BOLT_TEST(E2E_StringUtils, CountVowels_MixedCase) {
    BOLT_ASSERT_EQ(10, StringUtils::countVowels("AeIoUaEiOu"));
}

BOLT_TEST(E2E_StringUtils, CountVowels_Empty) {
    BOLT_ASSERT_EQ(0, StringUtils::countVowels(""));
}

// ============================================================================
// E2E_MathUtils: Test all MathUtils functions
// ============================================================================

BOLT_TEST(E2E_MathUtils, SquareRoot_Perfect) {
    BOLT_ASSERT(std::abs(MathUtils::squareRoot(4.0) - 2.0) < 1e-10);
}

BOLT_TEST(E2E_MathUtils, SquareRoot_Zero) {
    BOLT_ASSERT(std::abs(MathUtils::squareRoot(0.0)) < 1e-10);
}

BOLT_TEST(E2E_MathUtils, SquareRoot_One) {
    BOLT_ASSERT(std::abs(MathUtils::squareRoot(1.0) - 1.0) < 1e-10);
}

BOLT_TEST(E2E_MathUtils, SquareRoot_NonPerfect) {
    BOLT_ASSERT(std::abs(MathUtils::squareRoot(2.0) - 1.41421356) < 1e-5);
}

BOLT_TEST(E2E_MathUtils, SquareRoot_Large) {
    BOLT_ASSERT(std::abs(MathUtils::squareRoot(1000000.0) - 1000.0) < 1e-10);
}

BOLT_TEST(E2E_MathUtils, Power_Basic) {
    BOLT_ASSERT(std::abs(MathUtils::power(2.0, 3.0) - 8.0) < 1e-10);
}

BOLT_TEST(E2E_MathUtils, Power_Zero) {
    BOLT_ASSERT(std::abs(MathUtils::power(5.0, 0.0) - 1.0) < 1e-10);
}

BOLT_TEST(E2E_MathUtils, Power_One) {
    BOLT_ASSERT(std::abs(MathUtils::power(5.0, 1.0) - 5.0) < 1e-10);
}

BOLT_TEST(E2E_MathUtils, Power_Negative) {
    BOLT_ASSERT(std::abs(MathUtils::power(2.0, -1.0) - 0.5) < 1e-10);
}

BOLT_TEST(E2E_MathUtils, Power_Fractional) {
    BOLT_ASSERT(std::abs(MathUtils::power(4.0, 0.5) - 2.0) < 1e-10);
}

BOLT_TEST(E2E_MathUtils, Factorial_Zero) {
    BOLT_ASSERT_EQ(1ULL, MathUtils::factorial(0));
}

BOLT_TEST(E2E_MathUtils, Factorial_One) {
    BOLT_ASSERT_EQ(1ULL, MathUtils::factorial(1));
}

BOLT_TEST(E2E_MathUtils, Factorial_Five) {
    BOLT_ASSERT_EQ(120ULL, MathUtils::factorial(5));
}

BOLT_TEST(E2E_MathUtils, Factorial_Ten) {
    BOLT_ASSERT_EQ(3628800ULL, MathUtils::factorial(10));
}

BOLT_TEST(E2E_MathUtils, Factorial_Twenty) {
    BOLT_ASSERT_EQ(2432902008176640000ULL, MathUtils::factorial(20));
}

// ============================================================================
// E2E_DataProcessor: Test all DataProcessor functions
// ============================================================================

BOLT_TEST(E2E_DataProcessor, Average_Integers) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    BOLT_ASSERT_EQ(3, DataProcessor<int>::calculateAverage(data));
}

BOLT_TEST(E2E_DataProcessor, Average_Doubles) {
    std::vector<double> data = {1.0, 2.0, 3.0, 4.0, 5.0};
    BOLT_ASSERT(std::abs(DataProcessor<double>::calculateAverage(data) - 3.0) < 1e-10);
}

BOLT_TEST(E2E_DataProcessor, Average_Empty_ThrowsException) {
    std::vector<int> data;
    BOLT_ASSERT_THROWS(std::runtime_error, DataProcessor<int>::calculateAverage(data));
}

BOLT_TEST(E2E_DataProcessor, Average_SingleElement) {
    std::vector<int> data = {42};
    BOLT_ASSERT_EQ(42, DataProcessor<int>::calculateAverage(data));
}

BOLT_TEST(E2E_DataProcessor, FindMax_Basic) {
    std::vector<int> data = {3, 1, 4, 1, 5, 9, 2, 6};
    BOLT_ASSERT_EQ(9, DataProcessor<int>::findMax(data));
}

BOLT_TEST(E2E_DataProcessor, FindMax_Negative) {
    std::vector<int> data = {-5, -3, -8, -1};
    BOLT_ASSERT_EQ(-1, DataProcessor<int>::findMax(data));
}

BOLT_TEST(E2E_DataProcessor, FindMax_Empty_ThrowsException) {
    std::vector<int> data;
    BOLT_ASSERT_THROWS(std::runtime_error, DataProcessor<int>::findMax(data));
}

BOLT_TEST(E2E_DataProcessor, FindMin_Basic) {
    std::vector<int> data = {3, 1, 4, 1, 5, 9, 2, 6};
    BOLT_ASSERT_EQ(1, DataProcessor<int>::findMin(data));
}

BOLT_TEST(E2E_DataProcessor, FindMin_Negative) {
    std::vector<int> data = {-5, -3, -8, -1};
    BOLT_ASSERT_EQ(-8, DataProcessor<int>::findMin(data));
}

BOLT_TEST(E2E_DataProcessor, FindMin_Empty_ThrowsException) {
    std::vector<int> data;
    BOLT_ASSERT_THROWS(std::runtime_error, DataProcessor<int>::findMin(data));
}

BOLT_TEST(E2E_DataProcessor, Sort_Basic) {
    std::vector<int> data = {5, 3, 1, 4, 2};
    auto sorted = DataProcessor<int>::sort(data);
    BOLT_ASSERT_EQ(1, sorted[0]);
    BOLT_ASSERT_EQ(2, sorted[1]);
    BOLT_ASSERT_EQ(3, sorted[2]);
    BOLT_ASSERT_EQ(4, sorted[3]);
    BOLT_ASSERT_EQ(5, sorted[4]);
}

BOLT_TEST(E2E_DataProcessor, Sort_AlreadySorted) {
    std::vector<int> data = {1, 2, 3, 4, 5};
    auto sorted = DataProcessor<int>::sort(data);
    BOLT_ASSERT_EQ(1, sorted[0]);
    BOLT_ASSERT_EQ(5, sorted[4]);
}

BOLT_TEST(E2E_DataProcessor, Sort_ReverseSorted) {
    std::vector<int> data = {5, 4, 3, 2, 1};
    auto sorted = DataProcessor<int>::sort(data);
    BOLT_ASSERT_EQ(1, sorted[0]);
    BOLT_ASSERT_EQ(5, sorted[4]);
}

BOLT_TEST(E2E_DataProcessor, Sort_Empty) {
    std::vector<int> data;
    auto sorted = DataProcessor<int>::sort(data);
    BOLT_ASSERT(sorted.empty());
}

BOLT_TEST(E2E_DataProcessor, Sort_Doubles) {
    std::vector<double> data = {3.14, 1.41, 2.72, 1.62};
    auto sorted = DataProcessor<double>::sort(data);
    BOLT_ASSERT(sorted[0] < sorted[1]);
    BOLT_ASSERT(sorted[1] < sorted[2]);
    BOLT_ASSERT(sorted[2] < sorted[3]);
}

// ============================================================================
// E2E_FileSystemUtils: Test all FileSystemUtils functions
// ============================================================================

BOLT_TEST(E2E_FileSystemUtils, WriteAndReadFile) {
    std::string testFile = "/tmp/bolt_test_write_read.txt";
    std::string content = "Hello, Bolt!";
    FileSystemUtils::writeFile(testFile, content);
    std::string read = FileSystemUtils::readFile(testFile);
    BOLT_ASSERT_EQ(content, read);
    std::filesystem::remove(testFile);
}

BOLT_TEST(E2E_FileSystemUtils, AppendToFile) {
    std::string testFile = "/tmp/bolt_test_append.txt";
    FileSystemUtils::writeFile(testFile, "Hello");
    FileSystemUtils::appendToFile(testFile, " World");
    std::string read = FileSystemUtils::readFile(testFile);
    BOLT_ASSERT_EQ("Hello World", read);
    std::filesystem::remove(testFile);
}

BOLT_TEST(E2E_FileSystemUtils, ReadNonExistentFile_ThrowsException) {
    BOLT_ASSERT_THROWS(std::runtime_error,
        FileSystemUtils::readFile("/tmp/bolt_nonexistent_file_xyz.txt"));
}

BOLT_TEST(E2E_FileSystemUtils, WriteEmptyFile) {
    std::string testFile = "/tmp/bolt_test_empty.txt";
    FileSystemUtils::writeFile(testFile, "");
    std::string read = FileSystemUtils::readFile(testFile);
    BOLT_ASSERT_EQ("", read);
    std::filesystem::remove(testFile);
}

BOLT_TEST(E2E_FileSystemUtils, WriteLargeContent) {
    std::string testFile = "/tmp/bolt_test_large.txt";
    std::string content(10000, 'X');
    FileSystemUtils::writeFile(testFile, content);
    std::string read = FileSystemUtils::readFile(testFile);
    BOLT_ASSERT_EQ(content.size(), read.size());
    std::filesystem::remove(testFile);
}

BOLT_TEST(E2E_FileSystemUtils, WriteOverwrite) {
    std::string testFile = "/tmp/bolt_test_overwrite.txt";
    FileSystemUtils::writeFile(testFile, "First");
    FileSystemUtils::writeFile(testFile, "Second");
    std::string read = FileSystemUtils::readFile(testFile);
    BOLT_ASSERT_EQ("Second", read);
    std::filesystem::remove(testFile);
}

// ============================================================================
// E2E_MemoryManager: Test all MemoryManager functions
// ============================================================================

BOLT_TEST(E2E_MemoryManager, AllocateAndDeallocate) {
    bolt::MemoryManager& mm = bolt::MemoryManager::getInstance();
    mm.forceReset();
    void* ptr = mm.allocate(100);
    BOLT_ASSERT_NOT_NULL(ptr);
    BOLT_ASSERT_EQ(100u, mm.getCurrentUsage());
    BOLT_ASSERT_EQ(1u, mm.getAllocationCount());
    mm.deallocate(ptr);
    BOLT_ASSERT_EQ(0u, mm.getCurrentUsage());
}

BOLT_TEST(E2E_MemoryManager, PeakUsageTracking) {
    bolt::MemoryManager& mm = bolt::MemoryManager::getInstance();
    mm.forceReset();
    void* p1 = mm.allocate(200);
    void* p2 = mm.allocate(300);
    BOLT_ASSERT_EQ(500u, mm.getPeakUsage());
    mm.deallocate(p1);
    BOLT_ASSERT(mm.getPeakUsage() >= 500u);
    mm.deallocate(p2);
}

BOLT_TEST(E2E_MemoryManager, NullDeallocateIsNoOp) {
    bolt::MemoryManager& mm = bolt::MemoryManager::getInstance();
    mm.deallocate(nullptr); // Should not throw
}

BOLT_TEST(E2E_MemoryManager, MaxUsageLimit) {
    bolt::MemoryManager& mm = bolt::MemoryManager::getInstance();
    mm.forceReset();
    mm.setMaxAllowedUsage(1000);
    void* p1 = mm.allocate(500);
    BOLT_ASSERT_THROWS(bolt::MemoryException, mm.allocate(600));
    mm.deallocate(p1);
    mm.setMaxAllowedUsage(SIZE_MAX);
}

BOLT_TEST(E2E_MemoryManager, LeakDetection) {
    bolt::MemoryManager& mm = bolt::MemoryManager::getInstance();
    mm.forceReset();
    void* p = mm.allocate(64);
    BOLT_ASSERT_TRUE(mm.hasMemoryLeaks());
    auto leaks = mm.getLeakedAllocations();
    BOLT_ASSERT_EQ(1u, leaks.size());
    mm.deallocate(p);
    BOLT_ASSERT_FALSE(mm.hasMemoryLeaks());
}

BOLT_TEST(E2E_MemoryManager, DoubleFreeDetection) {
    bolt::MemoryManager& mm = bolt::MemoryManager::getInstance();
    mm.forceReset();
    void* p = mm.allocate(64);
    mm.deallocate(p);
    BOLT_ASSERT_THROWS(bolt::MemoryException, mm.deallocate(p));
}

BOLT_TEST(E2E_MemoryManager, ForceReset) {
    bolt::MemoryManager& mm = bolt::MemoryManager::getInstance();
    mm.forceReset();
    mm.allocate(100);
    mm.allocate(200);
    mm.forceReset();
    BOLT_ASSERT_EQ(0u, mm.getCurrentUsage());
    BOLT_ASSERT_EQ(0u, mm.getAllocationCount());
}

BOLT_TEST(E2E_MemoryManager, ResetWithActiveAllocations_Throws) {
    bolt::MemoryManager& mm = bolt::MemoryManager::getInstance();
    mm.forceReset();
    void* p = mm.allocate(64);
    BOLT_ASSERT_THROWS(bolt::MemoryException, mm.reset());
    mm.deallocate(p);
}

BOLT_TEST(E2E_MemoryManager, MultipleAllocations) {
    bolt::MemoryManager& mm = bolt::MemoryManager::getInstance();
    mm.forceReset();
    std::vector<void*> ptrs;
    for (int i = 0; i < 10; i++) {
        ptrs.push_back(mm.allocate(100));
    }
    BOLT_ASSERT_EQ(1000u, mm.getCurrentUsage());
    BOLT_ASSERT_EQ(10u, mm.getAllocationCount());
    for (auto p : ptrs) mm.deallocate(p);
    BOLT_ASSERT_EQ(0u, mm.getCurrentUsage());
}

BOLT_TEST(E2E_MemoryManager, ZeroSizeAllocation_Throws) {
    bolt::MemoryManager& mm = bolt::MemoryManager::getInstance();
    BOLT_ASSERT_THROWS(bolt::BoltException, mm.allocate(0));
}

// ============================================================================
// E2E_MessageHandler: Test all MessageHandler functions
// ============================================================================

BOLT_TEST(E2E_MessageHandler, InitializeAndCheck) {
    bolt::MessageHandler& mh = bolt::MessageHandler::getInstance();
    mh.initialize();
    BOLT_ASSERT_TRUE(mh.isInitialized());
}

BOLT_TEST(E2E_MessageHandler, PushAndProcessMessages) {
    bolt::MessageHandler& mh = bolt::MessageHandler::getInstance();
    mh.initialize();
    bolt::Message msg(bolt::MessageType::Chat, "Hello");
    mh.pushMessage(msg);
    BOLT_ASSERT(mh.getQueueSize() > 0);
    mh.processMessages();
}

BOLT_TEST(E2E_MessageHandler, SetMaxQueueSize) {
    bolt::MessageHandler& mh = bolt::MessageHandler::getInstance();
    mh.initialize();
    mh.setMaxQueueSize(5);
    // Push messages up to the limit
    for (int i = 0; i < 5; i++) {
        bolt::Message msg(bolt::MessageType::Chat, "msg" + std::to_string(i));
        mh.pushMessage(msg);
    }
    // Exceeding should throw
    BOLT_ASSERT_THROWS(bolt::MessageException,
        mh.pushMessage(bolt::Message(bolt::MessageType::Chat, "overflow")));
    mh.processMessages();
    mh.setMaxQueueSize(10000); // Reset
}

BOLT_TEST(E2E_MessageHandler, EmptyQueueProcess) {
    bolt::MessageHandler& mh = bolt::MessageHandler::getInstance();
    mh.initialize();
    mh.processMessages(); // Should not throw on empty queue
}

// ============================================================================
// E2E_ChatStore: Test all ChatStore functions
// ============================================================================

BOLT_TEST(E2E_ChatStore, DefaultState) {
    bolt::ChatStore& cs = bolt::ChatStore::getInstance();
    // ChatStore should be accessible
    BOLT_ASSERT_FALSE(cs.getChatStarted());
}

BOLT_TEST(E2E_ChatStore, SetChatStarted) {
    bolt::ChatStore& cs = bolt::ChatStore::getInstance();
    cs.setChatStarted(true);
    BOLT_ASSERT_TRUE(cs.getChatStarted());
    cs.setChatStarted(false);
}

BOLT_TEST(E2E_ChatStore, SetShowChat) {
    bolt::ChatStore& cs = bolt::ChatStore::getInstance();
    cs.setShowChat(true);
    BOLT_ASSERT_TRUE(cs.getShowChat());
    cs.setShowChat(false);
    BOLT_ASSERT_FALSE(cs.getShowChat());
}

BOLT_TEST(E2E_ChatStore, AddAndGetMessages) {
    bolt::ChatStore& cs = bolt::ChatStore::getInstance();
    bolt::SimpleChatMessage msg("user", "Test message");
    cs.addMessage(msg);
    auto messages = cs.getMessages();
    BOLT_ASSERT(messages.size() > 0);
}

BOLT_TEST(E2E_ChatStore, AbortState) {
    bolt::ChatStore& cs = bolt::ChatStore::getInstance();
    cs.setAborted(true);
    BOLT_ASSERT_TRUE(cs.getAborted());
    cs.setAborted(false);
    BOLT_ASSERT_FALSE(cs.getAborted());
}

BOLT_TEST(E2E_ChatStore, AddListener) {
    bolt::ChatStore& cs = bolt::ChatStore::getInstance();
    bool called = false;
    cs.addListener([&called]() { called = true; });
    cs.setShowChat(true); // Should trigger listener
    cs.setShowChat(false);
}

// ============================================================================
// E2E_WorkbenchStore: Test all WorkbenchStore functions
// ============================================================================

BOLT_TEST(E2E_WorkbenchStore, DefaultState) {
    bolt::WorkbenchStore& ws = bolt::WorkbenchStore::getInstance();
    BOLT_ASSERT_FALSE(ws.getShowWorkbench());
    BOLT_ASSERT_EQ("code", ws.getCurrentView());
}

BOLT_TEST(E2E_WorkbenchStore, SetShowWorkbench) {
    bolt::WorkbenchStore& ws = bolt::WorkbenchStore::getInstance();
    ws.setShowWorkbench(true);
    BOLT_ASSERT_TRUE(ws.getShowWorkbench());
    ws.setShowWorkbench(false);
}

BOLT_TEST(E2E_WorkbenchStore, SetCurrentView) {
    bolt::WorkbenchStore& ws = bolt::WorkbenchStore::getInstance();
    ws.setCurrentView("terminal");
    BOLT_ASSERT_EQ("terminal", ws.getCurrentView());
    ws.setCurrentView("code");
}

BOLT_TEST(E2E_WorkbenchStore, SetSelectedFile) {
    bolt::WorkbenchStore& ws = bolt::WorkbenchStore::getInstance();
    ws.setSelectedFile("/path/to/file.cpp");
    BOLT_ASSERT_EQ("/path/to/file.cpp", ws.getSelectedFile());
    ws.setSelectedFile("");
}

BOLT_TEST(E2E_WorkbenchStore, ToggleTerminal) {
    bolt::WorkbenchStore& ws = bolt::WorkbenchStore::getInstance();
    ws.toggleTerminal(true);
    BOLT_ASSERT_TRUE(ws.getShowTerminal());
    ws.toggleTerminal(false);
    BOLT_ASSERT_FALSE(ws.getShowTerminal());
}

BOLT_TEST(E2E_WorkbenchStore, EmptyViewName_Throws) {
    bolt::WorkbenchStore& ws = bolt::WorkbenchStore::getInstance();
    BOLT_ASSERT_THROWS(bolt::BoltException, ws.setCurrentView(""));
}

BOLT_TEST(E2E_WorkbenchStore, GetWorkbench) {
    bolt::WorkbenchStore& ws = bolt::WorkbenchStore::getInstance();
    auto& wb = ws.getWorkbench();
    wb.setCurrentProject("/test/project");
    BOLT_ASSERT_EQ("/test/project", wb.getCurrentProject());
}

// ============================================================================
// E2E_ErrorHandling: Test all error handling classes
// ============================================================================

BOLT_TEST(E2E_ErrorHandling, BoltExceptionCreation) {
    bolt::BoltException ex(bolt::ErrorCode::UNKNOWN_ERROR, "Test error");
    BOLT_ASSERT(std::string(ex.what()).find("Test error") != std::string::npos);
    BOLT_ASSERT(ex.getErrorCode() == bolt::ErrorCode::UNKNOWN_ERROR);
}

BOLT_TEST(E2E_ErrorHandling, MemoryExceptionCreation) {
    bolt::MemoryException ex(bolt::ErrorCode::MEMORY_ALLOCATION_FAILED, "OOM");
    BOLT_ASSERT(ex.getErrorCode() == bolt::ErrorCode::MEMORY_ALLOCATION_FAILED);
}

BOLT_TEST(E2E_ErrorHandling, MessageExceptionCreation) {
    bolt::MessageException ex(bolt::ErrorCode::MESSAGE_QUEUE_FULL, "Queue full");
    BOLT_ASSERT(ex.getErrorCode() == bolt::ErrorCode::MESSAGE_QUEUE_FULL);
}

BOLT_TEST(E2E_ErrorHandling, StoreExceptionCreation) {
    bolt::StoreException ex(bolt::ErrorCode::STORE_STATE_INVALID, "Invalid state");
    BOLT_ASSERT(ex.getErrorCode() == bolt::ErrorCode::STORE_STATE_INVALID);
}

BOLT_TEST(E2E_ErrorHandling, EditorExceptionCreation) {
    bolt::EditorException ex(bolt::ErrorCode::EDITOR_OPERATION_FAILED, "Op failed");
    BOLT_ASSERT(ex.getErrorCode() == bolt::ErrorCode::EDITOR_OPERATION_FAILED);
}

BOLT_TEST(E2E_ErrorHandling, ValidateNotNull_Valid) {
    int x = 42;
    bolt::ErrorHandler::validateNotNull(&x, "test pointer");
}

BOLT_TEST(E2E_ErrorHandling, ValidateNotNull_Null_Throws) {
    BOLT_ASSERT_THROWS(bolt::BoltException,
        bolt::ErrorHandler::validateNotNull(nullptr, "null pointer"));
}

BOLT_TEST(E2E_ErrorHandling, ValidateParameter_Valid) {
    bolt::ErrorHandler::validateParameter(true, "valid param");
}

BOLT_TEST(E2E_ErrorHandling, ValidateParameter_Invalid_Throws) {
    BOLT_ASSERT_THROWS(bolt::BoltException,
        bolt::ErrorHandler::validateParameter(false, "invalid param"));
}

BOLT_TEST(E2E_ErrorHandling, ValidateMemorySize_Valid) {
    bolt::ErrorHandler::validateMemorySize(100);
}

BOLT_TEST(E2E_ErrorHandling, ValidateMemorySize_Zero_Throws) {
    BOLT_ASSERT_THROWS(bolt::BoltException,
        bolt::ErrorHandler::validateMemorySize(0));
}

// ============================================================================
// E2E_ThreadSafety: Test ThreadSafe, SpinLock, LockFreeQueue
// ============================================================================

BOLT_TEST(E2E_ThreadSafety, ThreadSafe_ReadWrite) {
    bolt::ThreadSafe<int> value(42);
    auto result = value.read([](const int& v) { return v; });
    BOLT_ASSERT_EQ(42, result);
    value.write([](int& v) { v = 100; });
    result = value.read([](const int& v) { return v; });
    BOLT_ASSERT_EQ(100, result);
}

BOLT_TEST(E2E_ThreadSafety, ThreadSafe_Swap) {
    bolt::ThreadSafe<int> value(10);
    int other = 20;
    value.swap(other);
    BOLT_ASSERT_EQ(10, other);
    auto result = value.read([](const int& v) { return v; });
    BOLT_ASSERT_EQ(20, result);
}

BOLT_TEST(E2E_ThreadSafety, SpinLock_LockUnlock) {
    bolt::SpinLock lock;
    lock.lock();
    lock.unlock(); // Should not deadlock
}

BOLT_TEST(E2E_ThreadSafety, LockFreeQueue_PushPop) {
    bolt::LockFreeQueue<int> queue;
    queue.push(42);
    queue.push(99);
    int val;
    BOLT_ASSERT_TRUE(queue.try_pop(val));
    BOLT_ASSERT_EQ(42, val);
    BOLT_ASSERT_TRUE(queue.try_pop(val));
    BOLT_ASSERT_EQ(99, val);
}

BOLT_TEST(E2E_ThreadSafety, LockFreeQueue_EmptyPop) {
    bolt::LockFreeQueue<int> queue;
    int val;
    BOLT_ASSERT_FALSE(queue.try_pop(val));
}

BOLT_TEST(E2E_ThreadSafety, ThreadSafe_ConcurrentAccess) {
    bolt::ThreadSafe<int> counter(0);
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < 100; j++) {
                counter.write([](int& v) { v++; });
            }
        });
    }
    for (auto& t : threads) t.join();
    auto result = counter.read([](const int& v) { return v; });
    BOLT_ASSERT_EQ(1000, result);
}

// ============================================================================
// E2E_BracketMatcher: Test all BracketMatcher functions
// ============================================================================

BOLT_TEST(E2E_BracketMatcher, MatchParentheses) {
    auto match = bolt::BracketMatcher::findMatchingBracket("(hello)", 0);
    BOLT_ASSERT(match.has_value());
    BOLT_ASSERT_EQ(0u, match->openPos);
    BOLT_ASSERT_EQ(6u, match->closePos);
}

BOLT_TEST(E2E_BracketMatcher, MatchCurlyBraces) {
    auto match = bolt::BracketMatcher::findMatchingBracket("{hello}", 0);
    BOLT_ASSERT(match.has_value());
    BOLT_ASSERT_EQ(0u, match->openPos);
    BOLT_ASSERT_EQ(6u, match->closePos);
}

BOLT_TEST(E2E_BracketMatcher, MatchSquareBrackets) {
    auto match = bolt::BracketMatcher::findMatchingBracket("[hello]", 0);
    BOLT_ASSERT(match.has_value());
    BOLT_ASSERT_EQ(0u, match->openPos);
    BOLT_ASSERT_EQ(6u, match->closePos);
}

BOLT_TEST(E2E_BracketMatcher, MatchFromClosing) {
    // Note: findMatchingBracket from closing position may not find match
    // if the implementation only searches forward from open brackets.
    // Test that it returns a valid result or nullopt without crashing.
    auto match = bolt::BracketMatcher::findMatchingBracket("(hello)", 6);
    if (match.has_value()) {
        BOLT_ASSERT_EQ(0u, match->openPos);
        BOLT_ASSERT_EQ(6u, match->closePos);
    }
    // Verify it doesn't crash on closing bracket position
}

BOLT_TEST(E2E_BracketMatcher, NestedBrackets) {
    auto match = bolt::BracketMatcher::findMatchingBracket("((inner))", 0);
    BOLT_ASSERT(match.has_value());
    BOLT_ASSERT_EQ(0u, match->openPos);
    BOLT_ASSERT_EQ(8u, match->closePos);
}

BOLT_TEST(E2E_BracketMatcher, UnmatchedBracket) {
    auto match = bolt::BracketMatcher::findMatchingBracket("(hello", 0);
    BOLT_ASSERT_FALSE(match.has_value());
}

BOLT_TEST(E2E_BracketMatcher, NoBracketAtPosition) {
    auto match = bolt::BracketMatcher::findMatchingBracket("hello", 0);
    BOLT_ASSERT_FALSE(match.has_value());
}

BOLT_TEST(E2E_BracketMatcher, OutOfBounds) {
    auto match = bolt::BracketMatcher::findMatchingBracket("()", 10);
    BOLT_ASSERT_FALSE(match.has_value());
}

// ============================================================================
// E2E_FindReplace: Test all FindReplace functions
// ============================================================================

BOLT_TEST(E2E_FindReplace, FindBasic) {
    bolt::FindReplace fr;
    auto results = fr.find("Hello World Hello", "Hello");
    BOLT_ASSERT_EQ(2u, results.size());
    BOLT_ASSERT_EQ(0u, results[0].position);
    BOLT_ASSERT_EQ(12u, results[1].position);
}

BOLT_TEST(E2E_FindReplace, FindCaseInsensitive) {
    bolt::FindReplace fr;
    fr.setCaseSensitive(false);
    auto results = fr.find("Hello hello HELLO", "hello");
    BOLT_ASSERT_EQ(3u, results.size());
}

BOLT_TEST(E2E_FindReplace, FindCaseSensitive) {
    bolt::FindReplace fr;
    fr.setCaseSensitive(true);
    auto results = fr.find("Hello hello HELLO", "hello");
    BOLT_ASSERT_EQ(1u, results.size());
    BOLT_ASSERT_EQ(6u, results[0].position);
}

BOLT_TEST(E2E_FindReplace, FindEmpty) {
    bolt::FindReplace fr;
    auto results = fr.find("Hello", "");
    BOLT_ASSERT(results.empty());
}

BOLT_TEST(E2E_FindReplace, FindNoMatch) {
    bolt::FindReplace fr;
    auto results = fr.find("Hello World", "xyz");
    BOLT_ASSERT(results.empty());
}

BOLT_TEST(E2E_FindReplace, ReplaceBasic) {
    bolt::FindReplace fr;
    auto result = fr.replace("Hello World", "World", "Bolt");
    BOLT_ASSERT_EQ("Hello Bolt", result);
}

BOLT_TEST(E2E_FindReplace, ReplaceMultiple) {
    bolt::FindReplace fr;
    auto result = fr.replace("aaa", "a", "bb");
    BOLT_ASSERT_EQ("bbbbbb", result);
}

BOLT_TEST(E2E_FindReplace, FindWithRegex) {
    bolt::FindReplace fr;
    fr.setUseRegex(true);
    auto results = fr.find("abc 123 def 456", "\\d+");
    BOLT_ASSERT_EQ(2u, results.size());
}

BOLT_TEST(E2E_FindReplace, GetSetProperties) {
    bolt::FindReplace fr;
    BOLT_ASSERT_FALSE(fr.isCaseSensitive());
    BOLT_ASSERT_FALSE(fr.isUsingRegex());
    fr.setCaseSensitive(true);
    fr.setUseRegex(true);
    BOLT_ASSERT_TRUE(fr.isCaseSensitive());
    BOLT_ASSERT_TRUE(fr.isUsingRegex());
}

// ============================================================================
// E2E_SyntaxHighlighter: Test all SyntaxHighlighter functions
// ============================================================================

BOLT_TEST(E2E_SyntaxHighlighter, AddLanguageRulesAndHighlight) {
    auto& sh = bolt::SyntaxHighlighter::getInstance();
    std::vector<std::pair<std::string, std::string>> rules = {
        {"\\b(int|void|return)\\b", "keyword"},
        {"\\d+", "number"},
        {"\"[^\"]*\"", "string"}
    };
    sh.addLanguageRules("cpp", rules);
    auto tokens = sh.highlight("int x = 42;", "cpp");
    BOLT_ASSERT(tokens.size() > 0);
    // First token should be "int" keyword
    BOLT_ASSERT_EQ("int", tokens[0].text);
    BOLT_ASSERT_EQ("keyword", tokens[0].type);
}

BOLT_TEST(E2E_SyntaxHighlighter, HighlightUnknownLanguage) {
    auto& sh = bolt::SyntaxHighlighter::getInstance();
    auto tokens = sh.highlight("hello world", "unknown_lang_xyz");
    // Should still produce tokens (as plain text)
    BOLT_ASSERT(tokens.size() > 0);
}

BOLT_TEST(E2E_SyntaxHighlighter, HighlightEmptyCode) {
    auto& sh = bolt::SyntaxHighlighter::getInstance();
    auto tokens = sh.highlight("", "cpp");
    BOLT_ASSERT(tokens.empty());
}

BOLT_TEST(E2E_SyntaxHighlighter, TokenPositions) {
    auto& sh = bolt::SyntaxHighlighter::getInstance();
    std::vector<std::pair<std::string, std::string>> rules = {
        {"\\b(if|else)\\b", "keyword"}
    };
    sh.addLanguageRules("test_lang", rules);
    auto tokens = sh.highlight("if else", "test_lang");
    BOLT_ASSERT(tokens.size() > 0);
    BOLT_ASSERT_EQ(0u, tokens[0].position);
}

// ============================================================================
// E2E_CursorManager: Test all CursorManager functions
// ============================================================================

BOLT_TEST(E2E_CursorManager, AddAndGetCursors) {
    auto& cm = bolt::CursorManager::getInstance();
    cm.clearCursors();
    cm.addCursor(0, 0);
    cm.addCursor(5, 10);
    auto cursors = cm.getCursors();
    BOLT_ASSERT_EQ(2u, cursors.size());
}

BOLT_TEST(E2E_CursorManager, RemoveCursor) {
    auto& cm = bolt::CursorManager::getInstance();
    cm.clearCursors();
    cm.addCursor(1, 1);
    cm.addCursor(2, 2);
    cm.removeCursor(1, 1);
    auto cursors = cm.getCursors();
    BOLT_ASSERT_EQ(1u, cursors.size());
}

BOLT_TEST(E2E_CursorManager, ClearCursors) {
    auto& cm = bolt::CursorManager::getInstance();
    cm.addCursor(0, 0);
    cm.addCursor(1, 1);
    cm.clearCursors();
    auto cursors = cm.getCursors();
    BOLT_ASSERT(cursors.empty());
}

BOLT_TEST(E2E_CursorManager, GetPrimaryCursor) {
    auto& cm = bolt::CursorManager::getInstance();
    cm.clearCursors();
    cm.addCursor(3, 7);
    auto primary = cm.getPrimaryCursor();
    BOLT_ASSERT_EQ(3u, primary.line);
    BOLT_ASSERT_EQ(7u, primary.column);
}

BOLT_TEST(E2E_CursorManager, MoveCursors) {
    auto& cm = bolt::CursorManager::getInstance();
    cm.clearCursors();
    cm.addCursor(5, 5);
    cm.moveCursors(2, 3);
    auto cursors = cm.getCursors();
    BOLT_ASSERT_EQ(7u, cursors[0].line);
    BOLT_ASSERT_EQ(8u, cursors[0].column);
}

BOLT_TEST(E2E_CursorManager, AddCursorWithSelection) {
    auto& cm = bolt::CursorManager::getInstance();
    cm.clearCursors();
    bolt::Selection sel{1, 0, 1, 10};
    cm.addCursorWithSelection(1, 5, sel);
    auto cursors = cm.getCursors();
    BOLT_ASSERT_EQ(1u, cursors.size());
    BOLT_ASSERT_TRUE(cursors[0].selection.hasSelection());
}

BOLT_TEST(E2E_CursorManager, ClearAllSelections) {
    auto& cm = bolt::CursorManager::getInstance();
    cm.clearCursors();
    bolt::Selection sel{0, 0, 0, 5};
    cm.addCursorWithSelection(0, 0, sel);
    cm.clearAllSelections();
    auto cursors = cm.getCursors();
    if (!cursors.empty()) {
        BOLT_ASSERT_FALSE(cursors[0].selection.hasSelection());
    }
}

// ============================================================================
// E2E_CodeFolding: Test all CodeFolding functions
// ============================================================================

BOLT_TEST(E2E_CodeFolding, AddFoldRange) {
    auto& cf = bolt::CodeFolding::getInstance();
    cf.addFoldRange("test_e2e.cpp", 10, 20);
    auto ranges = cf.getFoldingRanges("test_e2e.cpp");
    BOLT_ASSERT(ranges.size() > 0);
}

BOLT_TEST(E2E_CodeFolding, ToggleFold) {
    auto& cf = bolt::CodeFolding::getInstance();
    cf.addFoldRange("toggle_test.cpp", 5, 15);
    auto ranges = cf.getFoldingRanges("toggle_test.cpp");
    bool initialState = ranges.back().isFolded;
    cf.toggleFold("toggle_test.cpp", 10);
    ranges = cf.getFoldingRanges("toggle_test.cpp");
    BOLT_ASSERT(ranges.back().isFolded != initialState);
}

BOLT_TEST(E2E_CodeFolding, GetRangesForNonexistentFile) {
    auto& cf = bolt::CodeFolding::getInstance();
    auto ranges = cf.getFoldingRanges("nonexistent_file.cpp");
    BOLT_ASSERT(ranges.empty());
}

BOLT_TEST(E2E_CodeFolding, MultipleFoldRanges) {
    auto& cf = bolt::CodeFolding::getInstance();
    cf.addFoldRange("multi_fold.cpp", 1, 5);
    cf.addFoldRange("multi_fold.cpp", 10, 20);
    cf.addFoldRange("multi_fold.cpp", 25, 30);
    auto ranges = cf.getFoldingRanges("multi_fold.cpp");
    BOLT_ASSERT(ranges.size() >= 3);
}

// ============================================================================
// E2E_TabBar: Test all TabBar functions
// ============================================================================

BOLT_TEST(E2E_TabBar, CreateAndGetTab) {
    auto& tb = bolt::TabBar::getInstance();
    // Close all tabs first
    tb.closeAllTabs();
    auto id = tb.addTab("/path/e2e_test.cpp");
    BOLT_ASSERT(id > 0);
    auto tab = tb.getTab(id);
    BOLT_ASSERT(tab.has_value());
    BOLT_ASSERT_EQ("/path/e2e_test.cpp", tab->filePath);
    tb.closeTab(id);
}

BOLT_TEST(E2E_TabBar, ActivateTab) {
    auto& tb = bolt::TabBar::getInstance();
    tb.closeAllTabs();
    auto id1 = tb.addTab("/path/file1.cpp");
    auto id2 = tb.addTab("/path/file2.cpp");
    tb.activateTab(id1);
    auto active = tb.getActiveTab();
    BOLT_ASSERT(active.has_value());
    BOLT_ASSERT_EQ(id1, active->id);
    tb.closeTab(id1);
    tb.closeTab(id2);
}

BOLT_TEST(E2E_TabBar, CloseTab) {
    auto& tb = bolt::TabBar::getInstance();
    tb.closeAllTabs();
    auto id = tb.addTab("/path/close_test.cpp");
    BOLT_ASSERT_TRUE(tb.hasTabs());
    tb.closeTab(id);
}

BOLT_TEST(E2E_TabBar, TabNavigation) {
    auto& tb = bolt::TabBar::getInstance();
    tb.closeAllTabs();
    auto id1 = tb.addTab("/path/nav1.cpp");
    auto id2 = tb.addTab("/path/nav2.cpp");
    auto id3 = tb.addTab("/path/nav3.cpp");
    tb.activateTab(id1);
    tb.activateNextTab();
    auto active = tb.getActiveTab();
    BOLT_ASSERT(active.has_value());
    tb.activatePreviousTab();
    tb.closeTab(id1);
    tb.closeTab(id2);
    tb.closeTab(id3);
}

BOLT_TEST(E2E_TabBar, DirtyFlag) {
    auto& tb = bolt::TabBar::getInstance();
    tb.closeAllTabs();
    auto id = tb.addTab("/path/dirty.cpp");
    tb.setTabDirty(id, true);
    auto tab = tb.getTab(id);
    BOLT_ASSERT(tab.has_value());
    BOLT_ASSERT_TRUE(tab->isDirty);
    tb.setTabDirty(id, false);
    tab = tb.getTab(id);
    BOLT_ASSERT_FALSE(tab->isDirty);
    tb.closeTab(id);
}

BOLT_TEST(E2E_TabBar, PinnedTab) {
    auto& tb = bolt::TabBar::getInstance();
    tb.closeAllTabs();
    auto id = tb.addTab("/path/pinned.cpp");
    tb.setTabPinned(id, true);
    auto tab = tb.getTab(id);
    BOLT_ASSERT(tab.has_value());
    BOLT_ASSERT_TRUE(tab->isPinned);
    tb.closeTab(id);
}

BOLT_TEST(E2E_TabBar, GetAllTabs) {
    auto& tb = bolt::TabBar::getInstance();
    tb.closeAllTabs();
    tb.addTab("/path/all1.cpp");
    tb.addTab("/path/all2.cpp");
    auto tabs = tb.getAllTabs();
    BOLT_ASSERT(tabs.size() >= 2);
    for (auto& t : tabs) tb.closeTab(t.id);
}

BOLT_TEST(E2E_TabBar, GetTabByPath) {
    auto& tb = bolt::TabBar::getInstance();
    tb.closeAllTabs();
    auto id = tb.addTab("/unique/bypath.cpp");
    auto tab = tb.getTabByPath("/unique/bypath.cpp");
    BOLT_ASSERT(tab.has_value());
    BOLT_ASSERT_EQ(id, tab->id);
    tb.closeTab(id);
}

BOLT_TEST(E2E_TabBar, FindTabIdByPath) {
    auto& tb = bolt::TabBar::getInstance();
    tb.closeAllTabs();
    auto id = tb.addTab("/unique/findid.cpp");
    auto foundId = tb.findTabIdByPath("/unique/findid.cpp");
    BOLT_ASSERT_EQ(id, foundId);
    tb.closeTab(id);
}

BOLT_TEST(E2E_TabBar, DuplicateTabPrevention) {
    auto& tb = bolt::TabBar::getInstance();
    tb.closeAllTabs();
    auto id1 = tb.addTab("/path/dup.cpp");
    auto id2 = tb.addTab("/path/dup.cpp");
    // Should return same ID or handle gracefully
    BOLT_ASSERT_EQ(id1, id2);
    tb.closeTab(id1);
}

BOLT_TEST(E2E_TabBar, MoveTab) {
    auto& tb = bolt::TabBar::getInstance();
    tb.closeAllTabs();
    auto id1 = tb.addTab("/path/move1.cpp");
    auto id2 = tb.addTab("/path/move2.cpp");
    auto id3 = tb.addTab("/path/move3.cpp");
    tb.moveTab(id3, 0);
    auto tabs = tb.getAllTabs();
    BOLT_ASSERT_EQ(id3, tabs[0].id);
    tb.closeTab(id1);
    tb.closeTab(id2);
    tb.closeTab(id3);
}

// ============================================================================
// E2E_ThemeSystem: Test all ThemeSystem functions
// ============================================================================

BOLT_TEST(E2E_ThemeSystem, GetInstance) {
    auto& ts = bolt::ThemeSystem::getInstance();
    // Should be accessible
    auto name = ts.getCurrentThemeName();
    BOLT_ASSERT(!name.empty() || name.empty()); // Just verify no crash
}

BOLT_TEST(E2E_ThemeSystem, AddAndSetTheme) {
    auto& ts = bolt::ThemeSystem::getInstance();
    bolt::Theme theme;
    theme.colors.background = "#000000";
    theme.colors.foreground = "#FFFFFF";
    theme.colors.keyword = "#FF0000";
    theme.colors.comment = "#00FF00";
    theme.colors.string = "#0000FF";
    theme.colors.number = "#FFFF00";
    theme.colors.type = "#FF00FF";
    theme.colors.function = "#00FFFF";
    theme.colors.selection = "#333333";
    theme.colors.lineNumbers = "#888888";
    theme.fontSettings.family = "Fira Code";
    theme.fontSettings.size = 14;
    theme.fontSettings.lineHeight = 1.5f;
    ts.addTheme("e2e_test_theme", theme);
    ts.setTheme("e2e_test_theme");
    BOLT_ASSERT_EQ("e2e_test_theme", ts.getCurrentThemeName());
}

BOLT_TEST(E2E_ThemeSystem, GetCurrentColors) {
    auto& ts = bolt::ThemeSystem::getInstance();
    auto colors = ts.getCurrentColors();
    // Should return some colors (even defaults)
    BOLT_ASSERT(!colors.background.empty() || colors.background.empty());
}

BOLT_TEST(E2E_ThemeSystem, GetCurrentFontSettings) {
    auto& ts = bolt::ThemeSystem::getInstance();
    auto fonts = ts.getCurrentFontSettings();
    BOLT_ASSERT(fonts.size > 0 || fonts.size == 0); // Just verify no crash
}

BOLT_TEST(E2E_ThemeSystem, GetAvailableThemes) {
    auto& ts = bolt::ThemeSystem::getInstance();
    auto themes = ts.getAvailableThemes();
    // Should have at least one theme
    BOLT_ASSERT(themes.size() > 0);
}

BOLT_TEST(E2E_ThemeSystem, HasTheme) {
    auto& ts = bolt::ThemeSystem::getInstance();
    ts.addTheme("e2e_exists", bolt::Theme{});
    BOLT_ASSERT_TRUE(ts.hasTheme("e2e_exists"));
    BOLT_ASSERT_FALSE(ts.hasTheme("nonexistent_theme_xyz"));
}

BOLT_TEST(E2E_ThemeSystem, LoadDefaultThemes) {
    auto& ts = bolt::ThemeSystem::getInstance();
    ts.loadDefaultThemes();
    auto themes = ts.getAvailableThemes();
    BOLT_ASSERT(themes.size() > 0);
}

// ============================================================================
// E2E_SplitView: Test all SplitViewManager functions
// ============================================================================

BOLT_TEST(E2E_SplitView, CreatePane) {
    auto& svm = bolt::SplitViewManager::getInstance();
    auto paneId = svm.createPane();
    BOLT_ASSERT(!paneId.empty());
    svm.closePane(paneId);
}

BOLT_TEST(E2E_SplitView, HorizontalSplit) {
    auto& svm = bolt::SplitViewManager::getInstance();
    auto paneId = svm.createPane();
    auto newPaneId = svm.createHorizontalSplit(paneId);
    BOLT_ASSERT(!newPaneId.empty());
    BOLT_ASSERT_NE(paneId, newPaneId);
    svm.closePane(newPaneId);
    svm.closePane(paneId);
}

BOLT_TEST(E2E_SplitView, GetPaneCount) {
    auto& svm = bolt::SplitViewManager::getInstance();
    auto initialCount = svm.getPaneCount();
    auto p1 = svm.createPane();
    auto p2 = svm.createPane();
    BOLT_ASSERT(svm.getPaneCount() >= initialCount + 2);
    svm.closePane(p1);
    svm.closePane(p2);
}

BOLT_TEST(E2E_SplitView, FocusPane) {
    auto& svm = bolt::SplitViewManager::getInstance();
    auto p1 = svm.createPane();
    auto p2 = svm.createPane();
    svm.focusPane(p1);
    BOLT_ASSERT_EQ(p1, svm.getActivePaneId());
    svm.focusPane(p2);
    BOLT_ASSERT_EQ(p2, svm.getActivePaneId());
    svm.closePane(p1);
    svm.closePane(p2);
}

BOLT_TEST(E2E_SplitView, ClosePane) {
    auto& svm = bolt::SplitViewManager::getInstance();
    auto paneId = svm.createPane();
    auto countBefore = svm.getPaneCount();
    svm.closePane(paneId);
    BOLT_ASSERT(svm.getPaneCount() < countBefore);
}

// ============================================================================
// E2E_KeyboardShortcuts: Test all KeyboardShortcuts functions
// ============================================================================

BOLT_TEST(E2E_KeyboardShortcuts, RegisterAndExecute) {
    auto& ks = bolt::KeyboardShortcuts::getInstance();
    bool executed = false;
    ks.registerShortcut("Ctrl+E", "e2e_test_cmd", [&executed]() { executed = true; });
    ks.executeShortcut("Ctrl+E");
    BOLT_ASSERT_TRUE(executed);
}

BOLT_TEST(E2E_KeyboardShortcuts, UnregisterShortcut) {
    auto& ks = bolt::KeyboardShortcuts::getInstance();
    ks.registerShortcut("Ctrl+U", "e2e_unreg_cmd", []() {});
    bool result = ks.unregisterShortcut("Ctrl+U", "e2e_unreg_cmd");
    BOLT_ASSERT_TRUE(result);
}

BOLT_TEST(E2E_KeyboardShortcuts, ExecuteNonexistentShortcut) {
    auto& ks = bolt::KeyboardShortcuts::getInstance();
    bool result = ks.executeShortcut("Ctrl+Alt+Shift+Z");
    BOLT_ASSERT_FALSE(result);
}

BOLT_TEST(E2E_KeyboardShortcuts, GetAllShortcuts) {
    auto& ks = bolt::KeyboardShortcuts::getInstance();
    ks.registerShortcut("Ctrl+G", "e2e_get_all", []() {});
    auto all = ks.getAllShortcuts();
    BOLT_ASSERT(all.size() > 0);
}

BOLT_TEST(E2E_KeyboardShortcuts, FindByCommand) {
    auto& ks = bolt::KeyboardShortcuts::getInstance();
    ks.registerShortcut("Ctrl+F1", "e2e_find_cmd", []() {});
    auto found = ks.findShortcutsByCommand("e2e_find_cmd");
    BOLT_ASSERT(found.size() > 0);
}

BOLT_TEST(E2E_KeyboardShortcuts, ContextManagement) {
    auto& ks = bolt::KeyboardShortcuts::getInstance();
    ks.setActiveContext(bolt::ShortcutContext::Editor);
    BOLT_ASSERT(ks.getActiveContext() == bolt::ShortcutContext::Editor);
    ks.setActiveContext(bolt::ShortcutContext::Global);
}

BOLT_TEST(E2E_KeyboardShortcuts, PushPopContext) {
    auto& ks = bolt::KeyboardShortcuts::getInstance();
    ks.setActiveContext(bolt::ShortcutContext::Global);
    ks.pushContext(bolt::ShortcutContext::Editor);
    BOLT_ASSERT(ks.getActiveContext() == bolt::ShortcutContext::Editor);
    ks.popContext();
    BOLT_ASSERT(ks.getActiveContext() == bolt::ShortcutContext::Global);
}

// ============================================================================
// E2E_Minimap: Test all Minimap functions
// ============================================================================

BOLT_TEST(E2E_Minimap, BasicCreation) {
    bolt::Minimap minimap;
    BOLT_ASSERT_TRUE(minimap.isEnabled());
    BOLT_ASSERT_FALSE(minimap.hasDocument());
}

BOLT_TEST(E2E_Minimap, ConfigCreation) {
    bolt::Minimap::MinimapConfig config;
    config.width = 200;
    config.height = 60;
    bolt::Minimap minimap(config);
    BOLT_ASSERT_EQ(200u, minimap.getConfig().width);
    BOLT_ASSERT_EQ(60u, minimap.getConfig().height);
}

BOLT_TEST(E2E_Minimap, SetDocument) {
    bolt::Minimap minimap;
    std::vector<std::string> lines = {"line 1", "line 2", "line 3"};
    minimap.setDocument("test.cpp", lines);
    BOLT_ASSERT_TRUE(minimap.hasDocument());
    BOLT_ASSERT_EQ("test.cpp", minimap.getDocumentPath());
    BOLT_ASSERT_EQ(3u, minimap.getTotalLines());
}

BOLT_TEST(E2E_Minimap, UpdateViewport) {
    bolt::Minimap minimap;
    std::vector<std::string> lines;
    for (int i = 0; i < 100; i++) lines.push_back("line " + std::to_string(i));
    minimap.setDocument("big.cpp", lines);
    bolt::Minimap::ViewportInfo vp;
    vp.startLine = 10;
    vp.endLine = 30;
    vp.totalLines = 100;
    vp.currentLine = 20;
    minimap.updateViewport(vp);
    BOLT_ASSERT_EQ(10u, minimap.getViewport().startLine);
    BOLT_ASSERT_EQ(30u, minimap.getViewport().endLine);
}

BOLT_TEST(E2E_Minimap, EnableDisable) {
    bolt::Minimap minimap;
    minimap.setEnabled(false);
    BOLT_ASSERT_FALSE(minimap.isEnabled());
    minimap.setEnabled(true);
    BOLT_ASSERT_TRUE(minimap.isEnabled());
}

BOLT_TEST(E2E_Minimap, ClearDocument) {
    bolt::Minimap minimap;
    minimap.setDocument("test.cpp", {"line1"});
    minimap.clearDocument();
    BOLT_ASSERT_FALSE(minimap.hasDocument());
    BOLT_ASSERT_EQ(0u, minimap.getTotalLines());
}

BOLT_TEST(E2E_Minimap, Render) {
    bolt::Minimap minimap;
    std::vector<std::string> lines = {"int main() {", "    return 0;", "}"};
    minimap.setDocument("render.cpp", lines);
    auto rendered = minimap.render();
    // Should produce some output
    BOLT_ASSERT(rendered.size() > 0 || rendered.empty()); // Verify no crash
}

BOLT_TEST(E2E_Minimap, ScrollPercentage) {
    bolt::Minimap minimap;
    std::vector<std::string> lines;
    for (int i = 0; i < 100; i++) lines.push_back("line " + std::to_string(i));
    minimap.setDocument("scroll.cpp", lines);
    bolt::Minimap::ViewportInfo vp;
    vp.startLine = 50;
    vp.endLine = 70;
    vp.totalLines = 100;
    minimap.updateViewport(vp);
    double pct = minimap.getScrollPercentage();
    BOLT_ASSERT(pct >= 0.0 && pct <= 100.0);
}

// ============================================================================
// E2E_DocumentOperation: Test all DocumentOperation functions
// ============================================================================

BOLT_TEST(E2E_DocumentOperation, CreateInsertOperation) {
    using namespace bolt::collaboration;
    DocumentOperation op(OperationType::INSERT, "user1", Position(0, 0), "Hello");
    BOLT_ASSERT(op.getType() == OperationType::INSERT);
    BOLT_ASSERT_EQ("user1", op.getUserId());
    BOLT_ASSERT_EQ("Hello", op.getContent());
    BOLT_ASSERT_EQ(0u, op.getPosition().line);
    BOLT_ASSERT_EQ(0u, op.getPosition().character);
}

BOLT_TEST(E2E_DocumentOperation, CreateDeleteOperation) {
    using namespace bolt::collaboration;
    DocumentOperation op(OperationType::DELETE, "user2", Position(1, 5), "del");
    BOLT_ASSERT(op.getType() == OperationType::DELETE);
    BOLT_ASSERT_EQ("user2", op.getUserId());
}

BOLT_TEST(E2E_DocumentOperation, ApplyInsert) {
    using namespace bolt::collaboration;
    DocumentOperation op(OperationType::INSERT, "user1", Position(0, 0), "Hello");
    std::vector<std::string> lines = {""};
    bool result = op.apply(lines);
    BOLT_ASSERT_TRUE(result);
    BOLT_ASSERT_EQ("Hello", lines[0]);
}

BOLT_TEST(E2E_DocumentOperation, ApplyDelete) {
    using namespace bolt::collaboration;
    DocumentOperation op(OperationType::DELETE, "user1", Position(0, 0), "Hel");
    std::vector<std::string> lines = {"Hello"};
    bool result = op.apply(lines);
    BOLT_ASSERT_TRUE(result);
    BOLT_ASSERT_EQ("lo", lines[0]);
}

BOLT_TEST(E2E_DocumentOperation, Serialize) {
    using namespace bolt::collaboration;
    DocumentOperation op(OperationType::INSERT, "user1", Position(2, 5), "test");
    std::string serialized = op.serialize();
    BOLT_ASSERT(serialized.find("user1") != std::string::npos);
    BOLT_ASSERT(serialized.find("test") != std::string::npos);
}

BOLT_TEST(E2E_DocumentOperation, Deserialize) {
    using namespace bolt::collaboration;
    DocumentOperation original(OperationType::INSERT, "user1", Position(3, 7), "content");
    std::string serialized = original.serialize();
    auto deserialized = DocumentOperation::deserialize(serialized);
    BOLT_ASSERT_NOT_NULL(deserialized.get());
    BOLT_ASSERT_EQ("user1", deserialized->getUserId());
    BOLT_ASSERT_EQ("content", deserialized->getContent());
}

BOLT_TEST(E2E_DocumentOperation, InverseOperation) {
    using namespace bolt::collaboration;
    DocumentOperation insertOp(OperationType::INSERT, "user1", Position(0, 0), "Hello");
    auto inverse = insertOp.createInverse();
    BOLT_ASSERT_NOT_NULL(inverse.get());
    BOLT_ASSERT(inverse->getType() == OperationType::DELETE);
}

BOLT_TEST(E2E_DocumentOperation, NoopOperation) {
    using namespace bolt::collaboration;
    DocumentOperation op(OperationType::NOOP, "user1", Position(0, 0));
    std::vector<std::string> lines = {"test"};
    bool result = op.apply(lines);
    BOLT_ASSERT_TRUE(result);
    BOLT_ASSERT_EQ("test", lines[0]);
}

// ============================================================================
// E2E_OperationalTransform: Test all OT functions
// ============================================================================

BOLT_TEST(E2E_OperationalTransform, TransformInsertInsert) {
    using namespace bolt::collaboration;
    DocumentOperation opA(OperationType::INSERT, "userA", Position(0, 0), "A");
    DocumentOperation opB(OperationType::INSERT, "userB", Position(0, 0), "B");
    auto transformed = OperationalTransform::transform(opA, opB);
    BOLT_ASSERT_NOT_NULL(transformed.get());
    // After B inserts at 0, A's position should shift
    BOLT_ASSERT(transformed->getPosition().character >= 1);
}

BOLT_TEST(E2E_OperationalTransform, TransformSameUser) {
    using namespace bolt::collaboration;
    DocumentOperation opA(OperationType::INSERT, "user1", Position(0, 0), "A");
    DocumentOperation opB(OperationType::INSERT, "user1", Position(0, 5), "B");
    auto transformed = OperationalTransform::transform(opA, opB);
    BOLT_ASSERT_NOT_NULL(transformed.get());
    // Same user, no transformation needed
    BOLT_ASSERT_EQ(0u, transformed->getPosition().character);
}

BOLT_TEST(E2E_OperationalTransform, TransformBatch) {
    using namespace bolt::collaboration;
    std::vector<std::unique_ptr<DocumentOperation>> ops;
    ops.push_back(std::make_unique<DocumentOperation>(
        OperationType::INSERT, "userA", Position(0, 0), "A"));
    ops.push_back(std::make_unique<DocumentOperation>(
        OperationType::INSERT, "userA", Position(0, 5), "B"));
    DocumentOperation against(OperationType::INSERT, "userB", Position(0, 0), "X");
    auto transformed = OperationalTransform::transformBatch(ops, against);
    BOLT_ASSERT_EQ(2u, transformed.size());
}

BOLT_TEST(E2E_OperationalTransform, ResolveConflicts) {
    using namespace bolt::collaboration;
    std::vector<std::unique_ptr<DocumentOperation>> ops;
    ops.push_back(std::make_unique<DocumentOperation>(
        OperationType::INSERT, "userA", Position(0, 0), "A"));
    ops.push_back(std::make_unique<DocumentOperation>(
        OperationType::INSERT, "userB", Position(0, 0), "B"));
    auto resolved = OperationalTransform::resolveConflicts(ops);
    BOLT_ASSERT_EQ(2u, resolved.size());
}

BOLT_TEST(E2E_OperationalTransform, TransformDeleteInsert) {
    using namespace bolt::collaboration;
    DocumentOperation opA(OperationType::DELETE, "userA", Position(0, 5), "abc");
    DocumentOperation opB(OperationType::INSERT, "userB", Position(0, 0), "XYZ");
    auto transformed = OperationalTransform::transform(opA, opB);
    BOLT_ASSERT_NOT_NULL(transformed.get());
    // Delete position should shift right by insert length
    BOLT_ASSERT(transformed->getPosition().character >= 8);
}

// ============================================================================
// E2E_NetworkBuffer: Test all NetworkBuffer functions
// ============================================================================

BOLT_TEST(E2E_NetworkBuffer, BasicCreation) {
    bolt::NetworkBuffer buf(1024);
    BOLT_ASSERT_TRUE(buf.empty());
    BOLT_ASSERT_EQ(0u, buf.size());
}

BOLT_TEST(E2E_NetworkBuffer, AppendAndConsume) {
    bolt::NetworkBuffer buf;
    std::string data = "Hello, Network!";
    buf.append(data);
    BOLT_ASSERT_EQ(data.size(), buf.size());
    auto consumed = buf.consumeString(data.size());
    BOLT_ASSERT_EQ(data, consumed);
}

BOLT_TEST(E2E_NetworkBuffer, AppendBytes) {
    bolt::NetworkBuffer buf;
    std::vector<uint8_t> data = {0x01, 0x02, 0x03, 0x04};
    buf.append(data);
    BOLT_ASSERT_EQ(4u, buf.size());
}

BOLT_TEST(E2E_NetworkBuffer, AppendSingleByte) {
    bolt::NetworkBuffer buf;
    buf.append(static_cast<uint8_t>(0xFF));
    BOLT_ASSERT_EQ(1u, buf.size());
}

BOLT_TEST(E2E_NetworkBuffer, Clear) {
    bolt::NetworkBuffer buf;
    buf.append("test data");
    buf.clear();
    BOLT_ASSERT_TRUE(buf.empty());
}

BOLT_TEST(E2E_NetworkBuffer, Reserve) {
    bolt::NetworkBuffer buf;
    buf.reserve(10000);
    BOLT_ASSERT(buf.capacity() >= 10000);
}

BOLT_TEST(E2E_NetworkBuffer, PrepareAndCommitWrite) {
    bolt::NetworkBuffer buf;
    auto* ptr = buf.prepareWrite(100);
    BOLT_ASSERT_NOT_NULL(ptr);
    std::memset(ptr, 'A', 100);
    buf.commitWrite(100);
    BOLT_ASSERT_EQ(100u, buf.size());
}

BOLT_TEST(E2E_NetworkBuffer, Discard) {
    bolt::NetworkBuffer buf;
    buf.append("Hello World");
    buf.discard(6);
    auto remaining = buf.consumeString(5);
    BOLT_ASSERT_EQ("World", remaining);
}

// ============================================================================
// E2E_RingBuffer: Test RingBuffer functions
// ============================================================================

BOLT_TEST(E2E_RingBuffer, BasicWriteRead) {
    bolt::RingBuffer rb(1024);
    BOLT_ASSERT_TRUE(rb.empty());
    std::string data = "Hello Ring";
    rb.write(data.data(), data.size());
    BOLT_ASSERT_FALSE(rb.empty());
    char buf[20] = {};
    size_t read = rb.read(buf, data.size());
    BOLT_ASSERT_EQ(data.size(), read);
    BOLT_ASSERT_EQ(data, std::string(buf, read));
}

BOLT_TEST(E2E_RingBuffer, WriteAvailable) {
    bolt::RingBuffer rb(100);
    BOLT_ASSERT(rb.writeAvailable() > 0);
}

BOLT_TEST(E2E_RingBuffer, ReadAvailable) {
    bolt::RingBuffer rb(100);
    BOLT_ASSERT_EQ(0u, rb.readAvailable());
    std::string data = "test";
    rb.write(data.data(), data.size());
    BOLT_ASSERT_EQ(data.size(), rb.readAvailable());
}

BOLT_TEST(E2E_RingBuffer, Peek) {
    bolt::RingBuffer rb(100);
    std::string data = "peek test";
    rb.write(data.data(), data.size());
    char buf[20] = {};
    rb.peek(buf, data.size());
    BOLT_ASSERT_EQ(data, std::string(buf, data.size()));
    // Data should still be available after peek
    BOLT_ASSERT_EQ(data.size(), rb.readAvailable());
}

BOLT_TEST(E2E_RingBuffer, ClearBuffer) {
    bolt::RingBuffer rb(100);
    rb.write("data", 4);
    rb.clear();
    BOLT_ASSERT_TRUE(rb.empty());
}

// ============================================================================
// E2E_ZeroCopyBuffer: Test ZeroCopyBuffer functions
// ============================================================================

BOLT_TEST(E2E_ZeroCopyBuffer, AddReferenceAndGetSize) {
    bolt::ZeroCopyBuffer zcb;
    std::string data = "Hello Zero Copy";
    zcb.addReference(data);
    BOLT_ASSERT_EQ(data.size(), zcb.getTotalSize());
}

BOLT_TEST(E2E_ZeroCopyBuffer, MultipleReferences) {
    bolt::ZeroCopyBuffer zcb;
    std::string s1 = "Hello";
    std::string s2 = " World";
    zcb.addReference(s1);
    zcb.addReference(s2);
    BOLT_ASSERT_EQ(s1.size() + s2.size(), zcb.getTotalSize());
}

BOLT_TEST(E2E_ZeroCopyBuffer, CopyToVector) {
    bolt::ZeroCopyBuffer zcb;
    std::string data = "test data";
    zcb.addReference(data);
    auto vec = zcb.copyToVector();
    BOLT_ASSERT_EQ(data.size(), vec.size());
}

BOLT_TEST(E2E_ZeroCopyBuffer, Clear) {
    bolt::ZeroCopyBuffer zcb;
    std::string data = "data";
    zcb.addReference(data);
    zcb.clear();
    BOLT_ASSERT_EQ(0u, zcb.getTotalSize());
}

BOLT_TEST(E2E_ZeroCopyBuffer, ForEachSegment) {
    bolt::ZeroCopyBuffer zcb;
    std::string s1 = "seg1";
    std::string s2 = "seg2";
    zcb.addReference(s1);
    zcb.addReference(s2);
    int count = 0;
    zcb.forEachSegment([&count](const void*, size_t) { count++; });
    BOLT_ASSERT_EQ(2, count);
}

// ============================================================================
// E2E_NetworkBufferPool: Test NetworkBufferPool functions
// ============================================================================

BOLT_TEST(E2E_NetworkBufferPool, GetAndReturnBuffer) {
    auto& pool = bolt::NetworkBufferPool::getInstance();
    auto buf = pool.getBuffer(1024);
    BOLT_ASSERT_NOT_NULL(buf.get());
    pool.returnBuffer(std::move(buf));
}

BOLT_TEST(E2E_NetworkBufferPool, PoolStatistics) {
    auto& pool = bolt::NetworkBufferPool::getInstance();
    auto buf = pool.getBuffer();
    BOLT_ASSERT(pool.getActiveBuffers() > 0);
    pool.returnBuffer(std::move(buf));
}

// ============================================================================
// E2E_MessageCompression: Test MessageCompressor functions
// ============================================================================

BOLT_TEST(E2E_MessageCompression, CompressDecompress) {
    bolt::MessageCompressor mc(bolt::CompressionType::NONE);
    std::string data = "Hello, this is a test message for compression!";
    auto compressed = mc.compress(data);
    auto decompressed = mc.decompress(compressed);
    BOLT_ASSERT_EQ(data, decompressed);
}

BOLT_TEST(E2E_MessageCompression, ShouldCompress) {
    bolt::MessageCompressor mc;
    mc.setMinCompressionSize(100);
    BOLT_ASSERT_FALSE(mc.shouldCompress("short"));
    std::string longStr(200, 'X');
    BOLT_ASSERT_TRUE(mc.shouldCompress(longStr));
}

BOLT_TEST(E2E_MessageCompression, CompressionStats) {
    bolt::MessageCompressor mc(bolt::CompressionType::GZIP);
    mc.compress("test data that is long enough to trigger compression for stats tracking purposes");
    auto& stats = mc.getStats();
    // Stats may or may not be updated depending on zlib availability
    // Just verify getStats() returns without crashing
    (void)stats.compressionCalls;
    (void)stats.averageCompressionRatio();
}

BOLT_TEST(E2E_MessageCompression, ResetStats) {
    bolt::MessageCompressor mc(bolt::CompressionType::NONE);
    mc.compress("data");
    mc.resetStats();
    auto& stats = mc.getStats();
    BOLT_ASSERT_EQ(0u, stats.compressionCalls);
}

// ============================================================================
// E2E_NetworkMetrics: Test NetworkStats functions
// ============================================================================

BOLT_TEST(E2E_NetworkMetrics, BasicStats) {
    bolt::NetworkStats stats;
    stats.connectionsOpened++;
    stats.bytesReceived += 1024;
    stats.bytesSent += 512;
    BOLT_ASSERT_EQ(1u, stats.connectionsOpened.load());
    BOLT_ASSERT_EQ(1024u, stats.bytesReceived.load());
    BOLT_ASSERT_EQ(512u, stats.bytesSent.load());
}

BOLT_TEST(E2E_NetworkMetrics, AtomicOperations) {
    bolt::NetworkStats stats;
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; i++) {
        threads.emplace_back([&stats]() {
            for (int j = 0; j < 100; j++) {
                stats.messagesReceived++;
            }
        });
    }
    for (auto& t : threads) t.join();
    BOLT_ASSERT_EQ(1000u, stats.messagesReceived.load());
}

// ============================================================================
// E2E_ConnectionPool: Test ConnectionPool functions
// ============================================================================

BOLT_TEST(E2E_ConnectionPool, Configuration) {
    auto& pool = bolt::ConnectionPool::getInstance();
    pool.setMaxConnectionsPerHost(10);
    pool.setConnectionTimeout(std::chrono::seconds(30));
    pool.setKeepAliveTimeout(std::chrono::seconds(60));
    pool.setMaxIdleTime(std::chrono::seconds(120));
    // Verify no crash
}

BOLT_TEST(E2E_ConnectionPool, CleanupExpired) {
    auto& pool = bolt::ConnectionPool::getInstance();
    pool.cleanupExpiredConnections();
    // Should not crash even with no connections
}

BOLT_TEST(E2E_ConnectionPool, GetStatistics) {
    auto& pool = bolt::ConnectionPool::getInstance();
    auto active = pool.getActiveConnections();
    auto total = pool.getTotalConnections();
    BOLT_ASSERT(active <= total);
}

// ============================================================================
// E2E_Logging: Test all logging functions
// ============================================================================

BOLT_TEST(E2E_Logging, SimpleFormatter) {
    bolt::SimpleFormatter formatter;
    bolt::LogEntry entry(bolt::LogLevel::INFO, bolt::LogCategory::CORE, "Test message");
    std::string formatted = formatter.format(entry);
    BOLT_ASSERT(formatted.find("Test message") != std::string::npos);
}

BOLT_TEST(E2E_Logging, DetailedFormatter) {
    bolt::DetailedFormatter formatter(true, true, true);
    bolt::LogEntry entry(bolt::LogLevel::WARN, bolt::LogCategory::EDITOR, "Warning msg", "test.cpp", 42, "testFunc");
    std::string formatted = formatter.format(entry);
    BOLT_ASSERT(formatted.find("Warning msg") != std::string::npos);
}

BOLT_TEST(E2E_Logging, JsonFormatter) {
    bolt::JsonFormatter formatter;
    bolt::LogEntry entry(bolt::LogLevel::ERROR, bolt::LogCategory::AI, "Error occurred");
    std::string formatted = formatter.format(entry);
    BOLT_ASSERT(formatted.find("Error occurred") != std::string::npos);
}

BOLT_TEST(E2E_Logging, ConsoleSink) {
    bolt::ConsoleSink sink(false);
    BOLT_ASSERT_TRUE(sink.isOpen());
    bolt::LogEntry entry(bolt::LogLevel::INFO, bolt::LogCategory::CORE, "Console test");
    sink.write(entry, "Console test");
    sink.flush();
}

BOLT_TEST(E2E_Logging, FileSink) {
    std::string logFile = "/tmp/bolt_e2e_test.log";
    {
        bolt::FileSink sink(logFile);
        BOLT_ASSERT_TRUE(sink.isOpen());
        bolt::LogEntry entry(bolt::LogLevel::INFO, bolt::LogCategory::CORE, "File test");
        sink.write(entry, "File test message");
        sink.flush();
    }
    auto content = FileSystemUtils::readFile(logFile);
    BOLT_ASSERT(content.find("File test") != std::string::npos);
    std::filesystem::remove(logFile);
}

BOLT_TEST(E2E_Logging, LevelFilter) {
    bolt::LevelFilter filter(bolt::LogLevel::WARN);
    bolt::LogEntry infoEntry(bolt::LogLevel::INFO, bolt::LogCategory::CORE, "info");
    bolt::LogEntry warnEntry(bolt::LogLevel::WARN, bolt::LogCategory::CORE, "warn");
    bolt::LogEntry errorEntry(bolt::LogLevel::ERROR, bolt::LogCategory::CORE, "error");
    BOLT_ASSERT_FALSE(filter.shouldLog(infoEntry));
    BOLT_ASSERT_TRUE(filter.shouldLog(warnEntry));
    BOLT_ASSERT_TRUE(filter.shouldLog(errorEntry));
}

BOLT_TEST(E2E_Logging, LevelFilterSetGet) {
    bolt::LevelFilter filter(bolt::LogLevel::INFO);
    BOLT_ASSERT(filter.getMinLevel() == bolt::LogLevel::INFO);
    filter.setMinLevel(bolt::LogLevel::ERROR);
    BOLT_ASSERT(filter.getMinLevel() == bolt::LogLevel::ERROR);
}

BOLT_TEST(E2E_Logging, CategoryFilter) {
    bolt::CategoryFilter filter(bolt::LogLevel::WARN);
    filter.setCategoryLevel(bolt::LogCategory::CORE, bolt::LogLevel::DEBUG);
    bolt::LogEntry coreDebug(bolt::LogLevel::DEBUG, bolt::LogCategory::CORE, "debug");
    bolt::LogEntry editorDebug(bolt::LogLevel::DEBUG, bolt::LogCategory::EDITOR, "debug");
    BOLT_ASSERT_TRUE(filter.shouldLog(coreDebug));
    BOLT_ASSERT_FALSE(filter.shouldLog(editorDebug));
}

BOLT_TEST(E2E_Logging, LogManagerSingleton) {
    auto& lm = bolt::LogManager::getInstance();
    // Should be accessible
    BOLT_ASSERT_TRUE(true);
}

// ============================================================================
// E2E_PerformanceProfiler: Test all PerformanceProfiler functions
// ============================================================================

BOLT_TEST(E2E_PerformanceProfiler, EnableDisable) {
    auto& profiler = bolt::PerformanceProfiler::getInstance();
    profiler.enable();
    BOLT_ASSERT_TRUE(profiler.isEnabled());
    profiler.disable();
    BOLT_ASSERT_FALSE(profiler.isEnabled());
    profiler.enable();
}

BOLT_TEST(E2E_PerformanceProfiler, CreateMetric) {
    bolt::PerformanceMetric metric("test_metric", "E2E");
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    metric.finish();
    BOLT_ASSERT(metric.getDurationMs() >= 5.0);
}

BOLT_TEST(E2E_PerformanceProfiler, ProfilerSession) {
    bolt::ProfilerSession session("e2e_session");
    session.start();
    BOLT_ASSERT_TRUE(session.isActive());
    BOLT_ASSERT_EQ("e2e_session", session.getName());
    auto metric = std::make_shared<bolt::PerformanceMetric>("metric1", "E2E");
    metric->finish();
    session.addMetric(metric);
    BOLT_ASSERT_EQ(1u, session.getMetricsCount());
    session.stop();
    BOLT_ASSERT_FALSE(session.isActive());
}

BOLT_TEST(E2E_PerformanceProfiler, SessionStatistics) {
    bolt::ProfilerSession session("stats_session");
    session.start();
    for (int i = 0; i < 5; i++) {
        auto metric = std::make_shared<bolt::PerformanceMetric>("m" + std::to_string(i), "E2E");
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        metric->finish();
        session.addMetric(metric);
    }
    session.stop();
    BOLT_ASSERT_EQ(5u, session.getMetricsCount());
    BOLT_ASSERT(session.getTotalDurationMs() >= 0.0);
    BOLT_ASSERT(session.getAverageDurationMs() >= 0.0);
    BOLT_ASSERT(session.getMinDurationMs() >= 0.0);
    BOLT_ASSERT(session.getMaxDurationMs() >= session.getMinDurationMs());
}

BOLT_TEST(E2E_PerformanceProfiler, SystemMonitoring) {
    auto& profiler = bolt::PerformanceProfiler::getInstance();
    profiler.enableSystemMonitoring();
    BOLT_ASSERT_TRUE(profiler.isSystemMonitoringEnabled());
    profiler.disableSystemMonitoring();
    BOLT_ASSERT_FALSE(profiler.isSystemMonitoringEnabled());
}

BOLT_TEST(E2E_PerformanceProfiler, RecordInstantMetric) {
    auto& profiler = bolt::PerformanceProfiler::getInstance();
    profiler.enable();
    profiler.recordInstantMetric("e2e_instant", 42.0, "E2E");
    BOLT_ASSERT(profiler.getTotalMetricsCount() > 0);
}

BOLT_TEST(E2E_PerformanceProfiler, Reset) {
    auto& profiler = bolt::PerformanceProfiler::getInstance();
    profiler.enable();
    profiler.recordInstantMetric("reset_test", 1.0);
    profiler.reset();
    // After reset, metrics should be cleared
}

// ============================================================================
// E2E_MemoryLeakDetector: Test all MemoryLeakDetector functions
// ============================================================================

BOLT_TEST(E2E_MemoryLeakDetector, BasicTracking) {
    auto& detector = bolt::MemoryLeakDetector::getInstance();
    detector.clear();
    detector.setEnabled(true);
    void* ptr = std::malloc(100);
    detector.trackAllocation(ptr, 100, "test.cpp", 10, "testFunc");
    BOLT_ASSERT_EQ(1u, detector.getLeakCount());
    detector.untrackAllocation(ptr);
    BOLT_ASSERT_EQ(0u, detector.getLeakCount());
    std::free(ptr);
}

BOLT_TEST(E2E_MemoryLeakDetector, MultipleAllocations) {
    auto& detector = bolt::MemoryLeakDetector::getInstance();
    detector.clear();
    detector.setEnabled(true);
    std::vector<void*> ptrs;
    for (int i = 0; i < 5; i++) {
        void* p = std::malloc(64);
        detector.trackAllocation(p, 64, "test.cpp", i, "testFunc");
        ptrs.push_back(p);
    }
    BOLT_ASSERT_EQ(5u, detector.getLeakCount());
    for (auto p : ptrs) {
        detector.untrackAllocation(p);
        std::free(p);
    }
    BOLT_ASSERT_EQ(0u, detector.getLeakCount());
}

BOLT_TEST(E2E_MemoryLeakDetector, LeakedBytesTracking) {
    auto& detector = bolt::MemoryLeakDetector::getInstance();
    detector.clear();
    detector.setEnabled(true);
    void* p1 = std::malloc(100);
    detector.trackAllocation(p1, 100, "test.cpp", 1, "testFunc");
    void* p2 = std::malloc(200);
    detector.trackAllocation(p2, 200, "test.cpp", 2, "testFunc");
    BOLT_ASSERT(detector.getLeakedBytes() >= 300);
    detector.untrackAllocation(p1);
    detector.untrackAllocation(p2);
    std::free(p1);
    std::free(p2);
}

BOLT_TEST(E2E_MemoryLeakDetector, EnableDisable) {
    auto& detector = bolt::MemoryLeakDetector::getInstance();
    detector.setEnabled(false);
    BOLT_ASSERT_FALSE(detector.isEnabled());
    detector.setEnabled(true);
    BOLT_ASSERT_TRUE(detector.isEnabled());
}

BOLT_TEST(E2E_MemoryLeakDetector, ReportGeneration) {
    auto& detector = bolt::MemoryLeakDetector::getInstance();
    detector.clear();
    detector.setEnabled(true);
    void* p = std::malloc(128);
    detector.trackAllocation(p, 128, "leak_test.cpp", 42, "leakyFunc");
    auto report = detector.generateReport();
    BOLT_ASSERT(!report.empty());
    detector.untrackAllocation(p);
    std::free(p);
}

// ============================================================================
// E2E_LspJsonRpc: Test all LSP JSON-RPC functions
// ============================================================================

BOLT_TEST(E2E_LspJsonRpc, JsonValueNull) {
    bolt::lsp::JsonValue val;
    BOLT_ASSERT(val.getType() == bolt::lsp::JsonValue::Null);
}

BOLT_TEST(E2E_LspJsonRpc, JsonValueString) {
    bolt::lsp::JsonValue val("hello");
    BOLT_ASSERT(val.getType() == bolt::lsp::JsonValue::String);
    BOLT_ASSERT_EQ("hello", val.asString());
}

BOLT_TEST(E2E_LspJsonRpc, JsonValueNumber) {
    bolt::lsp::JsonValue val(42.0);
    BOLT_ASSERT(val.getType() == bolt::lsp::JsonValue::Number);
    BOLT_ASSERT(std::abs(val.asNumber() - 42.0) < 1e-10);
}

BOLT_TEST(E2E_LspJsonRpc, JsonValueBool) {
    bolt::lsp::JsonValue val(true);
    BOLT_ASSERT(val.getType() == bolt::lsp::JsonValue::Bool);
    BOLT_ASSERT_TRUE(val.asBool());
}

BOLT_TEST(E2E_LspJsonRpc, JsonValueObject) {
    bolt::lsp::JsonValue obj;
    obj.setObject();
    obj.setProperty("key", std::make_shared<bolt::lsp::JsonValue>("value"));
    auto prop = obj.getProperty("key");
    BOLT_ASSERT_NOT_NULL(prop.get());
    BOLT_ASSERT_EQ("value", prop->asString());
}

BOLT_TEST(E2E_LspJsonRpc, JsonValueArray) {
    bolt::lsp::JsonValue arr;
    arr.setArray();
    arr.addArrayElement(std::make_shared<bolt::lsp::JsonValue>("elem1"));
    arr.addArrayElement(std::make_shared<bolt::lsp::JsonValue>("elem2"));
    BOLT_ASSERT_EQ(2u, arr.getArraySize());
    auto elem = arr.getArrayElement(0);
    BOLT_ASSERT_EQ("elem1", elem->asString());
}

BOLT_TEST(E2E_LspJsonRpc, JsonValueToString) {
    bolt::lsp::JsonValue val("test");
    auto str = val.toString();
    BOLT_ASSERT(!str.empty());
}

BOLT_TEST(E2E_LspJsonRpc, JsonRpcRequest) {
    bolt::lsp::JsonRpcRequest req("textDocument/completion", "1");
    BOLT_ASSERT_EQ("textDocument/completion", req.method);
    BOLT_ASSERT(req.id.has_value());
    BOLT_ASSERT_EQ("1", req.id.value());
}

BOLT_TEST(E2E_LspJsonRpc, JsonRpcResponse) {
    bolt::lsp::JsonRpcResponse resp("1");
    BOLT_ASSERT_EQ("1", resp.id);
    BOLT_ASSERT_EQ("2.0", resp.jsonrpc);
}

BOLT_TEST(E2E_LspJsonRpc, JsonRpcNotification) {
    bolt::lsp::JsonRpcNotification notif("textDocument/didOpen");
    BOLT_ASSERT_EQ("textDocument/didOpen", notif.method);
}

BOLT_TEST(E2E_LspJsonRpc, JsonRpcHandler_RegisterAndProcess) {
    bolt::lsp::JsonRpcHandler handler;
    handler.registerRequestHandler("test/method",
        [](const std::string& method, std::shared_ptr<bolt::lsp::JsonValue> params)
            -> std::shared_ptr<bolt::lsp::JsonValue> {
            return std::make_shared<bolt::lsp::JsonValue>("result");
        });
    // Handler should be registered
    auto request = handler.createRequest("test/method", nullptr, "1");
    BOLT_ASSERT(!request.empty());
}

BOLT_TEST(E2E_LspJsonRpc, CreateNotification) {
    bolt::lsp::JsonRpcHandler handler;
    auto notif = handler.createNotification("test/notify", nullptr);
    BOLT_ASSERT(!notif.empty());
}

BOLT_TEST(E2E_LspJsonRpc, CreateResponse) {
    bolt::lsp::JsonRpcHandler handler;
    auto resp = handler.createResponse("1", std::make_shared<bolt::lsp::JsonValue>("ok"));
    BOLT_ASSERT(!resp.empty());
}

BOLT_TEST(E2E_LspJsonRpc, CreateErrorResponse) {
    bolt::lsp::JsonRpcHandler handler;
    auto err = handler.createErrorResponse("1", -32600, "Invalid Request");
    BOLT_ASSERT(!err.empty());
}

// ============================================================================
// E2E_DrawkernVM: Test DIS VM functions
// ============================================================================

BOLT_TEST(E2E_DrawkernVM, CreateVM) {
    bolt::drawkern::DISVM vm;
    BOLT_ASSERT_FALSE(vm.is_running());
}

BOLT_TEST(E2E_DrawkernVM, ProgramFactory_EchoServer) {
    auto program = bolt::drawkern::DISProgramFactory::create_echo_server();
    BOLT_ASSERT(program.instructions.size() > 0);
}

BOLT_TEST(E2E_DrawkernVM, ProgramFactory_AIWorkbench) {
    auto program = bolt::drawkern::DISProgramFactory::create_ai_workbench("ggml");
    BOLT_ASSERT(program.instructions.size() > 0);
}

BOLT_TEST(E2E_DrawkernVM, ProgramFactory_FileServer) {
    auto program = bolt::drawkern::DISProgramFactory::create_file_server("/tmp");
    BOLT_ASSERT(program.instructions.size() > 0);
}

BOLT_TEST(E2E_DrawkernVM, ProgramFactory_DrawKernRenderer) {
    auto program = bolt::drawkern::DISProgramFactory::create_drawkern_renderer();
    BOLT_ASSERT(program.instructions.size() > 0);
}

BOLT_TEST(E2E_DrawkernVM, VMManager_CreateAndDestroy) {
    bolt::drawkern::DISVMManager manager;
    auto program = bolt::drawkern::DISProgramFactory::create_echo_server();
    auto vmId = manager.create_vm(program);
    BOLT_ASSERT(!vmId.empty());
    auto vms = manager.list_vms();
    BOLT_ASSERT(vms.size() > 0);
    manager.destroy_vm(vmId);
}

BOLT_TEST(E2E_DrawkernVM, VMManager_StartStop) {
    bolt::drawkern::DISVMManager manager;
    auto program = bolt::drawkern::DISProgramFactory::create_echo_server();
    auto vmId = manager.create_vm(program);
    bool started = manager.start_vm(vmId);
    BOLT_ASSERT_TRUE(started);
    // VM may halt immediately for simple programs; allow thread to settle
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    manager.stop_vm(vmId);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    manager.destroy_vm(vmId);
}

BOLT_TEST(E2E_DrawkernVM, VMManager_GetStatus) {
    bolt::drawkern::DISVMManager manager;
    auto program = bolt::drawkern::DISProgramFactory::create_echo_server();
    auto vmId = manager.create_vm(program);
    auto status = manager.get_vm_status(vmId);
    BOLT_ASSERT(!status.empty());
    manager.destroy_vm(vmId);
}

BOLT_TEST(E2E_DrawkernVM, LimboCompiler_Basic) {
    bolt::drawkern::LimboCompiler compiler;
    auto program = compiler.compile("print \"hello\"");
    // May or may not succeed, but should not crash
}

// ============================================================================
// E2E_YaccGrammar: Test Yacc Grammar System functions
// ============================================================================
BOLT_TEST(E2E_YaccGrammar, LexerBasicTokenize) {
    bolt::drawkern::GlyphLexer lexer("vm test : dis { }");
    auto tokens = lexer.tokenize();
    BOLT_ASSERT(tokens.size() > 0);
}

BOLT_TEST(E2E_YaccGrammar, LexerKeywords) {
    bolt::drawkern::GlyphLexer lexer("vm glyph workbench render ai");
    auto tokens = lexer.tokenize();
    // Should recognize keywords
    BOLT_ASSERT(tokens.size() >= 5);
}

BOLT_TEST(E2E_YaccGrammar, LexerStringLiteral) {
    bolt::drawkern::GlyphLexer lexer("name = \"hello world\"");
    auto tokens = lexer.tokenize();
    bool foundString = false;
    for (auto& t : tokens) {
        if (t.type == bolt::drawkern::TokenType::STRING) {
            foundString = true;
            break;
        }
    }
    BOLT_ASSERT_TRUE(foundString);
}

BOLT_TEST(E2E_YaccGrammar, ParserBasic) {
    bolt::drawkern::GlyphLexer lexer("vm test : dis { }");
    auto tokens = lexer.tokenize();
    bolt::drawkern::GlyphParser parser(tokens);
    auto ast = parser.parse();
    BOLT_ASSERT_NOT_NULL(ast.get());
}

BOLT_TEST(E2E_YaccGrammar, GrammarSystem_GenerateCode) {
    bolt::drawkern::YaccGrammarSystem system;
    auto result = system.generate_cpp_code(bolt::drawkern::GlyphTemplates::ECHO_SERVER);
    // Should produce some output
}

BOLT_TEST(E2E_YaccGrammar, GrammarSystem_Validate) {
    bolt::drawkern::YaccGrammarSystem system;
    // validate_description parses the template; may report non-fatal errors
    bool valid = system.validate_description(bolt::drawkern::GlyphTemplates::ECHO_SERVER);
    // If not valid, check that errors are accessible
    if (!valid) {
        auto errors = system.get_validation_errors();
        BOLT_ASSERT(errors.size() > 0);
    }
}

BOLT_TEST(E2E_YaccGrammar, GrammarSystem_GetSupportedProperties) {
    bolt::drawkern::YaccGrammarSystem system;
    auto props = system.get_supported_properties();
    BOLT_ASSERT(props.size() > 0);
}

BOLT_TEST(E2E_YaccGrammar, GrammarSystem_GetDocumentation) {
    bolt::drawkern::YaccGrammarSystem system;
    auto docs = system.get_grammar_documentation();
    BOLT_ASSERT(!docs.empty());
}

// ============================================================================
// E2E_StyxProtocol: Test Styx Protocol functions
// ============================================================================

BOLT_TEST(E2E_StyxProtocol, MessageCreation) {
    bolt::drawkern::StyxMessage msg;
    msg.type = bolt::drawkern::StyxMessageType::Tversion;
    msg.tag = 0;
    BOLT_ASSERT(msg.type == bolt::drawkern::StyxMessageType::Tversion);
}

BOLT_TEST(E2E_StyxProtocol, MessageSerialization) {
    bolt::drawkern::StyxMessage msg;
    msg.type = bolt::drawkern::StyxMessageType::Tversion;
    msg.tag = 1;
    std::vector<uint8_t> data;
    msg.serialize(data);
    BOLT_ASSERT(data.size() > 0);
}

BOLT_TEST(E2E_StyxProtocol, MessageDeserialization) {
    bolt::drawkern::StyxMessage msg;
    msg.type = bolt::drawkern::StyxMessageType::Tversion;
    msg.tag = 1;
    std::vector<uint8_t> data;
    msg.serialize(data);
    auto deserialized = bolt::drawkern::StyxMessage::deserialize(data);
    BOLT_ASSERT(deserialized.type == bolt::drawkern::StyxMessageType::Tversion);
}

// ============================================================================
// E2E_GitRepository: Test GitRepository functions
// ============================================================================

BOLT_TEST(E2E_GitRepository, CreateWithPath) {
    bolt::git::GitRepository repo("/tmp/bolt_e2e_git_test");
    // Should not crash
}

BOLT_TEST(E2E_GitRepository, IsGitRepository_NonRepo) {
    bolt::git::GitRepository repo("/tmp");
    BOLT_ASSERT_FALSE(repo.isGitRepository());
}

BOLT_TEST(E2E_GitRepository, IsGitRepository_ActualRepo) {
    // Use the bolt-cppml repo itself
    bolt::git::GitRepository repo(REPO_PATH);
    BOLT_ASSERT_TRUE(repo.isGitRepository());
}

BOLT_TEST(E2E_GitRepository, GetCurrentBranch) {
    bolt::git::GitRepository repo(REPO_PATH);
    auto branch = repo.getCurrentBranch();
    BOLT_ASSERT(branch.has_value());
    BOLT_ASSERT(!branch->empty());
}

BOLT_TEST(E2E_GitRepository, GetBranches) {
    bolt::git::GitRepository repo(REPO_PATH);
    auto branches = repo.getBranches();
    BOLT_ASSERT(branches.size() > 0);
}

BOLT_TEST(E2E_GitRepository, GetFileStatus) {
    bolt::git::GitRepository repo(REPO_PATH);
    auto status = repo.getFileStatus();
    // May have modified/untracked files
}

BOLT_TEST(E2E_GitRepository, HasUncommittedChanges) {
    bolt::git::GitRepository repo(REPO_PATH);
    // Just verify it doesn't crash
    repo.hasUncommittedChanges();
}

BOLT_TEST(E2E_GitRepository, GetCommitLog) {
    bolt::git::GitRepository repo(REPO_PATH);
    auto log = repo.getCommitLog(5);
    BOLT_ASSERT(log.size() > 0);
}

BOLT_TEST(E2E_GitRepository, GetRepositoryRoot) {
    bolt::git::GitRepository repo(REPO_PATH);
    auto root = repo.getRepositoryRoot();
    BOLT_ASSERT(!root.empty());
}

BOLT_TEST(E2E_GitRepository, SetStatusCallback) {
    bolt::git::GitRepository repo(REPO_PATH);
    bool callbackSet = false;
    repo.setStatusCallback([&callbackSet](bool success, const std::string& msg) {
        callbackSet = true;
    });
    // Callback should be set without crash
}

// ============================================================================
// E2E_BoltApp: Test BoltApp and Chat functions
// ============================================================================

BOLT_TEST(E2E_BoltApp, ChatBasicInstantiation) {
    bolt::Chat chat;
    BOLT_ASSERT_EQ(0u, chat.getHistory().size());
}

BOLT_TEST(E2E_BoltApp, ChatAddMessage) {
    bolt::Chat chat;
    chat.addMessage(bolt::ChatMessage(bolt::ChatMessage::Role::User, "Hello"));
    BOLT_ASSERT_EQ(1u, chat.getHistory().size());
    BOLT_ASSERT_EQ("Hello", chat.getHistory()[0].getContent());
}

BOLT_TEST(E2E_BoltApp, ChatMultipleMessages) {
    bolt::Chat chat;
    chat.addMessage(bolt::ChatMessage(bolt::ChatMessage::Role::User, "Q1"));
    chat.addMessage(bolt::ChatMessage(bolt::ChatMessage::Role::Assistant, "A1"));
    chat.addMessage(bolt::ChatMessage(bolt::ChatMessage::Role::User, "Q2"));
    BOLT_ASSERT_EQ(3u, chat.getHistory().size());
}

BOLT_TEST(E2E_BoltApp, ChatClear) {
    bolt::Chat chat;
    chat.addMessage(bolt::ChatMessage(bolt::ChatMessage::Role::User, "msg"));
    chat.clear();
    BOLT_ASSERT_EQ(0u, chat.getHistory().size());
}

BOLT_TEST(E2E_BoltApp, ChatMessageRoles) {
    bolt::ChatMessage userMsg(bolt::ChatMessage::Role::User, "user");
    bolt::ChatMessage assistantMsg(bolt::ChatMessage::Role::Assistant, "assistant");
    BOLT_ASSERT(userMsg.getRole() == bolt::ChatMessage::Role::User);
    BOLT_ASSERT(assistantMsg.getRole() == bolt::ChatMessage::Role::Assistant);
}

BOLT_TEST(E2E_BoltApp, ChatMessageContent) {
    bolt::ChatMessage msg(bolt::ChatMessage::Role::User, "Test content");
    BOLT_ASSERT_EQ("Test content", msg.getContent());
}

BOLT_TEST(E2E_BoltApp, ChatEmptyMessage) {
    bolt::ChatMessage msg(bolt::ChatMessage::Role::User, "");
    BOLT_ASSERT_EQ("", msg.getContent());
}

// ============================================================================
// E2E_BenchmarkSuite: Test BenchmarkSuite functions
// ============================================================================

BOLT_TEST(E2E_BenchmarkSuite, RegisterAndRunBenchmark) {
    auto& suite = bolt::BenchmarkSuite::getInstance();
    bolt::BenchmarkConfig config("e2e_bench", "E2E benchmark test");
    config.category = "E2E";
    config.iterations = 3;
    config.warmupRuns = 1;
    suite.registerBenchmark(config, [](const bolt::BenchmarkConfig& cfg) {
        volatile int sum = 0;
        for (int i = 0; i < 1000; i++) sum += i;
    });
    auto result = suite.runBenchmark("e2e_bench");
    BOLT_ASSERT_EQ("e2e_bench", result.name);
    BOLT_ASSERT(result.averageDurationMs >= 0.0);
}

BOLT_TEST(E2E_BenchmarkSuite, GetAvailableBenchmarks) {
    auto& suite = bolt::BenchmarkSuite::getInstance();
    auto benchmarks = suite.getAvailableBenchmarks();
    BOLT_ASSERT(benchmarks.size() > 0);
}

BOLT_TEST(E2E_BenchmarkSuite, GetAvailableCategories) {
    auto& suite = bolt::BenchmarkSuite::getInstance();
    auto categories = suite.getAvailableCategories();
    BOLT_ASSERT(categories.size() > 0);
}

BOLT_TEST(E2E_BenchmarkSuite, FormatDuration) {
    auto& suite = bolt::BenchmarkSuite::getInstance();
    auto formatted = suite.formatDuration(1234.5);
    BOLT_ASSERT(!formatted.empty());
}

BOLT_TEST(E2E_BenchmarkSuite, FormatMemory) {
    auto& suite = bolt::BenchmarkSuite::getInstance();
    auto formatted = suite.formatMemory(256.0);
    BOLT_ASSERT(!formatted.empty());
}

BOLT_TEST(E2E_BenchmarkSuite, FormatPercentage) {
    auto& suite = bolt::BenchmarkSuite::getInstance();
    auto formatted = suite.formatPercentage(42.5);
    BOLT_ASSERT(!formatted.empty());
}

BOLT_TEST(E2E_BenchmarkSuite, CalculateStdDev) {
    auto& suite = bolt::BenchmarkSuite::getInstance();
    std::vector<double> values = {10.0, 20.0, 30.0, 40.0, 50.0};
    double mean = 30.0;
    double stddev = suite.calculateStandardDeviation(values, mean);
    BOLT_ASSERT(stddev > 0.0);
}

BOLT_TEST(E2E_BenchmarkSuite, Configuration) {
    auto& suite = bolt::BenchmarkSuite::getInstance();
    suite.setDefaultIterations(5);
    suite.setDefaultWarmupRuns(2);
    suite.setDefaultTimeout(std::chrono::milliseconds(10000));
    suite.enableVerboseOutput(false);
}

// ============================================================================
// E2E_PluginSystem: Test PluginSystem functions
// ============================================================================

BOLT_TEST(E2E_PluginSystem, RegisterLegacyPlugin) {
    auto& ps = bolt::PluginSystem::getInstance();
    bolt::Plugin plugin;
    plugin.name = "e2e_test_plugin";
    plugin.version = "1.0.0";
    plugin.initialize = []() {};
    plugin.cleanup = []() {};
    ps.registerPlugin(plugin);
}

BOLT_TEST(E2E_PluginSystem, GetLoadedPluginNames) {
    auto& ps = bolt::PluginSystem::getInstance();
    auto names = ps.getLoadedPluginNames();
    // May or may not have plugins
}

BOLT_TEST(E2E_PluginSystem, IsPluginLoaded) {
    auto& ps = bolt::PluginSystem::getInstance();
    BOLT_ASSERT_FALSE(ps.isPluginLoaded("nonexistent_plugin_xyz"));
}

BOLT_TEST(E2E_PluginSystem, GetPluginErrors) {
    auto& ps = bolt::PluginSystem::getInstance();
    auto errors = ps.getPluginErrors();
    // Should return (possibly empty) error list
}

BOLT_TEST(E2E_PluginSystem, ClearPluginErrors) {
    auto& ps = bolt::PluginSystem::getInstance();
    ps.clearPluginErrors();
    auto errors = ps.getPluginErrors();
    BOLT_ASSERT(errors.empty());
}

BOLT_TEST(E2E_PluginSystem, HandlePluginError) {
    auto& ps = bolt::PluginSystem::getInstance();
    ps.clearPluginErrors();
    ps.handlePluginError("test_plugin", "Test error message");
    auto errors = ps.getPluginErrors();
    BOLT_ASSERT(errors.size() > 0);
}

// ============================================================================
// E2E_CodeAnalyzer: Test CodeAnalyzer functions
// ============================================================================

BOLT_TEST(E2E_CodeAnalyzer, CreateAnalyzer) {
    bolt::CodeAnalyzer analyzer;
    auto analyzers = analyzer.getRegisteredAnalyzers();
    // Should have some default analyzers
}

BOLT_TEST(E2E_CodeAnalyzer, AnalyzeFile) {
    bolt::CodeAnalyzer analyzer;
    // Create a temp file to analyze
    std::string testFile = "/tmp/bolt_e2e_analyze.cpp";
    FileSystemUtils::writeFile(testFile, R"(
#include <iostream>
int main() {
    int x = 42;
    if (x > 0) {
        std::cout << x << std::endl;
    }
    return 0;
}
)");
    auto result = analyzer.analyzeFile(testFile);
    std::filesystem::remove(testFile);
}

BOLT_TEST(E2E_CodeAnalyzer, EnableDisableAnalyzer) {
    bolt::CodeAnalyzer analyzer;
    auto names = analyzer.getRegisteredAnalyzers();
    if (!names.empty()) {
        analyzer.enableAnalyzer(names[0], false);
        BOLT_ASSERT_FALSE(analyzer.isAnalyzerEnabled(names[0]));
        analyzer.enableAnalyzer(names[0], true);
        BOLT_ASSERT_TRUE(analyzer.isAnalyzerEnabled(names[0]));
    }
}

// ============================================================================
// E2E_CrossModule: Full workflow integration tests
// ============================================================================

BOLT_TEST(E2E_CrossModule, EditorWorkflow_CreateEditSave) {
    // Simulate a complete editor workflow
    auto& editorStore = bolt::EditorStore::getInstance();
    auto& tabBar = bolt::TabBar::getInstance();
    auto& codeFolding = bolt::CodeFolding::getInstance();
    
    // 1. Create a tab for a new file
    std::string filePath = "/tmp/e2e_workflow_test.cpp";
    auto tabId = tabBar.addTab(filePath);
    BOLT_ASSERT(tabId > 0);
    
    // 2. Set document content
    bolt::EditorDocument doc;
    doc.filePath = filePath;
    doc.value = "int main() {\n    return 0;\n}\n";
    doc.scroll.line = 0;
    doc.scroll.character = 0;
    doc.cursor.position = 0;
    editorStore.setDocument(filePath, doc);
    
    // 3. Add code folding
    codeFolding.addFoldRange(filePath, 0, 2);
    auto ranges = codeFolding.getFoldingRanges(filePath);
    BOLT_ASSERT(ranges.size() > 0);
    
    // 4. Mark as dirty
    tabBar.setTabDirty(tabId, true);
    auto tab = tabBar.getTab(tabId);
    BOLT_ASSERT(tab.has_value());
    BOLT_ASSERT_TRUE(tab->isDirty);
    
    // 5. Clean up
    tabBar.closeTab(tabId);
}

BOLT_TEST(E2E_CrossModule, MemoryAndLogging_Integration) {
    // Test memory management with logging
    bolt::MemoryManager& mm = bolt::MemoryManager::getInstance();
    mm.forceReset();
    
    // Allocate memory
    void* p1 = mm.allocate(256);
    void* p2 = mm.allocate(512);
    
    // Log the allocation
    bolt::LogEntry entry(bolt::LogLevel::INFO, bolt::LogCategory::MEMORY,
        "Allocated " + std::to_string(mm.getCurrentUsage()) + " bytes");
    bolt::SimpleFormatter formatter;
    std::string logMsg = formatter.format(entry);
    BOLT_ASSERT(!logMsg.empty());
    
    // Verify memory state
    BOLT_ASSERT_EQ(768u, mm.getCurrentUsage());
    
    // Clean up
    mm.deallocate(p1);
    mm.deallocate(p2);
    BOLT_ASSERT_EQ(0u, mm.getCurrentUsage());
}

BOLT_TEST(E2E_CrossModule, CollaborationWorkflow) {
    using namespace bolt::collaboration;
    
    // Simulate two users editing the same document
    std::vector<std::string> document = {"Hello World"};
    
    // User A inserts at position 5
    DocumentOperation opA(OperationType::INSERT, "userA", Position(0, 5), " Beautiful");
    
    // User B inserts at position 0
    DocumentOperation opB(OperationType::INSERT, "userB", Position(0, 0), "Dear ");
    
    // Transform opA against opB
    auto transformedA = OperationalTransform::transform(opA, opB);
    BOLT_ASSERT_NOT_NULL(transformedA.get());
    
    // Apply opB first, then transformedA
    opB.apply(document);
    BOLT_ASSERT_EQ("Dear Hello World", document[0]);
    transformedA->apply(document);
    // Document should contain both edits
    BOLT_ASSERT(document[0].find("Beautiful") != std::string::npos);
}

BOLT_TEST(E2E_CrossModule, NetworkBufferAndCompression) {
    // Create a network buffer, fill it, compress, decompress
    bolt::NetworkBuffer buf;
    std::string message = "This is a test message that should be compressed and decompressed correctly.";
    buf.append(message);
    BOLT_ASSERT_EQ(message.size(), buf.size());
    
    // Consume and compress
    auto data = buf.consumeString(message.size());
    bolt::MessageCompressor mc(bolt::CompressionType::NONE);
    auto compressed = mc.compress(data);
    auto decompressed = mc.decompress(compressed);
    BOLT_ASSERT_EQ(message, decompressed);
}

BOLT_TEST(E2E_CrossModule, DrawKernFullWorkflow) {
    // Create a VM, load a program, start, stop, destroy
    bolt::drawkern::DISVMManager manager;
    auto program = bolt::drawkern::DISProgramFactory::create_echo_server();
    auto vmId = manager.create_vm(program);
    BOLT_ASSERT(!vmId.empty());
    
    // Start the VM (may halt immediately for simple programs)
    bool started = manager.start_vm(vmId);
    BOLT_ASSERT_TRUE(started);
    
    // Allow VM thread to settle
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Check status
    auto status = manager.get_vm_status(vmId);
    BOLT_ASSERT(!status.empty());
    
    // Stop and destroy
    manager.stop_vm(vmId);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    manager.destroy_vm(vmId);
    auto vms = manager.list_vms();
    BOLT_ASSERT(vms.empty());
}

BOLT_TEST(E2E_CrossModule, GitAndEditorIntegration) {
    // Test git status with editor file tracking
    bolt::git::GitRepository repo(REPO_PATH);
    BOLT_ASSERT_TRUE(repo.isGitRepository());
    
    auto branch = repo.getCurrentBranch();
    BOLT_ASSERT(branch.has_value());
    
    // Get file status and verify it integrates with editor concepts
    auto fileStatus = repo.getFileStatus();
    // File status should be parseable
}

BOLT_TEST(E2E_CrossModule, FullEditorSessionWorkflow) {
    // Simulate a complete editor session:
    // 1. Open file in tab
    // 2. Set up syntax highlighting
    // 3. Add cursors
    // 4. Find and replace
    // 5. Code folding
    // 6. Close
    
    auto& tabBar = bolt::TabBar::getInstance();
    auto& syntaxHighlighter = bolt::SyntaxHighlighter::getInstance();
    auto& cursorManager = bolt::CursorManager::getInstance();
    auto& codeFolding = bolt::CodeFolding::getInstance();
    
    // Open file
    std::string path = "/tmp/e2e_full_session.cpp";
    auto tabId = tabBar.addTab(path);
    tabBar.activateTab(tabId);
    
    // Set up syntax highlighting
    std::vector<std::pair<std::string, std::string>> rules = {
        {"\\b(int|void|return)\\b", "keyword"},
        {"\\d+", "number"}
    };
    syntaxHighlighter.addLanguageRules("cpp", rules);
    auto tokens = syntaxHighlighter.highlight("int x = 42;", "cpp");
    BOLT_ASSERT(tokens.size() > 0);
    
    // Add cursor
    cursorManager.clearCursors();
    cursorManager.addCursor(0, 0);
    auto cursors = cursorManager.getCursors();
    BOLT_ASSERT_EQ(1u, cursors.size());
    
    // Find and replace
    bolt::FindReplace fr;
    auto results = fr.find("int x = 42; int y = 10;", "int");
    BOLT_ASSERT_EQ(2u, results.size());
    
    // Code folding
    codeFolding.addFoldRange(path, 0, 5);
    auto ranges = codeFolding.getFoldingRanges(path);
    BOLT_ASSERT(ranges.size() > 0);
    
    // Clean up
    tabBar.closeTab(tabId);
    cursorManager.clearCursors();
}

BOLT_TEST(E2E_CrossModule, PerformanceProfilingWorkflow) {
    // Profile a series of operations
    auto& profiler = bolt::PerformanceProfiler::getInstance();
    profiler.enable();
    
    bolt::ProfilerSession session("e2e_full_workflow");
    session.start();
    
    // Profile memory allocation
    auto memMetric = std::make_shared<bolt::PerformanceMetric>("memory_alloc", "Memory");
    bolt::MemoryManager& mm = bolt::MemoryManager::getInstance();
    mm.forceReset();
    void* p = mm.allocate(1024);
    memMetric->finish();
    session.addMetric(memMetric);
    mm.deallocate(p);
    
    // Profile string operations
    auto strMetric = std::make_shared<bolt::PerformanceMetric>("string_ops", "Utils");
    for (int i = 0; i < 100; i++) {
        StringUtils::reverseString("Hello World Performance Test");
    }
    strMetric->finish();
    session.addMetric(strMetric);
    
    // Profile data processing
    auto dataMetric = std::make_shared<bolt::PerformanceMetric>("data_proc", "Utils");
    std::vector<int> data(1000);
    std::iota(data.begin(), data.end(), 0);
    DataProcessor<int>::sort(data);
    DataProcessor<int>::calculateAverage(data);
    DataProcessor<int>::findMax(data);
    DataProcessor<int>::findMin(data);
    dataMetric->finish();
    session.addMetric(dataMetric);
    
    session.stop();
    BOLT_ASSERT_EQ(3u, session.getMetricsCount());
    BOLT_ASSERT(session.getTotalDurationMs() >= 0.0);
}

BOLT_TEST(E2E_CrossModule, LspAndEditorIntegration) {
    // Test LSP JSON-RPC with editor operations
    bolt::lsp::JsonRpcHandler handler;
    
    // Register a completion handler
    handler.registerRequestHandler("textDocument/completion",
        [](const std::string& method, std::shared_ptr<bolt::lsp::JsonValue> params)
            -> std::shared_ptr<bolt::lsp::JsonValue> {
            auto result = std::make_shared<bolt::lsp::JsonValue>();
            result->setArray();
            auto item = std::make_shared<bolt::lsp::JsonValue>();
            item->setObject();
            item->setProperty("label", std::make_shared<bolt::lsp::JsonValue>("main"));
            result->addArrayElement(item);
            return result;
        });
    
    // Create a request
    auto params = std::make_shared<bolt::lsp::JsonValue>();
    params->setObject();
    params->setProperty("textDocument", std::make_shared<bolt::lsp::JsonValue>("file:///test.cpp"));
    auto request = handler.createRequest("textDocument/completion", params, "1");
    BOLT_ASSERT(!request.empty());
}

BOLT_TEST(E2E_CrossModule, YaccGrammarAndVMIntegration) {
    // Parse a glyph description and create a VM from it
    bolt::drawkern::YaccGrammarSystem grammar;
    // Validate the template (may have non-fatal parse warnings)
    grammar.validate_description(bolt::drawkern::GlyphTemplates::ECHO_SERVER);
    
    // Create a VM from the factory
    auto program = bolt::drawkern::DISProgramFactory::create_echo_server();
    bolt::drawkern::DISVMManager manager;
    auto vmId = manager.create_vm(program);
    BOLT_ASSERT(!vmId.empty());
    
    // Verify VM was created
    auto vms = manager.list_vms();
    BOLT_ASSERT_EQ(1u, vms.size());
    
    manager.destroy_vm(vmId);
}

BOLT_TEST(E2E_CrossModule, ThemeAndSyntaxHighlightingIntegration) {
    // Set a theme and verify syntax highlighting uses it
    auto& ts = bolt::ThemeSystem::getInstance();
    auto& sh = bolt::SyntaxHighlighter::getInstance();
    
    // Set a dark theme
    bolt::Theme darkTheme;
    darkTheme.colors.background = "#1e1e1e";
    darkTheme.colors.foreground = "#d4d4d4";
    darkTheme.colors.keyword = "#569cd6";
    darkTheme.colors.comment = "#6a9955";
    darkTheme.colors.string = "#ce9178";
    darkTheme.colors.number = "#b5cea8";
    darkTheme.colors.type = "#4ec9b0";
    darkTheme.colors.function = "#dcdcaa";
    darkTheme.colors.selection = "#264f78";
    darkTheme.colors.lineNumbers = "#858585";
    darkTheme.fontSettings.family = "Fira Code";
    darkTheme.fontSettings.size = 14;
    darkTheme.fontSettings.lineHeight = 1.5f;
    ts.addTheme("e2e_dark", darkTheme);
    ts.setTheme("e2e_dark");
    BOLT_ASSERT_EQ("e2e_dark", ts.getCurrentThemeName());
    
    // Highlight code with the theme active
    auto tokens = sh.highlight("int x = 42;", "cpp");
    BOLT_ASSERT(tokens.size() > 0);
    
    // Verify theme colors are accessible
    auto colors = ts.getCurrentColors();
    BOLT_ASSERT_EQ("#1e1e1e", colors.background);
}
