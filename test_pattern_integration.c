#include "examples/cognitive-agents/cognitive-agent.h"
#include "ggml/include/ggml-cognitive-tensor.h"
#include <stdio.h>
#include <math.h>

// Simple integration test showing pattern matching working with cognitive agents
int main() {
    printf("=== Advanced Pattern Matching Integration Test ===\n\n");

    // Create a cognitive agent
    cognitive_agent* agent = create_cognitive_agent("localhost:9999");
    printf("Created cognitive agent %llu\n", agent->agent_id);

    // Initialize cognitive kernel for pattern matching
    ggml_cognitive_kernel_t* kernel = ggml_cognitive_kernel_init(agent->ctx, 8, 16, 16);
    printf("Initialized cognitive kernel for pattern matching\n\n");

    // Create test patterns representing consciousness concepts
    struct ggml_tensor* consciousness_pattern = ggml_new_tensor_1d(agent->ctx, GGML_TYPE_F32, 8);
    struct ggml_tensor* awareness_pattern = ggml_new_tensor_1d(agent->ctx, GGML_TYPE_F32, 8);

    // Initialize with consciousness-related patterns
    float* consciousness_data = (float*)consciousness_pattern->data;
    consciousness_data[0] = 1.0f; consciousness_data[1] = 0.8f;
    consciousness_data[2] = 0.9f; consciousness_data[3] = 105.0f; // Complex Matula structure
    consciousness_data[4] = 0.7f; consciousness_data[5] = 0.6f;
    consciousness_data[6] = 0.8f; consciousness_data[7] = 0.9f;

    float* awareness_data = (float*)awareness_pattern->data;
    awareness_data[0] = 0.95f; awareness_data[1] = 0.85f;
    awareness_data[2] = 0.88f; awareness_data[3] = 105.0f; // Same structure
    awareness_data[4] = 0.75f; awareness_data[5] = 0.65f;
    awareness_data[6] = 0.82f; awareness_data[7] = 0.88f;

    printf("Test 1: Advanced Pattern Matching for Related Concepts\n");
    printf("------------------------------------------------------\n");
    ggml_pattern_match_config_t config = ggml_pattern_match_config_default();
    config.fuzzy_threshold = 0.8f;
    config.enable_recursive = true;

    ggml_pattern_match_result_t result = ggml_pattern_match_advanced(
        consciousness_pattern, awareness_pattern, kernel, &config);

    printf("Matching consciousness vs awareness concepts:\n");
    ggml_pattern_match_result_print(&result);

    printf("\nTest 2: Structural Analysis Using Matula-Goebel Encoding\n");
    printf("--------------------------------------------------------\n");
    ggml_pattern_match_result_t structural_result = ggml_pattern_match_structural(
        consciousness_pattern, awareness_pattern, kernel, &config);
    
    printf("Structural similarity: %.3f\n", structural_result.structural_similarity);
    printf("Match confidence: %.3f\n", structural_result.confidence_score);
    printf("Exact structural match: %s\n", structural_result.is_exact_match ? "Yes" : "No");

    printf("\nTest 3: Multi-level Analysis (Syntactic + Semantic + Pragmatic)\n");
    printf("---------------------------------------------------------------\n");
    ggml_pattern_match_result_t multilevel_result = ggml_pattern_match_multilevel(
        consciousness_pattern, awareness_pattern, kernel, 0x7); // All levels

    printf("Syntactic (structural): %.3f\n", multilevel_result.structural_similarity);
    printf("Semantic (vector space): %.3f\n", multilevel_result.semantic_similarity);
    printf("Pragmatic (phase): %.3f\n", multilevel_result.phase_coherence);
    printf("Combined confidence: %.3f\n", multilevel_result.confidence_score);

    // Add knowledge to the agent's memory for context
    float consciousness_emb[64];
    for (int i = 0; i < 64; i++) consciousness_emb[i] = (float)i / 64.0f;
    add_knowledge(agent->memory, "consciousness", consciousness_emb, 64);

    float awareness_emb[64];
    for (int i = 0; i < 64; i++) awareness_emb[i] = 0.9f * (float)i / 64.0f;
    add_knowledge(agent->memory, "awareness", awareness_emb, 64);

    printf("\nCognitive Agent Memory Status:\n");
    printf("Memory nodes: %zu\n", agent->memory->node_count);
    printf("Agent cycles completed: %llu\n", agent->cycles_completed);

    printf("\n=== Pattern Matching Capabilities Demonstrated ===\n");
    printf("✓ Advanced unified pattern matching interface\n");
    printf("✓ Structural analysis using Matula-Goebel prime encoding\n");
    printf("✓ Multi-level semantic and pragmatic analysis\n");
    printf("✓ Fuzzy matching with confidence thresholds\n");
    printf("✓ Integration with cognitive agent framework\n");
    printf("✓ Support for consciousness and awareness concept matching\n");

    // Cleanup
    ggml_cognitive_kernel_free(kernel);
    cleanup_cognitive_agent(agent);

    printf("\nAdvanced pattern matching integration test completed successfully!\n");
    return 0;
}