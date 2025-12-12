#include "bolt/ai/vector_database.hpp"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <stdexcept>

namespace bolt {
namespace ai {

void VectorDatabase::addOrUpdateRecord(const VectorRecord& record) {
    std::lock_guard<std::mutex> lock(mutex_);
    records_[record.id] = record;
}

bool VectorDatabase::removeRecord(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    return records_.erase(id) > 0;
}

bool VectorDatabase::hasRecord(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return records_.find(id) != records_.end();
}

VectorRecord VectorDatabase::getRecord(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = records_.find(id);
    if (it != records_.end()) {
        return it->second;
    }
    return VectorRecord();
}

float VectorDatabase::cosineSimilarity(
    const std::vector<float>& a, 
    const std::vector<float>& b) {
    
    if (a.size() != b.size() || a.empty()) {
        return 0.0f;
    }
    
    float dot_product = 0.0f;
    float norm_a = 0.0f;
    float norm_b = 0.0f;
    
    for (size_t i = 0; i < a.size(); ++i) {
        dot_product += a[i] * b[i];
        norm_a += a[i] * a[i];
        norm_b += b[i] * b[i];
    }
    
    if (norm_a == 0.0f || norm_b == 0.0f) {
        return 0.0f;
    }
    
    return dot_product / (std::sqrt(norm_a) * std::sqrt(norm_b));
}

float VectorDatabase::euclideanDistance(
    const std::vector<float>& a, 
    const std::vector<float>& b) {
    
    if (a.size() != b.size() || a.empty()) {
        return std::numeric_limits<float>::max();
    }
    
    float sum = 0.0f;
    for (size_t i = 0; i < a.size(); ++i) {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    
    return std::sqrt(sum);
}

std::vector<SearchResult> VectorDatabase::searchSimilar(
    const std::vector<float>& query_embedding,
    int top_k,
    float min_similarity) const {
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<SearchResult> results;
    results.reserve(records_.size());
    
    // Calculate similarity for each record
    for (const auto& pair : records_) {
        const VectorRecord& record = pair.second;
        
        float similarity = cosineSimilarity(query_embedding, record.embedding);
        
        if (similarity >= min_similarity) {
            results.emplace_back(
                record.id,
                similarity,
                record.content_preview,
                record.file_type
            );
        }
    }
    
    // Sort by similarity (descending)
    std::sort(results.begin(), results.end(), 
        [](const SearchResult& a, const SearchResult& b) {
            return a.similarity_score > b.similarity_score;
        });
    
    // Return top K results
    if (results.size() > static_cast<size_t>(top_k)) {
        results.resize(top_k);
    }
    
    return results;
}

size_t VectorDatabase::getRecordCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return records_.size();
}

std::vector<std::string> VectorDatabase::getAllIds() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<std::string> ids;
    ids.reserve(records_.size());
    
    for (const auto& pair : records_) {
        ids.push_back(pair.first);
    }
    
    return ids;
}

void VectorDatabase::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    records_.clear();
}

bool VectorDatabase::saveToFile(const std::string& file_path) const {
    // TODO: Implement binary serialization for persistence
    // This is a placeholder for future implementation
    return false;
}

bool VectorDatabase::loadFromFile(const std::string& file_path) {
    // TODO: Implement binary deserialization
    // This is a placeholder for future implementation
    return false;
}

} // namespace ai
} // namespace bolt
