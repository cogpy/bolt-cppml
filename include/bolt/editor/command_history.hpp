
#ifndef BOLT_EDITOR_COMMAND_HISTORY_HPP_
#define BOLT_EDITOR_COMMAND_HISTORY_HPP_

#include <deque>
#include <string>

#include "bolt/core/thread_safety.hpp"

namespace bolt {

class CommandHistory {
 private:
    ThreadSafe<std::deque<std::string>> history_;
    ThreadSafe<size_t> maxSize_{100};
    ThreadSafe<size_t> currentIndex_{0};

 public:
    static CommandHistory& getInstance() {
        static CommandHistory instance;
        return instance;
    }

    void addCommand(const std::string& command) {
        history_.write([&](auto& hist) {
            if (hist.size() >= maxSize_.read()) {
                hist.pop_front();
            }
            hist.push_back(command);
            currentIndex_.write([&](auto& idx) {
                idx = hist.size();
                return idx;
            });
        });
    }

    std::string getPreviousCommand() {
        return stepHistory([](size_t& current, size_t) {
            if (current > 0)
                current--;
        });
    }

    std::string getNextCommand() {
        return stepHistory([](size_t& current, size_t historySize) {
            if (current < historySize)
                current++;
        });
    }

 private:
    // Moves currentIndex_ with step(index, history size) and returns the command at the new index, or an empty
    // string if there is none. Takes the history_ read lock first, then the currentIndex_ write lock.
    template <typename Step>
    std::string stepHistory(Step&& step) {
        return history_.read([&](const auto& hist) {
            if (hist.empty())
                return std::string();

            size_t idx = currentIndex_.write([&](auto& current) {
                step(current, hist.size());
                return current;
            });

            return idx < hist.size() ? hist[idx] : std::string();
        });
    }
};

}  // namespace bolt

#endif  // BOLT_EDITOR_COMMAND_HISTORY_HPP_
