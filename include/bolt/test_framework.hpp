#ifndef BOLT_TEST_FRAMEWORK_HPP_
#define BOLT_TEST_FRAMEWORK_HPP_

#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <functional>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace bolt {
namespace test {

// Returns a path in the system temporary directory for a file or directory
// that a test creates. The same test sources are built into several test
// programs and CTest runs those in parallel, so a fixed path would be shared
// between processes. The result is `name` with a tag inserted before its
// extension; the tag is chosen once per process, so the same name always
// gives the same path within one run and a different one in another process.
inline std::string uniqueTempPath(const std::string& name) {
    static const std::string process_tag = [] {
        static const char anchor = 0;
        const auto ticks = static_cast<std::uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
        const auto address = static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(&anchor));
        std::random_device entropy;
        std::ostringstream tag;
        tag << std::hex << (ticks ^ (address << 20)) << entropy();
        return tag.str();
    }();

    const std::filesystem::path file(name);
    const std::string unique_name = file.stem().string() + "_" + process_tag + file.extension().string();
    return (std::filesystem::temp_directory_path() / unique_name).string();
}

class TestFailure : public std::exception {
 public:
    TestFailure(const std::string& message, const std::string& file, int line)
        : message_(message), file_(file), line_(line) {
        std::stringstream ss;
        ss << file << ":" << line << " - " << message;
        full_message_ = ss.str();
    }

    const char* what() const noexcept override { return full_message_.c_str(); }

    const std::string& getMessage() const { return message_; }
    const std::string& getFile() const { return file_; }
    int getLine() const { return line_; }

 private:
    std::string message_;
    std::string file_;
    int line_;
    std::string full_message_;
};

class TestSuite {
 public:
    struct TestCase {
        std::string name;
        std::function<void()> test_func;
        std::string suite_name;
    };

    struct TestResult {
        std::string name;
        bool passed;
        std::string error_message;
        double duration_ms;
        std::string suite_name;
    };

    static TestSuite& getInstance() {
        static TestSuite instance;
        return instance;
    }

    void addTest(const std::string& suite_name, const std::string& test_name, std::function<void()> test_func) {
        tests_.push_back({test_name, test_func, suite_name});
    }

    int runAllTests(bool verbose = true) {
        std::vector<TestResult> results;
        int passed = 0, failed = 0;

        if (verbose) {
            std::cout << "Running " << tests_.size() << " tests...\n" << std::endl;
        }

        for (const auto& test : tests_) {
            runSingleTest(test, verbose, passed, failed);
        }

        if (verbose) {
            std::cout << std::endl;
            printSummary(passed, failed);
        }

        return failed;
    }

    int runTestsInSuite(const std::string& suite_name, bool verbose = true) {
        std::vector<TestResult> results;
        int passed = 0, failed = 0;
        int total_in_suite = 0;

        // Count tests in suite
        for (const auto& test : tests_) {
            if (test.suite_name == suite_name) {
                total_in_suite++;
            }
        }

        if (total_in_suite == 0) {
            if (verbose) {
                std::cout << "No tests found in suite: " << suite_name << std::endl;
            }
            return 0;
        }

        if (verbose) {
            std::cout << "Running " << total_in_suite << " tests in suite '" << suite_name << "'...\n" << std::endl;
        }

        for (const auto& test : tests_) {
            if (test.suite_name != suite_name)
                continue;

            runSingleTest(test, verbose, passed, failed);
        }

        if (verbose) {
            std::cout << std::endl;
            printSummary(passed, failed);
        }

        return failed;
    }

    const std::vector<TestResult>& getResults() const { return results_; }

    void listTests() const {
        std::cout << "Available tests:\n" << std::endl;
        std::string current_suite = "";
        for (const auto& test : tests_) {
            if (test.suite_name != current_suite) {
                current_suite = test.suite_name;
                std::cout << "Suite: " << current_suite << std::endl;
            }
            std::cout << "  " << test.name << std::endl;
        }
        std::cout << std::endl;
        std::cout << "Total: " << tests_.size() << " tests" << std::endl;
    }

 private:
    std::vector<TestCase> tests_;
    std::vector<TestResult> results_;

    // Runs one test case, prints its outcome when verbose, updates the pass/fail counters and records the result.
    void runSingleTest(const TestCase& test, bool verbose, int& passed, int& failed) {
        if (verbose) {
            std::cout << "[" << test.suite_name << "] " << test.name << " ... ";
            std::cout.flush();
        }

        auto start = std::chrono::high_resolution_clock::now();
        TestResult result;
        result.name = test.name;
        result.suite_name = test.suite_name;

        try {
            test.test_func();
            result.passed = true;
            passed++;
            if (verbose) {
                std::cout << "PASS" << std::endl;
            }
        } catch (const TestFailure& e) {
            result.passed = false;
            result.error_message = e.what();
            failed++;
            if (verbose) {
                std::cout << "FAIL" << std::endl;
                std::cout << "  " << e.what() << std::endl;
            }
        } catch (const std::exception& e) {
            result.passed = false;
            result.error_message = "Unexpected exception: " + std::string(e.what());
            failed++;
            if (verbose) {
                std::cout << "ERROR" << std::endl;
                std::cout << "  " << result.error_message << std::endl;
            }
        } catch (...) {
            result.passed = false;
            result.error_message = "Unexpected exception: Unknown exception";
            failed++;
            if (verbose) {
                std::cout << "ERROR" << std::endl;
                std::cout << "  " << result.error_message << std::endl;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        result.duration_ms = duration.count() / 1000.0;

        results_.push_back(result);
    }

    void printSummary(int passed, int failed) {
        std::cout << "Test Results:" << std::endl;
        std::cout << "  Passed: " << passed << std::endl;
        std::cout << "  Failed: " << failed << std::endl;
        std::cout << "  Total:  " << (passed + failed) << std::endl;

        if (failed == 0) {
            std::cout << std::endl << "All tests passed!" << std::endl;
        } else {
            std::cout << std::endl << failed << " test(s) failed." << std::endl;
        }
    }
};

// Test registration helper
class TestRegistrar {
 public:
    TestRegistrar(const std::string& suite_name, const std::string& test_name, std::function<void()> test_func) {
        TestSuite::getInstance().addTest(suite_name, test_name, test_func);
    }
};

// Assertion macros
#define BOLT_ASSERT(condition)                                                                  \
    do {                                                                                        \
        if (!(condition)) {                                                                     \
            throw bolt::test::TestFailure("Assertion failed: " #condition, __FILE__, __LINE__); \
        }                                                                                       \
    } while (0)

#define BOLT_ASSERT_EQ(expected, actual)                                           \
    do {                                                                           \
        if ((expected) != (actual)) {                                              \
            std::stringstream ss;                                                  \
            ss << "Expected '" << (expected) << "', but got '" << (actual) << "'"; \
            throw bolt::test::TestFailure(ss.str(), __FILE__, __LINE__);           \
        }                                                                          \
    } while (0)

#define BOLT_ASSERT_NE(expected, actual)                                                 \
    do {                                                                                 \
        if ((expected) == (actual)) {                                                    \
            std::stringstream ss;                                                        \
            ss << "Expected values to be different, but both were '" << (actual) << "'"; \
            throw bolt::test::TestFailure(ss.str(), __FILE__, __LINE__);                 \
        }                                                                                \
    } while (0)

#define BOLT_ASSERT_TRUE(condition) BOLT_ASSERT(condition)

#define BOLT_ASSERT_FALSE(condition)                                                                        \
    do {                                                                                                    \
        if (condition) {                                                                                    \
            throw bolt::test::TestFailure("Expected false, but got true: " #condition, __FILE__, __LINE__); \
        }                                                                                                   \
    } while (0)

#define BOLT_ASSERT_NULL(ptr)                                                                             \
    do {                                                                                                  \
        if ((ptr) != nullptr) {                                                                           \
            throw bolt::test::TestFailure("Expected null pointer, but got non-null", __FILE__, __LINE__); \
        }                                                                                                 \
    } while (0)

#define BOLT_ASSERT_NOT_NULL(ptr)                                                                         \
    do {                                                                                                  \
        if ((ptr) == nullptr) {                                                                           \
            throw bolt::test::TestFailure("Expected non-null pointer, but got null", __FILE__, __LINE__); \
        }                                                                                                 \
    } while (0)

#define BOLT_ASSERT_THROWS(exception_type, code)                                                                   \
    do {                                                                                                           \
        bool threw_expected = false;                                                                               \
        try {                                                                                                      \
            code;                                                                                                  \
        } catch (const exception_type&) {                                                                          \
            threw_expected = true;                                                                                 \
        } catch (...) {                                                                                            \
            throw bolt::test::TestFailure("Expected " #exception_type " but caught different exception", __FILE__, \
                                          __LINE__);                                                               \
        }                                                                                                          \
        if (!threw_expected) {                                                                                     \
            throw bolt::test::TestFailure("Expected " #exception_type " but no exception was thrown", __FILE__,    \
                                          __LINE__);                                                               \
        }                                                                                                          \
    } while (0)

// Test case registration macro
#define BOLT_TEST(suite_name, test_name)                                                                    \
    void test_##suite_name##_##test_name();                                                                 \
    static bolt::test::TestRegistrar registrar_##suite_name##_##test_name(#suite_name, #test_name,          \
                                                                          test_##suite_name##_##test_name); \
    void test_##suite_name##_##test_name()

}  // namespace test
}  // namespace bolt

#endif  // BOLT_TEST_FRAMEWORK_HPP_
