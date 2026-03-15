
#ifndef TENSOR_UTILS_HPP
#define TENSOR_UTILS_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <limits>
#include <cstring>
#include "bolt/ai/ggml.hpp"

namespace bolt {

#ifdef BOLT_HAVE_GGML

class TensorUtils {
public:
    static void serializeTensor(FILE* f, ggml_tensor* tensor) {
        fwrite(&tensor->type, sizeof(tensor->type), 1, f);
        int n_dims = ggml_n_dims(tensor);
        fwrite(&n_dims, sizeof(n_dims), 1, f);
        for (int i = 0; i < n_dims; i++) {
            int64_t ne = tensor->ne[i];
            fwrite(&ne, sizeof(ne), 1, f);
        }
        size_t size = ggml_nbytes(tensor);
        fwrite(tensor->data, size, 1, f);
    }

    static ggml_tensor* deserializeTensor(FILE* f, ggml_context* ctx) {
        enum ggml_type type;
        int n_dims;
        int64_t ne[GGML_MAX_DIMS] = {0};
        fread(&type, sizeof(type), 1, f);
        fread(&n_dims, sizeof(n_dims), 1, f);
        for (int i = 0; i < n_dims; i++) {
            fread(&ne[i], sizeof(ne[i]), 1, f);
        }
        ggml_tensor* tensor = ggml_new_tensor(ctx, type, n_dims, ne);
        size_t size = ggml_nbytes(tensor);
        fread(tensor->data, size, 1, f);
        return tensor;
    }

    static ggml_tensor* quantizeTensor(ggml_context* ctx, ggml_tensor* tensor, enum ggml_type target_type) {
        if (!tensor) return nullptr;
        if (tensor->type == target_type) return ggml_dup_tensor(ctx, tensor);

        int64_t n_elements = ggml_nelements(tensor);
        std::vector<float> src_data(n_elements);
        if (tensor->type == GGML_TYPE_F32) {
            float* data = ggml_get_data_f32(tensor);
            std::copy(data, data + n_elements, src_data.begin());
        } else if (tensor->type == GGML_TYPE_F16) {
            for (int64_t i = 0; i < n_elements; i++) {
                src_data[i] = ggml_fp16_to_fp32(((ggml_fp16_t*)tensor->data)[i]);
            }
        } else {
            return ggml_dup_tensor(ctx, tensor);
        }

        ggml_tensor* quantized = ggml_new_tensor(ctx, target_type, ggml_n_dims(tensor), tensor->ne);

        if (target_type == GGML_TYPE_F32) {
            float* dst = ggml_get_data_f32(quantized);
            std::copy(src_data.begin(), src_data.end(), dst);
        } else if (target_type == GGML_TYPE_F16) {
            ggml_fp16_t* dst = (ggml_fp16_t*)quantized->data;
            for (int64_t i = 0; i < n_elements; i++) {
                dst[i] = ggml_fp32_to_fp16(src_data[i]);
            }
        } else if (target_type == GGML_TYPE_Q4_0 || target_type == GGML_TYPE_Q4_1 ||
                   target_type == GGML_TYPE_Q8_0) {
            size_t qk = ggml_blck_size(target_type);
            size_t n_blocks = (n_elements + qk - 1) / qk;
            std::vector<float> padded_data = src_data;
            padded_data.resize(n_blocks * qk, 0.0f);
            ggml_quantize_chunk(target_type, padded_data.data(), quantized->data,
                               0, static_cast<int>(n_blocks), static_cast<int64_t>(qk), nullptr);
        } else {
            memcpy(quantized->data, tensor->data, std::min(ggml_nbytes(tensor), ggml_nbytes(quantized)));
        }
        return quantized;
    }

    static float calculateQuantizationError(ggml_tensor* original, ggml_tensor* quantized) {
        if (!original || !quantized) throw std::runtime_error("Null tensor provided");
        int64_t n_orig = ggml_nelements(original);
        int64_t n_quant = ggml_nelements(quantized);
        if (n_orig != n_quant) throw std::runtime_error("Tensor dimensions mismatch");

        std::vector<float> orig_data(n_orig), quant_data(n_quant);
        extractFloatData(original, orig_data);
        extractFloatData(quantized, quant_data);

        double mse = 0.0;
        for (int64_t i = 0; i < n_orig; i++) {
            double diff = orig_data[i] - quant_data[i];
            mse += diff * diff;
        }
        return static_cast<float>(mse / n_orig);
    }

    static float calculateRMSE(ggml_tensor* original, ggml_tensor* quantized) {
        return std::sqrt(calculateQuantizationError(original, quantized));
    }

    static float calculateMAE(ggml_tensor* original, ggml_tensor* quantized) {
        if (!original || !quantized) throw std::runtime_error("Null tensor provided");
        int64_t n = ggml_nelements(original);
        if (n != ggml_nelements(quantized)) throw std::runtime_error("Tensor dimensions mismatch");

        std::vector<float> orig_data(n), quant_data(n);
        extractFloatData(original, orig_data);
        extractFloatData(quantized, quant_data);

        double mae = 0.0;
        for (int64_t i = 0; i < n; i++) mae += std::fabs(orig_data[i] - quant_data[i]);
        return static_cast<float>(mae / n);
    }

    static float calculateMaxError(ggml_tensor* original, ggml_tensor* quantized) {
        if (!original || !quantized) throw std::runtime_error("Null tensor provided");
        int64_t n = ggml_nelements(original);
        if (n != ggml_nelements(quantized)) throw std::runtime_error("Tensor dimensions mismatch");

        std::vector<float> orig_data(n), quant_data(n);
        extractFloatData(original, orig_data);
        extractFloatData(quantized, quant_data);

        float max_err = 0.0f;
        for (int64_t i = 0; i < n; i++) {
            float err = std::fabs(orig_data[i] - quant_data[i]);
            if (err > max_err) max_err = err;
        }
        return max_err;
    }

    struct TensorStats {
        float min = 0; float max = 0; float mean = 0; float std = 0;
        int64_t n_elements = 0; int64_t n_zero = 0; float sparsity = 0;
    };

    static TensorStats getTensorStats(ggml_tensor* tensor) {
        TensorStats stats;
        if (!tensor) return stats;
        int64_t n = ggml_nelements(tensor);
        stats.n_elements = n;
        std::vector<float> data(n);
        extractFloatData(tensor, data);
        stats.min = std::numeric_limits<float>::max();
        stats.max = std::numeric_limits<float>::lowest();
        double sum = 0.0;
        stats.n_zero = 0;
        for (int64_t i = 0; i < n; i++) {
            float val = data[i];
            if (val < stats.min) stats.min = val;
            if (val > stats.max) stats.max = val;
            sum += val;
            if (val == 0.0f) stats.n_zero++;
        }
        stats.mean = static_cast<float>(sum / n);
        stats.sparsity = static_cast<float>(stats.n_zero) / static_cast<float>(n);
        double var_sum = 0.0;
        for (int64_t i = 0; i < n; i++) {
            double diff = data[i] - stats.mean;
            var_sum += diff * diff;
        }
        stats.std = std::sqrt(static_cast<float>(var_sum / n));
        return stats;
    }

private:
    static void extractFloatData(ggml_tensor* tensor, std::vector<float>& out) {
        int64_t n = ggml_nelements(tensor);
        if (tensor->type == GGML_TYPE_F32) {
            float* data = ggml_get_data_f32(tensor);
            std::copy(data, data + n, out.begin());
        } else if (tensor->type == GGML_TYPE_F16) {
            ggml_fp16_t* data = (ggml_fp16_t*)tensor->data;
            for (int64_t i = 0; i < n; i++) out[i] = ggml_fp16_to_fp32(data[i]);
        } else {
            std::fill(out.begin(), out.end(), 0.0f);
        }
    }
};

#else // !BOLT_HAVE_GGML

// Stub TensorUtils when GGML is not available
class TensorUtils {
public:
    struct TensorStats {
        float min = 0; float max = 0; float mean = 0; float std = 0;
        int64_t n_elements = 0; int64_t n_zero = 0; float sparsity = 0;
    };
};

#endif // BOLT_HAVE_GGML

} // namespace bolt

#endif // TENSOR_UTILS_HPP
