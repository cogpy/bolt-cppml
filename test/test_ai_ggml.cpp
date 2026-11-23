#include "bolt/ai/ggml.hpp"
#include "bolt/ai/ggml_wrapper.hpp"
#include "bolt/ai/rwkv_wrapper.hpp"
#include "bolt/test_framework.hpp"

using namespace bolt::test;

// ===== GGML Component Tests =====

#ifdef BOLT_HAVE_GGML

BOLT_TEST(GGMLTest, GGMLContextCreation) {
    const size_t mem_size = 1024 * 1024;  // 1MB

    // Test context creation
    bolt::GGMLContext context(mem_size);
    BOLT_ASSERT_TRUE(context.get() != nullptr);

    // Test basic tensor operations
    auto* tensor = ggml_new_tensor_1d(context.get(), GGML_TYPE_F32, 100);
    BOLT_ASSERT_TRUE(tensor != nullptr);
    BOLT_ASSERT_EQ(ggml_nelements(tensor), 100);
    BOLT_ASSERT_TRUE(tensor->type == GGML_TYPE_F32);
}

BOLT_TEST(GGMLTest, TensorOperations) {
    const size_t mem_size = 1024 * 1024;
    bolt::GGMLContext context(mem_size);

    // Create tensors
    auto* a = ggml_new_tensor_1d(context.get(), GGML_TYPE_F32, 10);
    auto* b = ggml_new_tensor_1d(context.get(), GGML_TYPE_F32, 10);

    BOLT_ASSERT_TRUE(a != nullptr);
    BOLT_ASSERT_TRUE(b != nullptr);

    // Test basic operations
    auto* c = ggml_add(context.get(), a, b);
    BOLT_ASSERT_TRUE(c != nullptr);

    auto* d = ggml_mul(context.get(), a, b);
    BOLT_ASSERT_TRUE(d != nullptr);
}

BOLT_TEST(GGMLTest, GraphOperations) {
    const size_t mem_size = 1024 * 1024;
    bolt::GGMLContext context(mem_size);

    // Create computation graph
    auto* gf = ggml_new_graph_custom(context.get(), GGML_DEFAULT_GRAPH_SIZE, true);
    BOLT_ASSERT_TRUE(gf != nullptr);

    // Create simple computation
    auto* x = ggml_new_tensor_1d(context.get(), GGML_TYPE_F32, 5);
    auto* y = ggml_dup(context.get(), x);

    // Build graph
    ggml_build_forward_expand(gf, y);
    BOLT_ASSERT_TRUE(gf != nullptr);
}

BOLT_TEST(GGMLTest, TensorUtilsEmptyTensor) {
    bolt::GGMLContext context(1024 * 1024);
    auto* empty = ggml_new_tensor_1d(context.get(), GGML_TYPE_F32, 0);
    auto* alsoEmpty = ggml_new_tensor_1d(context.get(), GGML_TYPE_F32, 0);
    BOLT_ASSERT_TRUE(empty != nullptr && alsoEmpty != nullptr);
    BOLT_ASSERT_EQ(0, ggml_nelements(empty));

    // Each of these divided 0 by 0 and returned NaN.
    BOLT_ASSERT_EQ(0.0f, bolt::TensorUtils::calculateQuantizationError(empty, alsoEmpty));
    BOLT_ASSERT_EQ(0.0f, bolt::TensorUtils::calculateRMSE(empty, alsoEmpty));
    BOLT_ASSERT_EQ(0.0f, bolt::TensorUtils::calculateMAE(empty, alsoEmpty));

    auto stats = bolt::TensorUtils::getTensorStats(empty);
    BOLT_ASSERT_EQ(0, stats.n_elements);
    BOLT_ASSERT_EQ(0, stats.n_zero);
    BOLT_ASSERT_EQ(0.0f, stats.min);
    BOLT_ASSERT_EQ(0.0f, stats.max);
    BOLT_ASSERT_EQ(0.0f, stats.mean);
    BOLT_ASSERT_EQ(0.0f, stats.std);
    BOLT_ASSERT_EQ(0.0f, stats.sparsity);
}

BOLT_TEST(GGMLTest, GGMLWrapperBasics) {
    auto& wrapper = bolt::GGMLWrapper::getInstance();
    (void)wrapper;
}

#else  // !BOLT_HAVE_GGML

BOLT_TEST(GGMLTest, GGMLNotAvailable) {
    // When GGML is not available, GGMLContext should throw
    BOLT_ASSERT_THROWS(std::runtime_error, { bolt::GGMLContext context(1024); });
}

BOLT_TEST(GGMLTest, GGMLWrapperStub) {
    auto& wrapper = bolt::GGMLWrapper::getInstance();
    // Wrapper should exist but throw on initialize
    BOLT_ASSERT_THROWS(bolt::GGMLException, { wrapper.initialize("nonexistent.gguf"); });
}

#endif  // BOLT_HAVE_GGML

// RWKV wrapper tests work regardless of GGML availability
BOLT_TEST(GGMLTest, RWKVWrapperBasics) {
    auto& wrapper = bolt::RWKVWrapper::getInstance();

    // Test that wrapper starts uninitialized
    BOLT_ASSERT_FALSE(wrapper.isInitialized());

    // Test basic API accessibility
    BOLT_ASSERT_EQ(wrapper.getNumLayers(), 2);
    BOLT_ASSERT_EQ(wrapper.getEmbedDim(), 64);
}
