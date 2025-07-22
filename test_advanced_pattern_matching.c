#include "ggml/include/ggml-cognitive-tensor.h"
#include "ggml/include/ggml.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// Test basic advanced pattern matching functionality
void test_advanced_pattern_matching() {
    printf("Advanced Pattern Matching Test Suite\n");
    printf("=====================================\n\n");

    // Initialize ggml context
    struct ggml_init_params params = {
        .mem_size   = 16*1024*1024,
        .mem_buffer = NULL,
        .no_alloc   = false,
    };
    struct ggml_context* ctx = ggml_init(params);
    if (!ctx) {
        printf("Failed to initialize ggml context\n");
        return;
    }

    // Initialize cognitive kernel
    ggml_cognitive_kernel_t* kernel = ggml_cognitive_kernel_init(ctx, 8, 16, 16);
    if (!kernel) {
        printf("Failed to initialize cognitive kernel\n");
        ggml_free(ctx);
        return;
    }

    // Create test patterns
    struct ggml_tensor* pattern1 = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 4);
    struct ggml_tensor* pattern2 = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 4);
    struct ggml_tensor* pattern3 = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 4);

    // Initialize test data
    float* data1 = (float*)pattern1->data;
    float* data2 = (float*)pattern2->data;
    float* data3 = (float*)pattern3->data;

    // Pattern 1: [1.0, 2.0, 3.0, 30.0] (Matula value 30)
    data1[0] = 1.0f; data1[1] = 2.0f; data1[2] = 3.0f; data1[3] = 30.0f;
    
    // Pattern 2: [1.0, 2.0, 3.0, 30.0] (identical)
    data2[0] = 1.0f; data2[1] = 2.0f; data2[2] = 3.0f; data2[3] = 30.0f;
    
    // Pattern 3: [1.1, 2.1, 3.1, 42.0] (similar but different Matula)
    data3[0] = 1.1f; data3[1] = 2.1f; data3[2] = 3.1f; data3[3] = 42.0f;

    printf("Test 1: Exact Pattern Match\n");
    printf("----------------------------\n");
    ggml_pattern_match_config_t config = ggml_pattern_match_config_default();
    ggml_pattern_match_result_t result1 = ggml_pattern_match_advanced(
        pattern1, pattern2, kernel, &config);
    ggml_pattern_match_result_print(&result1);
    printf("Expected: Exact match with high confidence\n\n");

    printf("Test 2: Fuzzy Pattern Match\n");
    printf("---------------------------\n");
    config.fuzzy_threshold = 0.8f;
    ggml_pattern_match_result_t result2 = ggml_pattern_match_advanced(
        pattern1, pattern3, kernel, &config);
    ggml_pattern_match_result_print(&result2);
    printf("Expected: Fuzzy match with moderate confidence\n\n");

    printf("Test 3: Structural Pattern Match\n");
    printf("--------------------------------\n");
    ggml_pattern_match_result_t result3 = ggml_pattern_match_structural(
        pattern1, pattern3, kernel, &config);
    ggml_pattern_match_result_print(&result3);
    printf("Expected: Structural analysis based on Matula-Goebel encoding\n\n");

    printf("Test 4: Phase Coherence Match\n");
    printf("-----------------------------\n");
    // Create complex-valued patterns for phase testing
    struct ggml_tensor* phase_pattern1 = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 8);
    struct ggml_tensor* phase_pattern2 = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 8);
    
    float* phase_data1 = (float*)phase_pattern1->data;
    float* phase_data2 = (float*)phase_pattern2->data;
    
    // Initialize with complex values [real, imag, real, imag, ...]
    for (int i = 0; i < 8; i += 2) {
        phase_data1[i] = cosf(i * 0.5f);     // real part
        phase_data1[i+1] = sinf(i * 0.5f);   // imaginary part
        phase_data2[i] = cosf(i * 0.5f + 0.1f);     // slightly shifted phase
        phase_data2[i+1] = sinf(i * 0.5f + 0.1f);
    }
    
    ggml_pattern_match_result_t result4 = ggml_pattern_match_phase_coherence(
        phase_pattern1, phase_pattern2, kernel);
    ggml_pattern_match_result_print(&result4);
    printf("Expected: High phase coherence with slight phase shift\n\n");

    printf("Test 5: Multi-level Pattern Match\n");
    printf("---------------------------------\n");
    ggml_pattern_match_result_t result5 = ggml_pattern_match_multilevel(
        pattern1, pattern3, kernel, 0x7); // All levels
    ggml_pattern_match_result_print(&result5);
    printf("Expected: Combined syntactic, semantic, and pragmatic analysis\n\n");

    printf("Test 6: Recursive Pattern Match\n");
    printf("-------------------------------\n");
    config.enable_recursive = true;
    config.max_recursion_depth = 3;
    ggml_pattern_match_result_t result6 = ggml_pattern_match_recursive(
        pattern1, pattern3, kernel, 0, config.max_recursion_depth);
    ggml_pattern_match_result_print(&result6);
    printf("Expected: Hierarchical pattern analysis\n\n");

    printf("Test 7: Probabilistic Pattern Match\n");
    printf("-----------------------------------\n");
    float confidence_map[4] = {0.9f, 0.8f, 0.7f, 0.6f};
    ggml_pattern_match_result_t result7 = ggml_pattern_match_probabilistic(
        pattern1, pattern3, kernel, confidence_map, 4);
    ggml_pattern_match_result_print(&result7);
    printf("Expected: Confidence-weighted similarity analysis\n\n");

    // Cleanup
    ggml_cognitive_kernel_free(kernel);
    ggml_free(ctx);

    printf("Advanced Pattern Matching Tests Complete!\n");
}

int main() {
    test_advanced_pattern_matching();
    return 0;
}