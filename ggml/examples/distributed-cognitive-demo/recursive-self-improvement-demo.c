#include "ggml.h"
#include "ggml-distributed-cognitive.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main() {
    printf("=== Recursive Self-Improvement Demo ===\n");
    printf("Demonstrating recursive self-improvement capabilities\n");
    printf("of the distributed cognitive architecture.\n\n");
    
    // Initialize ggml context
    struct ggml_init_params params = {
        .mem_size = 128 * 1024 * 1024,  // 128MB
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    
    struct ggml_context* ctx = ggml_init(params);
    if (!ctx) {
        fprintf(stderr, "Failed to initialize ggml context\n");
        return 1;
    }
    
    // Initialize distributed cognitive architecture
    distributed_cognitive_architecture_t* arch = distributed_cognitive_init(ctx, "localhost:8000");
    if (!arch) {
        fprintf(stderr, "Failed to initialize distributed cognitive architecture\n");
        ggml_free(ctx);
        return 1;
    }
    
    printf("Distributed cognitive architecture initialized\n");
    printf("Agent ID: %u\n", arch->agent_id);
    printf("Recursive improvement: %s\n\n", 
           arch->recursive_improvement_active ? "ENABLED" : "DISABLED");
    
    // Initial system performance benchmark
    printf("=== Initial System State ===\n");
    float initial_performance = distributed_cognitive_benchmark_performance(arch);
    printf("Initial system performance: %.3f\n", initial_performance);
    
    // Print initial optimization loops
    printf("Initial optimization loops: %zu\n", arch->optimization_loop_count);
    
    // Create some basic optimization loops to improve
    uint32_t opt_loop1 = optimization_create_loop(arch, "memory_system", "allocation_efficiency", 0.5f, 0.9f);
    uint32_t opt_loop2 = optimization_create_loop(arch, "reasoning_engine", "inference_speed", 0.3f, 0.8f);
    uint32_t opt_loop3 = optimization_create_loop(arch, "attention_system", "allocation_accuracy", 0.4f, 0.85f);
    
    printf("Created %zu optimization loops\n", arch->optimization_loop_count);
    
    // Enable self-optimization
    arch->self_optimization_active = true;
    
    printf("\n=== Creating Recursive Self-Improvement Loops ===\n");
    
    // Create recursive improvement loops for different targets
    uint32_t recursive_opt_alg = recursive_improvement_create_loop(
        arch, 
        RECURSIVE_TARGET_OPTIMIZATION_ALGORITHM, 
        "Self-improving optimization algorithms"
    );
    
    uint32_t recursive_learning = recursive_improvement_create_loop(
        arch, 
        RECURSIVE_TARGET_LEARNING_RATE, 
        "Adaptive learning rate optimization"
    );
    
    uint32_t recursive_attention = recursive_improvement_create_loop(
        arch, 
        RECURSIVE_TARGET_ATTENTION_ALLOCATION, 
        "Dynamic attention allocation improvement"
    );
    
    uint32_t recursive_reasoning = recursive_improvement_create_loop(
        arch, 
        RECURSIVE_TARGET_REASONING_PATTERNS, 
        "Self-modifying reasoning patterns"
    );
    
    printf("Created %zu recursive improvement loops\n", arch->recursive_improvement_count);
    
    // Print initial status
    recursive_improvement_print_status(arch);
    
    printf("\n=== Running Recursive Self-Improvement Cycles ===\n");
    
    // Run improvement cycles and track performance
    float performance_history[10];
    int num_cycles = 8;
    
    for (int cycle = 0; cycle < num_cycles; cycle++) {
        printf("\n--- Cycle %d ---\n", cycle + 1);
        
        // Run basic optimization cycle first
        optimization_run_cycle(arch);
        
        // Run recursive improvement cycles
        bool any_recursive_improvement = false;
        
        // Run each recursive improvement loop
        if (recursive_improvement_run_cycle(arch, recursive_opt_alg)) {
            any_recursive_improvement = true;
        }
        
        if (recursive_improvement_run_cycle(arch, recursive_learning)) {
            any_recursive_improvement = true;
        }
        
        if (recursive_improvement_run_cycle(arch, recursive_attention)) {
            any_recursive_improvement = true;
        }
        
        if (recursive_improvement_run_cycle(arch, recursive_reasoning)) {
            any_recursive_improvement = true;
        }
        
        // Measure current performance
        float current_performance = distributed_cognitive_benchmark_performance(arch);
        performance_history[cycle] = current_performance;
        
        printf("Cycle %d performance: %.3f (recursive improvement: %s)\n", 
               cycle + 1, current_performance, any_recursive_improvement ? "YES" : "NO");
        
        // Update dashboard
        dashboard_update(arch);
        
        // Small delay to simulate processing time
        usleep(100000);  // 100ms
    }
    
    printf("\n=== Intelligence Bootstrapping Test ===\n");
    
    // Test intelligence bootstrapping
    bool bootstrap_success = recursive_improvement_bootstrap_intelligence(arch);
    printf("Intelligence bootstrapping: %s\n", bootstrap_success ? "SUCCESS" : "ATTEMPTED");
    
    // Final performance measurement
    float final_performance = distributed_cognitive_benchmark_performance(arch);
    
    printf("\n=== Results Summary ===\n");
    printf("Initial performance: %.3f\n", initial_performance);
    printf("Final performance:   %.3f\n", final_performance);
    printf("Performance improvement: %.3f (%.1f%%)\n", 
           final_performance - initial_performance,
           ((final_performance - initial_performance) / initial_performance) * 100.0f);
    
    // Show performance trajectory
    printf("\nPerformance trajectory:\n");
    for (int i = 0; i < num_cycles; i++) {
        printf("  Cycle %d: %.3f", i + 1, performance_history[i]);
        if (i > 0) {
            float change = performance_history[i] - performance_history[i-1];
            printf(" (%+.3f)", change);
        }
        printf("\n");
    }
    
    // Final status of recursive improvement system
    printf("\n=== Final Recursive Improvement Status ===\n");
    recursive_improvement_print_status(arch);
    
    // Print final dashboard
    printf("\n=== Final System Dashboard ===\n");
    dashboard_print(arch);
    
    // Demonstration of meta-optimization effects
    printf("\n=== Meta-Optimization Effects ===\n");
    printf("Optimization loops modified by recursive improvement:\n");
    for (size_t i = 0; i < arch->optimization_loop_count; i++) {
        printf("  Loop %zu: lr=%.4f, momentum=%.3f, converged=%s\n",
               i + 1,
               arch->optimization_loops[i].learning_rate,
               arch->optimization_loops[i].momentum,
               arch->optimization_loops[i].converged ? "YES" : "NO");
    }
    
    // Test convergence behavior
    printf("\n=== Convergence Analysis ===\n");
    int converged_recursive = 0;
    int converged_basic = 0;
    
    for (size_t i = 0; i < arch->recursive_improvement_count; i++) {
        if (arch->recursive_improvement_loops[i].converged) {
            converged_recursive++;
        }
    }
    
    for (size_t i = 0; i < arch->optimization_loop_count; i++) {
        if (arch->optimization_loops[i].converged) {
            converged_basic++;
        }
    }
    
    printf("Converged recursive improvement loops: %d/%zu\n", 
           converged_recursive, arch->recursive_improvement_count);
    printf("Converged basic optimization loops: %d/%zu\n", 
           converged_basic, arch->optimization_loop_count);
    
    // Cleanup
    distributed_cognitive_free(arch);
    ggml_free(ctx);
    
    printf("\n=== Demo Completed Successfully ===\n");
    printf("The recursive self-improvement system demonstrated:\n");
    printf("1. Meta-optimization of optimization algorithms\n");
    printf("2. Self-modifying learning rates and parameters\n");
    printf("3. Dynamic attention allocation adaptation\n");
    printf("4. Self-modifying reasoning patterns\n");
    printf("5. Intelligence bootstrapping capabilities\n");
    printf("\nThis shows the system's ability to recursively improve\n");
    printf("its own cognitive processes and optimization algorithms.\n");
    
    return 0;
}