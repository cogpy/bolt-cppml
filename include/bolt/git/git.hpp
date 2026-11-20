#ifndef BOLT_GIT_GIT_HPP_
#define BOLT_GIT_GIT_HPP_

/**
 * Git integration module for Bolt C++ IDE
 *
 * This module provides comprehensive Git integration including:
 * - Repository status monitoring
 * - Basic Git operations (add, commit, push, pull)
 * - Branch management
 * - File status tracking
 * - Integration with editor components
 */

#include <string>

#include "bolt/git/git_integration.hpp"
#include "bolt/git/git_repository.hpp"

namespace bolt {
namespace git {

/**
 * Initialize Git integration for the current project
 * @param projectPath Path to the project directory
 * @return true if Git integration was successfully initialized
 */
inline bool initializeGit(const std::string& projectPath) {
    return GitIntegration::getInstance().initialize(projectPath);
}

/**
 * Get the Git integration manager instance
 */
inline GitIntegration& getGitIntegration() {
    return GitIntegration::getInstance();
}

/**
 * Check if Git integration is active
 */
inline bool isGitActive() {
    return GitIntegration::getInstance().isActive();
}

/**
 * Quick access to common Git operations
 */
namespace quick {

inline bool stage(const std::string& filePath) {
    return GitIntegration::getInstance().stageFile(filePath);
}

inline bool stageAll() {
    return GitIntegration::getInstance().stageAllFiles();
}

inline bool commit(const std::string& message) {
    return GitIntegration::getInstance().commitChanges(message);
}

inline bool push() {
    return GitIntegration::getInstance().pushChanges();
}

inline bool pull() {
    return GitIntegration::getInstance().pullChanges();
}

inline std::optional<GitFileStatus> status(const std::string& filePath) {
    return GitIntegration::getInstance().getFileStatus(filePath);
}

}  // namespace quick

}  // namespace git
}  // namespace bolt

#endif  // BOLT_GIT_GIT_HPP_
