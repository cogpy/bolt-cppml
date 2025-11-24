#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace bolt {

/**
 * @brief Memory allocation information with extended metadata
 */
struct AllocationInfo {
    void* pointer;
    size_t size;
    std::string file;
    int line;
    std::string function;
    std::chrono::steady_clock::time_point timestamp;
    std::string category;
    bool tracked;
    
    AllocationInfo(void* ptr = nullptr, size_t sz = 0, 
                   const std::string& f = "", int l = 0, 
                   const std::string& func = "", const std::string& cat = "general")
        : pointer(ptr), size(sz), file(f), line(l), function(func)
        , timestamp(std::chrono::steady_clock::now())
        , category(cat), tracked(true) {}
};

/**
 * @brief Memory leak detection statistics
 */
struct LeakDetectionStats {
    size_t totalLeaks = 0;
    size_t totalLeakedBytes = 0;
    size_t peakMemoryUsage = 0;
    size_t currentMemoryUsage = 0;
    std::map<std::string, size_t> leaksByCategory;
    std::map<std::string, size_t> leaksByFile;
    std::chrono::steady_clock::time_point detectionTime;
    
    LeakDetectionStats() : detectionTime(std::chrono::steady_clock::now()) {}
};

/**
 * @brief Enhanced Memory Leak Detector
 * 
 * Provides comprehensive memory leak detection with:
 * - Allocation tracking with source location
 * - Leak categorization and reporting
 * - Statistical analysis
 * - Integration with existing MemoryManager
 */
class MemoryLeakDetector {
public:
    static MemoryLeakDetector& getInstance() {
        static MemoryLeakDetector instance;
        return instance;
    }
    
    /**
     * @brief Enable or disable leak detection
     */
    void setEnabled(bool enabled) {
        enabled_ = enabled;
    }
    
    bool isEnabled() const {
        return enabled_;
    }
    
    /**
     * @brief Track a memory allocation with source location
     */
    void trackAllocation(void* ptr, size_t size, const std::string& file, 
                        int line, const std::string& function, 
                        const std::string& category = "general") {
        if (!enabled_ || !ptr) return;
        
        AllocationInfo info(ptr, size, file, line, function, category);
        allocations_[ptr] = info;
        
        currentMemoryUsage_ += size;
        if (currentMemoryUsage_ > peakMemoryUsage_) {
            peakMemoryUsage_ = currentMemoryUsage_;
        }
        
        categoryStats_[category] += size;
    }
    
    /**
     * @brief Untrack a memory deallocation
     */
    void untrackAllocation(void* ptr) {
        if (!enabled_ || !ptr) return;
        
        auto it = allocations_.find(ptr);
        if (it != allocations_.end()) {
            currentMemoryUsage_ -= it->second.size;
            categoryStats_[it->second.category] -= it->second.size;
            allocations_.erase(it);
        }
    }
    
    /**
     * @brief Check if there are any memory leaks
     */
    bool hasLeaks() const {
        return !allocations_.empty();
    }
    
    /**
     * @brief Get the number of leaked allocations
     */
    size_t getLeakCount() const {
        return allocations_.size();
    }
    
    /**
     * @brief Get total bytes leaked
     */
    size_t getLeakedBytes() const {
        size_t total = 0;
        for (const auto& pair : allocations_) {
            total += pair.second.size;
        }
        return total;
    }
    
    /**
     * @brief Get all leaked allocations
     */
    std::vector<AllocationInfo> getLeaks() const {
        std::vector<AllocationInfo> leaks;
        for (const auto& pair : allocations_) {
            leaks.push_back(pair.second);
        }
        return leaks;
    }
    
    /**
     * @brief Get leak detection statistics
     */
    LeakDetectionStats getStats() const {
        LeakDetectionStats stats;
        stats.totalLeaks = getLeakCount();
        stats.totalLeakedBytes = getLeakedBytes();
        stats.peakMemoryUsage = peakMemoryUsage_;
        stats.currentMemoryUsage = currentMemoryUsage_;
        
        // Group leaks by category
        for (const auto& pair : allocations_) {
            const auto& info = pair.second;
            stats.leaksByCategory[info.category] += info.size;
            stats.leaksByFile[info.file] += info.size;
        }
        
        return stats;
    }
    
    /**
     * @brief Generate a detailed leak report
     */
    std::string generateReport() const {
        std::ostringstream report;
        
        report << "=== Memory Leak Detection Report ===\n\n";
        
        if (!hasLeaks()) {
            report << "✓ No memory leaks detected!\n";
            report << "Peak memory usage: " << formatBytes(peakMemoryUsage_) << "\n";
            return report.str();
        }
        
        auto stats = getStats();
        
        report << "⚠ Memory leaks detected!\n\n";
        report << "Summary:\n";
        report << "  Total leaks: " << stats.totalLeaks << "\n";
        report << "  Total leaked: " << formatBytes(stats.totalLeakedBytes) << "\n";
        report << "  Peak usage: " << formatBytes(stats.peakMemoryUsage) << "\n";
        report << "  Current usage: " << formatBytes(stats.currentMemoryUsage) << "\n\n";
        
        // Leaks by category
        if (!stats.leaksByCategory.empty()) {
            report << "Leaks by category:\n";
            for (const auto& pair : stats.leaksByCategory) {
                report << "  " << pair.first << ": " 
                       << formatBytes(pair.second) << "\n";
            }
            report << "\n";
        }
        
        // Leaks by file
        if (!stats.leaksByFile.empty()) {
            report << "Leaks by file:\n";
            for (const auto& pair : stats.leaksByFile) {
                report << "  " << pair.first << ": " 
                       << formatBytes(pair.second) << "\n";
            }
            report << "\n";
        }
        
        // Detailed leak information
        report << "Detailed leak information:\n";
        auto leaks = getLeaks();
        for (size_t i = 0; i < leaks.size(); ++i) {
            const auto& leak = leaks[i];
            report << "\nLeak #" << (i + 1) << ":\n";
            report << "  Address: " << leak.pointer << "\n";
            report << "  Size: " << formatBytes(leak.size) << "\n";
            report << "  Location: " << leak.file << ":" << leak.line << "\n";
            report << "  Function: " << leak.function << "\n";
            report << "  Category: " << leak.category << "\n";
            
            // Calculate allocation age
            auto age = std::chrono::steady_clock::now() - leak.timestamp;
            auto seconds = std::chrono::duration_cast<std::chrono::seconds>(age).count();
            report << "  Age: " << seconds << " seconds\n";
        }
        
        return report.str();
    }
    
    /**
     * @brief Generate a summary report
     */
    std::string generateSummary() const {
        std::ostringstream summary;
        
        if (!hasLeaks()) {
            summary << "No memory leaks detected.";
        } else {
            auto stats = getStats();
            summary << stats.totalLeaks << " leaks, "
                   << formatBytes(stats.totalLeakedBytes) << " leaked";
        }
        
        return summary.str();
    }
    
    /**
     * @brief Clear all tracked allocations (use with caution!)
     */
    void clear() {
        allocations_.clear();
        currentMemoryUsage_ = 0;
        categoryStats_.clear();
    }
    
    /**
     * @brief Reset statistics but keep tracking
     */
    void resetStats() {
        peakMemoryUsage_ = currentMemoryUsage_;
    }
    
private:
    MemoryLeakDetector() = default;
    ~MemoryLeakDetector() = default;
    MemoryLeakDetector(const MemoryLeakDetector&) = delete;
    MemoryLeakDetector& operator=(const MemoryLeakDetector&) = delete;
    
    std::string formatBytes(size_t bytes) const {
        std::ostringstream oss;
        
        if (bytes < 1024) {
            oss << bytes << " B";
        } else if (bytes < 1024 * 1024) {
            oss << std::fixed << std::setprecision(2) 
                << (bytes / 1024.0) << " KB";
        } else if (bytes < 1024 * 1024 * 1024) {
            oss << std::fixed << std::setprecision(2) 
                << (bytes / (1024.0 * 1024.0)) << " MB";
        } else {
            oss << std::fixed << std::setprecision(2) 
                << (bytes / (1024.0 * 1024.0 * 1024.0)) << " GB";
        }
        
        return oss.str();
    }
    
    bool enabled_ = true;
    std::map<void*, AllocationInfo> allocations_;
    size_t currentMemoryUsage_ = 0;
    size_t peakMemoryUsage_ = 0;
    std::map<std::string, size_t> categoryStats_;
};

// Convenience macros for leak detection
#define TRACK_ALLOCATION(ptr, size, category) \
    bolt::MemoryLeakDetector::getInstance().trackAllocation( \
        ptr, size, __FILE__, __LINE__, __FUNCTION__, category)

#define UNTRACK_ALLOCATION(ptr) \
    bolt::MemoryLeakDetector::getInstance().untrackAllocation(ptr)

} // namespace bolt
