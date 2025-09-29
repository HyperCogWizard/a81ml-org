#include "ggml-distributed-cognitive.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void test_meta_meta_reasoning_creation() {
    printf("Testing meta-meta-reasoning loop creation...\n");
    
    // Initialize GGML context
    struct ggml_init_params params = {
        .mem_size = 64 * 1024 * 1024,  // 64MB
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    
    struct ggml_context* ctx = ggml_init(params);
    assert(ctx != NULL);
    
    // Initialize distributed cognitive architecture
    distributed_cognitive_architecture_t* arch = distributed_cognitive_init(ctx, "localhost");
    assert(arch != NULL);
    assert(arch->meta_meta_reasoning_active == true);
    
    // Test meta-meta-reasoning loop creation
    uint32_t meta_meta_id = meta_meta_reasoning_create_loop(
        arch, 
        "Test meta-meta-reasoning loop"
    );
    
    assert(meta_meta_id > 0);
    assert(arch->meta_meta_reasoning_count == 1);
    
    // Verify loop initialization
    meta_meta_reasoning_loop_t* loop = &arch->meta_meta_reasoning_loops[0];
    assert(loop->meta_meta_id == meta_meta_id);
    assert(strcmp(loop->description, "Test meta-meta-reasoning loop") == 0);
    assert(loop->active == true);
    assert(loop->self_reflective == true);
    assert(loop->pattern_learning_active == true);
    assert(loop->pattern_count == 0);
    assert(loop->pattern_capacity == 16);
    
    // Test pattern addition
    bool pattern_added = meta_meta_reasoning_add_pattern(
        arch, meta_meta_id,
        META_META_REASONING_DEDUCTION,
        "Test deduction pattern",
        0.8f
    );
    
    assert(pattern_added == true);
    assert(loop->pattern_count == 1);
    
    meta_reasoning_pattern_t* pattern = &loop->patterns[0];
    assert(pattern->reasoning_type == META_META_REASONING_DEDUCTION);
    assert(strcmp(pattern->reasoning_pattern, "Test deduction pattern") == 0);
    assert(pattern->pattern_strength == 0.8f);
    
    printf("✅ Meta-meta-reasoning creation test passed\n");
    
    // Cleanup
    distributed_cognitive_free(arch);
    ggml_free(ctx);
}

void test_meta_meta_reasoning_cycles() {
    printf("Testing meta-meta-reasoning cycles...\n");
    
    // Initialize GGML context
    struct ggml_init_params params = {
        .mem_size = 64 * 1024 * 1024,  // 64MB
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    
    struct ggml_context* ctx = ggml_init(params);  
    assert(ctx != NULL);
    
    // Initialize distributed cognitive architecture
    distributed_cognitive_architecture_t* arch = distributed_cognitive_init(ctx, "localhost");
    assert(arch != NULL);
    
    // Create meta-meta-reasoning loop
    uint32_t meta_meta_id = meta_meta_reasoning_create_loop(
        arch,
        "Test reasoning cycle"
    );
    assert(meta_meta_id > 0);
    
    // Add some patterns
    meta_meta_reasoning_add_pattern(
        arch, meta_meta_id,
        META_META_REASONING_REFLECTION,
        "Self-reflection pattern",
        0.7f
    );
    
    meta_meta_reasoning_add_pattern(
        arch, meta_meta_id,
        META_META_REASONING_ANALOGY,
        "Analogical reasoning pattern",
        0.6f
    );
    
    meta_meta_reasoning_loop_t* loop = &arch->meta_meta_reasoning_loops[0];
    
    // Test initial coherence measurement
    float initial_coherence = meta_meta_reasoning_measure_coherence(arch, meta_meta_id);
    assert(initial_coherence >= 0.0f && initial_coherence <= 1.0f);
    
    // Test self-reflection
    bool reflection_result = meta_meta_reasoning_self_reflect(arch, meta_meta_id);
    printf("Self-reflection result: %s\n", reflection_result ? "SUCCESS" : "NO-CHANGE");
    
    // Test pattern evolution
    bool evolution_result = meta_meta_reasoning_evolve_patterns(arch, meta_meta_id);
    printf("Pattern evolution result: %s\n", evolution_result ? "SUCCESS" : "NO-CHANGE");
    
    // Test full cycle
    bool cycle_result = meta_meta_reasoning_run_cycle(arch, meta_meta_id);
    printf("Full cycle result: %s\n", cycle_result ? "IMPROVEMENT" : "NO-IMPROVEMENT");
    
    // Verify that cycle completed
    assert(loop->meta_cycles_completed > 0);
    
    // Test coherence after cycle
    float final_coherence = meta_meta_reasoning_measure_coherence(arch, meta_meta_id);
    assert(final_coherence >= 0.0f && final_coherence <= 1.0f);
    
    printf("Initial coherence: %.3f, Final coherence: %.3f\n", 
           initial_coherence, final_coherence);
    
    printf("✅ Meta-meta-reasoning cycles test passed\n");
    
    // Cleanup
    distributed_cognitive_free(arch);
    ggml_free(ctx);
}

void test_meta_meta_reasoning_integration() {
    printf("Testing meta-meta-reasoning integration...\n");
    
    // Initialize GGML context
    struct ggml_init_params params = {
        .mem_size = 64 * 1024 * 1024,  // 64MB
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    
    struct ggml_context* ctx = ggml_init(params);
    assert(ctx != NULL);
    
    // Initialize distributed cognitive architecture
    distributed_cognitive_architecture_t* arch = distributed_cognitive_init(ctx, "localhost");
    assert(arch != NULL);
    assert(arch->meta_meta_reasoning_active == true);
    assert(arch->recursive_improvement_active == true);
    
    // Create a recursive improvement loop for integration
    uint32_t recursive_id = recursive_improvement_create_loop(
        arch,
        RECURSIVE_TARGET_REASONING_PATTERNS,
        "Integration test recursive loop"  
    );
    assert(recursive_id > 0);
    
    // Create meta-meta-reasoning loop
    uint32_t meta_meta_id = meta_meta_reasoning_create_loop(
        arch,
        "Integration test meta-meta loop"
    );
    assert(meta_meta_id > 0);
    
    // Add patterns
    meta_meta_reasoning_add_pattern(
        arch, meta_meta_id,
        META_META_REASONING_INDUCTION,
        "Integration reasoning pattern",
        0.75f
    );
    
    // Test both systems working together
    bool recursive_result = recursive_improvement_run_cycle(arch, recursive_id);
    bool meta_meta_result = meta_meta_reasoning_run_cycle(arch, meta_meta_id);
    
    printf("Recursive improvement: %s\n", recursive_result ? "SUCCESS" : "NO-CHANGE");
    printf("Meta-meta reasoning: %s\n", meta_meta_result ? "SUCCESS" : "NO-CHANGE");
    
    // Verify both systems are active
    assert(arch->recursive_improvement_count > 0);
    assert(arch->meta_meta_reasoning_count > 0);
    
    // Test system coherence
    float system_coherence = dashboard_compute_coherence(arch);
    printf("System coherence: %.3f\n", system_coherence);
    assert(system_coherence >= 0.0f && system_coherence <= 1.0f);
    
    printf("✅ Meta-meta-reasoning integration test passed\n");
    
    // Cleanup
    distributed_cognitive_free(arch);
    ggml_free(ctx);
}

int main() {
    printf("Meta-Meta-Reasoning Unit Tests\n");
    printf("==============================\n\n");
    
    test_meta_meta_reasoning_creation();
    printf("\n");
    
    test_meta_meta_reasoning_cycles();
    printf("\n");
    
    test_meta_meta_reasoning_integration();
    printf("\n");
    
    printf("🎉 All meta-meta-reasoning tests passed!\n");
    return 0;
}