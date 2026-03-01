# Template: Add comprehensive E2E test target to CMakeLists.txt
# Insert this block into the test/CMakeLists.txt of the target project.
# Adjust library names and include paths to match the project.

add_executable(bolt_comprehensive_e2e_tests
    test_comprehensive_e2e.cpp
    test_runner.cpp  # or the project's main() for tests
)

target_include_directories(bolt_comprehensive_e2e_tests PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(bolt_comprehensive_e2e_tests PRIVATE
    bolt_core        # adjust to actual library targets
    bolt_editor
    bolt_network
    bolt_collaboration
    bolt_drawkern
    bolt_git
    bolt_lsp
    bolt_ai
    pthread
)

# Register with CTest
add_test(NAME ComprehensiveE2ETests COMMAND bolt_comprehensive_e2e_tests)
