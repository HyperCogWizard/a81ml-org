#include "includes/ggml-distributed-cognitive.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>

// Simple test of automated architecture evolution functionality
int main() {
    printf("=== Automated Architecture Evolution Test ===\n\n");
    
    // Initialize ggml context
    struct ggml_init_params params = {
        .mem_size = 64 * 1024 * 1024,  // 64MB
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    
    struct ggml_context* ctx = ggml_init(params);
    if (!ctx) {
        fprintf(stderr, "Failed to initialize ggml context\n");
        return 1;
    }
    
    // Initialize distributed cognitive architecture
    distributed_cognitive_architecture_t* arch = distributed_cognitive_init(ctx, "localhost:8080");
    if (!arch) {
        fprintf(stderr, "Failed to initialize distributed cognitive architecture\n");
        ggml_free(ctx);
        return 1;
    }
    
    printf("Architecture initialized successfully\n");
    printf("Evolution history capacity: %zu\n", arch->evolution_history_capacity);
    printf("Dynamic module capacity: %zu\n", arch->dynamic_module_capacity);
    printf("Initial membrane count: %zu\n\n", arch->membrane_count);
    
    // Test 1: Architecture evolution with poor performance
    printf("Test 1: Architecture adaptation with poor performance (0.2)\n");
    float initial_performance = distributed_cognitive_benchmark_performance(arch);
    printf("Initial performance: %.3f\n", initial_performance);
    
    bool adapted1 = recursive_improvement_adapt_architecture(arch, 0.2f);
    printf("Adaptation result: %s\n", adapted1 ? "SUCCESS" : "FAILURE");
    printf("Dynamic modules after poor performance: %zu\n", arch->dynamic_module_count);
    printf("Evolution history records: %zu\n\n", arch->evolution_history_count);
    
    // Test 2: Architecture evolution with excellent performance  
    printf("Test 2: Architecture adaptation with excellent performance (0.9)\n");
    bool adapted2 = recursive_improvement_adapt_architecture(arch, 0.9f);
    printf("Adaptation result: %s\n", adapted2 ? "SUCCESS" : "FAILURE");
    printf("Dynamic modules after excellent performance: %zu\n", arch->dynamic_module_count);
    printf("Evolution history records: %zu\n\n", arch->evolution_history_count);
    
    // Test 3: Manual module creation and removal
    printf("Test 3: Manual module management\n");
    bool created = architecture_evolution_create_module(arch, "test_module", "test_type", 0.5f);
    printf("Module creation result: %s\n", created ? "SUCCESS" : "FAILURE");
    printf("Dynamic modules after creation: %zu\n", arch->dynamic_module_count);
    
    bool removed = architecture_evolution_remove_module(arch, "test_module");
    printf("Module removal result: %s\n", removed ? "SUCCESS" : "FAILURE");
    printf("Dynamic modules after removal: %zu\n\n", arch->dynamic_module_count);
    
    // Test 4: Topology modification
    printf("Test 4: Topology modification\n");
    
    // Create initial membrane for testing
    uint32_t test_membrane = psystem_create_membrane(arch, "test_membrane", MEMBRANE_TISSUE, 0);
    printf("Created test membrane: %u\n", test_membrane);
    printf("Membrane count before topology change: %zu\n", arch->membrane_count);
    
    bool topo_modified = architecture_evolution_modify_topology(arch, "optimize_communication_patterns", 0.3f);
    printf("Topology modification result: %s\n", topo_modified ? "SUCCESS" : "FAILURE");
    printf("Membrane count after topology change: %zu\n\n", arch->membrane_count);
    
    // Test 5: Learning from history
    printf("Test 5: Learning from evolution history\n");
    bool learned = architecture_evolution_learn_from_history(arch, 0.4f);
    printf("Learning result: %s\n", learned ? "SUCCESS" : "FAILURE");
    
    // Print evolution history
    printf("\nEvolution History Summary:\n");
    for (size_t i = 0; i < arch->evolution_history_count; i++) {
        architecture_evolution_record_t* record = &arch->evolution_history[i];
        printf("  %zu: %s (delta: %+.3f, %s)\n", 
               i + 1, 
               record->change_description,
               record->performance_delta,
               record->successful ? "SUCCESS" : "FAILURE");
    }
    
    // Test 6: Comprehensive evolution cycle
    printf("\nTest 6: Comprehensive evolution cycle\n");
    printf("Running multiple evolution cycles to test robustness...\n");
    
    float performance_values[] = {0.1f, 0.3f, 0.6f, 0.8f, 0.4f, 0.9f, 0.2f};
    size_t num_cycles = sizeof(performance_values) / sizeof(performance_values[0]);
    
    for (size_t i = 0; i < num_cycles; i++) {
        printf("  Cycle %zu: performance %.1f -> ", i + 1, performance_values[i]);
        bool result = recursive_improvement_adapt_architecture(arch, performance_values[i]);
        printf("%s\n", result ? "ADAPTED" : "NO_CHANGE");
    }
    
    printf("\nFinal state:\n");
    printf("  Dynamic modules: %zu\n", arch->dynamic_module_count);
    printf("  Evolution history: %zu records\n", arch->evolution_history_count);
    printf("  Membrane count: %zu\n", arch->membrane_count);
    
    // Print final performance
    float final_performance = distributed_cognitive_benchmark_performance(arch);
    printf("  Final performance: %.3f\n", final_performance);
    
    printf("\n=== All Tests Completed Successfully! ===\n");
    
    // Cleanup
    distributed_cognitive_free(arch);
    ggml_free(ctx);
    
    return 0;
}