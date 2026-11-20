
#ifndef BOLT_UTILS_DATA_PROCESSOR_HPP_
#define BOLT_UTILS_DATA_PROCESSOR_HPP_

#include <algorithm>
#include <numeric>
#include <stdexcept>
#include <vector>

template <typename T>
class DataProcessor {
 public:
    static T calculateAverage(const std::vector<T>& data) {
        if (data.empty()) {
            throw std::runtime_error("Cannot calculate average of empty dataset");
        }
        return std::accumulate(data.begin(), data.end(), T{}) / static_cast<T>(data.size());
    }

    static T findMax(const std::vector<T>& data) {
        if (data.empty()) {
            throw std::runtime_error("Cannot find maximum in empty dataset");
        }
        return *std::max_element(data.begin(), data.end());
    }

    static T findMin(const std::vector<T>& data) {
        if (data.empty()) {
            throw std::runtime_error("Cannot find minimum in empty dataset");
        }
        return *std::min_element(data.begin(), data.end());
    }

    static std::vector<T> sort(std::vector<T> data) {
        std::sort(data.begin(), data.end());
        return data;
    }
};

#endif  // BOLT_UTILS_DATA_PROCESSOR_HPP_
