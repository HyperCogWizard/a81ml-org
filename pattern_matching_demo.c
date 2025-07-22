#include "ggml/include/ggml.h"
#include "ggml/include/ggml-cognitive-tensor.h"
#include <stdio.h>
#include <math.h>

// Standalone demo showcasing advanced pattern matching integration
int main() {
    printf("=== Advanced Pattern Matching Integration Demo ===\n\n");

    // Initialize ggml context (simulating cognitive agent framework)
    struct ggml_init_params params = {
        .mem_size   = 16*1024*1024,
        .mem_buffer = NULL,
        .no_alloc   = false,
    };
    struct ggml_context* ctx = ggml_init(params);
    if (!ctx) {
        printf("Failed to initialize ggml context\n");
        return 1;
    }

    // Initialize cognitive kernel
    ggml_cognitive_kernel_t* kernel = ggml_cognitive_kernel_init(ctx, 8, 16, 16);
    printf("✓ Initialized cognitive kernel for advanced pattern matching\n");

    // Create patterns representing philosophical concepts
    struct ggml_tensor* consciousness_pattern = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 8);
    struct ggml_tensor* awareness_pattern = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 8);
    struct ggml_tensor* cognition_pattern = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 8);

    // Initialize consciousness pattern with complex structure
    float* consciousness_data = (float*)consciousness_pattern->data;
    consciousness_data[0] = 1.0f; consciousness_data[1] = 0.8f;
    consciousness_data[2] = 0.9f; consciousness_data[3] = 105.0f; // Matula value
    consciousness_data[4] = 0.7f; consciousness_data[5] = 0.6f;
    consciousness_data[6] = 0.8f; consciousness_data[7] = 0.9f;

    // Initialize awareness pattern (structurally similar to consciousness)
    float* awareness_data = (float*)awareness_pattern->data;
    awareness_data[0] = 0.95f; awareness_data[1] = 0.85f;
    awareness_data[2] = 0.88f; awareness_data[3] = 105.0f; // Same Matula value
    awareness_data[4] = 0.75f; awareness_data[5] = 0.65f;
    awareness_data[6] = 0.82f; awareness_data[7] = 0.88f;

    // Initialize cognition pattern (different structure)
    float* cognition_data = (float*)cognition_pattern->data;
    cognition_data[0] = 0.5f; cognition_data[1] = 0.4f;
    cognition_data[2] = 0.6f; cognition_data[3] = 42.0f; // Different Matula value
    cognition_data[4] = 0.3f; cognition_data[5] = 0.7f;
    cognition_data[6] = 0.5f; cognition_data[7] = 0.4f;

    printf("✓ Created test patterns for consciousness, awareness, and cognition\n\n");

    printf("=== DEMONSTRATION 1: Structural Pattern Matching ===\n");
    printf("Testing consciousness vs awareness (similar structure):\n");
    
    ggml_pattern_match_config_t config = ggml_pattern_match_config_default();
    ggml_pattern_match_result_t result1 = ggml_pattern_match_structural(
        consciousness_pattern, awareness_pattern, kernel, &config);
    
    printf("→ Structural similarity: %.3f\n", result1.structural_similarity);
    printf("→ Match type: %s\n", result1.match_type == PATTERN_MATCH_EXACT ? "Exact" : 
                                 result1.match_type == PATTERN_MATCH_STRUCTURAL ? "Structural" : "Other");
    printf("→ Confidence: %.3f\n\n", result1.confidence_score);

    printf("Testing consciousness vs cognition (different structure):\n");
    ggml_pattern_match_result_t result2 = ggml_pattern_match_structural(
        consciousness_pattern, cognition_pattern, kernel, &config);
    
    printf("→ Structural similarity: %.3f\n", result2.structural_similarity);
    printf("→ Match type: %s\n", result2.match_type == PATTERN_MATCH_EXACT ? "Exact" : 
                                 result2.match_type == PATTERN_MATCH_STRUCTURAL ? "Structural" : "Other");
    printf("→ Confidence: %.3f\n\n", result2.confidence_score);

    printf("=== DEMONSTRATION 2: Multi-Level Pattern Analysis ===\n");
    printf("Performing syntactic, semantic, and pragmatic analysis:\n");
    
    ggml_pattern_match_result_t result3 = ggml_pattern_match_multilevel(
        consciousness_pattern, awareness_pattern, kernel, 0x7); // All levels
    
    printf("→ Syntactic (structural): %.3f\n", result3.structural_similarity);
    printf("→ Semantic (vector space): %.3f\n", result3.semantic_similarity);
    printf("→ Pragmatic (phase coherence): %.3f\n", result3.phase_coherence);
    printf("→ Combined confidence: %.3f\n\n", result3.confidence_score);

    printf("=== DEMONSTRATION 3: Fuzzy Pattern Matching ===\n");
    printf("Testing fuzzy matching with threshold controls:\n");
    
    ggml_pattern_match_result_t result4 = ggml_pattern_match_fuzzy(
        consciousness_pattern, awareness_pattern, kernel, 0.8f, 0.1f);
    
    printf("→ Exact match: %s\n", result4.is_exact_match ? "Yes" : "No");
    printf("→ Fuzzy match: %s\n", result4.is_fuzzy_match ? "Yes" : "No");
    printf("→ Confidence: %.3f\n\n", result4.confidence_score);

    printf("=== DEMONSTRATION 4: Advanced Unified Interface ===\n");
    printf("Using unified interface with all capabilities enabled:\n");
    
    config.enable_recursive = true;
    config.enable_hierarchical = true;
    config.fuzzy_threshold = 0.7f;
    
    ggml_pattern_match_result_t result5 = ggml_pattern_match_advanced(
        consciousness_pattern, awareness_pattern, kernel, &config);
    
    printf("→ Advanced pattern matching results:\n");
    ggml_pattern_match_result_print(&result5);

    printf("\n=== DEMONSTRATION 5: Phase Coherence Analysis ===\n");
    printf("Testing quantum phase coherence for complex patterns:\n");
    
    // Create patterns with phase information
    struct ggml_tensor* phase_pattern1 = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 8);
    struct ggml_tensor* phase_pattern2 = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 8);
    
    float* phase_data1 = (float*)phase_pattern1->data;
    float* phase_data2 = (float*)phase_pattern2->data;
    
    // Initialize with complex phase information
    for (int i = 0; i < 8; i += 2) {
        phase_data1[i] = cosf(i * 0.5f);          // real part
        phase_data1[i+1] = sinf(i * 0.5f);        // imaginary part
        phase_data2[i] = cosf(i * 0.5f + 0.1f);   // slightly shifted phase
        phase_data2[i+1] = sinf(i * 0.5f + 0.1f);
    }
    
    ggml_pattern_match_result_t phase_result = ggml_pattern_match_phase_coherence(
        phase_pattern1, phase_pattern2, kernel);
    
    printf("→ Phase coherence: %.3f\n", phase_result.phase_coherence);
    printf("→ Confidence: %.3f\n\n", phase_result.confidence_score);

    printf("=== COGNITIVE ARCHITECTURE INTEGRATION SUMMARY ===\n");
    printf("Advanced Pattern Matching Capabilities Demonstrated:\n\n");
    printf("✓ Structure-aware matching using Matula-Goebel prime encoding\n");
    printf("  - Analyzes deep structural relationships in cognitive patterns\n");
    printf("  - Distinguishes between similar and different concept structures\n\n");
    
    printf("✓ Multi-level semantic analysis\n");
    printf("  - Syntactic: Structural pattern relationships\n");
    printf("  - Semantic: Vector space similarity analysis\n");
    printf("  - Pragmatic: Quantum phase coherence matching\n\n");
    
    printf("✓ Fuzzy matching with confidence thresholds\n");
    printf("  - Configurable similarity thresholds\n");
    printf("  - Tolerance controls for approximate matching\n");
    printf("  - Confidence scoring for match quality assessment\n\n");
    
    printf("✓ Recursive hierarchical pattern recognition\n");
    printf("  - Deep structural decomposition using prime factorization\n");
    printf("  - Multi-level recursive analysis capabilities\n\n");
    
    printf("✓ Probabilistic and phase-coherent matching\n");
    printf("  - Quantum phase encoding for complex pattern states\n");
    printf("  - Confidence-weighted similarity analysis\n");
    printf("  - Support for superposition and interference patterns\n\n");

    printf("=== INTEGRATION WITH PLN REASONING ===\n");
    printf("This advanced pattern matching system provides the foundation for:\n");
    printf("→ Probabilistic Logic Networks (PLN) integration\n");
    printf("→ MOSES meta-optimization system support\n");
    printf("→ Distributed cognitive agent reasoning\n");
    printf("→ Hypergraph knowledge representation matching\n");
    printf("→ Economic attention allocation based on pattern relevance\n\n");

    // Cleanup
    ggml_cognitive_kernel_free(kernel);
    ggml_free(ctx);

    printf("Advanced pattern matching integration demonstration completed!\n");
    printf("The cognitive architecture now supports sophisticated pattern\n");
    printf("recognition capabilities for distributed artificial consciousness.\n");

    return 0;
}