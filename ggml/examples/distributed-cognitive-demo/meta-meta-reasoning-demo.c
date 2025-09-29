#include "ggml-distributed-cognitive.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

void demo_meta_meta_reasoning_patterns(distributed_cognitive_architecture_t* arch) {
    printf("\n=== Meta-Meta-Reasoning Patterns Demo ===\n");
    
    // Create a meta-meta-reasoning loop for self-reflective pattern analysis
    uint32_t meta_meta_id = meta_meta_reasoning_create_loop(
        arch, 
        "Self-reflective reasoning pattern optimizer"
    );
    
    if (meta_meta_id == 0) {
        printf("Failed to create meta-meta-reasoning loop\n");
        return;
    }
    
    // Add various reasoning patterns to analyze
    meta_meta_reasoning_add_pattern(
        arch, meta_meta_id, 
        META_META_REASONING_DEDUCTION, 
        "If A implies B and A is true, then B is true",
        0.9f
    );
    
    meta_meta_reasoning_add_pattern(
        arch, meta_meta_id,
        META_META_REASONING_INDUCTION,
        "From specific observations, infer general patterns", 
        0.7f
    );
    
    meta_meta_reasoning_add_pattern(
        arch, meta_meta_id,
        META_META_REASONING_ABDUCTION,
        "Given effect, hypothesize most likely cause",
        0.6f
    );
    
    meta_meta_reasoning_add_pattern(
        arch, meta_meta_id,
        META_META_REASONING_ANALOGY,
        "Map structure from familiar domain to unfamiliar domain",
        0.8f
    );
    
    meta_meta_reasoning_add_pattern(
        arch, meta_meta_id,
        META_META_REASONING_REFLECTION,
        "Analyze own reasoning processes for optimization",
        0.75f
    );
    
    // Run several meta-meta-reasoning cycles
    printf("\n--- Running Meta-Meta-Reasoning Cycles ---\n");
    
    for (int cycle = 0; cycle < 5; cycle++) {
        printf("\n--- Cycle %d ---\n", cycle + 1);
        
        bool improvement = meta_meta_reasoning_run_cycle(arch, meta_meta_id);
        float coherence = meta_meta_reasoning_measure_coherence(arch, meta_meta_id);
        
        printf("Cycle %d: Improvement: %s, Coherence: %.3f\n",
               cycle + 1, improvement ? "YES" : "NO", coherence);
        
        // Simulate some time passing
        usleep(50000);  // 50ms
    }
    
    printf("\n--- Final Meta-Meta-Reasoning Status ---\n");
    meta_meta_reasoning_print_status(arch);
}

void demo_meta_meta_reasoning_self_reflection(distributed_cognitive_architecture_t* arch) {
    printf("\n=== Meta-Meta-Reasoning Self-Reflection Demo ===\n");
    
    // Create a meta-meta-reasoning loop focused on self-awareness
    uint32_t reflection_id = meta_meta_reasoning_create_loop(
        arch,
        "Deep self-awareness and reasoning optimization"
    );
    
    if (reflection_id == 0) {
        printf("Failed to create reflection loop\n");
        return;
    }
    
    // Add self-reflective reasoning patterns
    meta_meta_reasoning_add_pattern(
        arch, reflection_id,
        META_META_REASONING_REFLECTION,
        "Monitor effectiveness of current reasoning strategies",
        0.85f
    );
    
    meta_meta_reasoning_add_pattern(
        arch, reflection_id,
        META_META_REASONING_REFLECTION,
        "Evaluate coherence between different reasoning modes",
        0.8f
    );
    
    meta_meta_reasoning_add_pattern(
        arch, reflection_id,
        META_META_REASONING_REFLECTION,
        "Assess temporal consistency of reasoning patterns",
        0.75f
    );
    
    printf("\n--- Running Self-Reflection Cycles ---\n");
    
    // Track performance over time
    float coherence_history[8];
    
    for (int cycle = 0; cycle < 8; cycle++) {
        printf("\n--- Self-Reflection Cycle %d ---\n", cycle + 1);
        
        // Focus on self-reflection specifically
        bool self_reflected = meta_meta_reasoning_self_reflect(arch, reflection_id);
        bool patterns_evolved = meta_meta_reasoning_evolve_patterns(arch, reflection_id);
        
        float coherence = meta_meta_reasoning_measure_coherence(arch, reflection_id);
        coherence_history[cycle] = coherence;
        
        printf("Self-reflection: %s, Pattern evolution: %s, Coherence: %.3f\n",
               self_reflected ? "SUCCESS" : "NO-CHANGE",
               patterns_evolved ? "SUCCESS" : "NO-CHANGE",
               coherence);
        
        // Show coherence trend
        if (cycle > 0) {
            float trend = coherence - coherence_history[cycle-1];
            printf("Coherence trend: %+.3f %s\n", 
                   trend, trend > 0 ? "↑" : (trend < 0 ? "↓" : "→"));
        }
        
        usleep(75000);  // 75ms
    }
    
    // Analyze coherence progression
    printf("\n--- Coherence Analysis ---\n");
    float initial_coherence = coherence_history[0];
    float final_coherence = coherence_history[7];
    float improvement = final_coherence - initial_coherence;
    
    printf("Initial coherence: %.3f\n", initial_coherence);
    printf("Final coherence: %.3f\n", final_coherence);
    printf("Overall improvement: %+.3f (%.1f%%)\n", 
           improvement, improvement * 100.0f);
    
    if (improvement > 0.1f) {
        printf("✅ Significant coherence improvement achieved!\n");
    } else if (improvement > 0.05f) {
        printf("✓ Moderate coherence improvement achieved.\n");
    } else {
        printf("→ Stable coherence maintained.\n");
    }
}

void demo_meta_meta_reasoning_integration(distributed_cognitive_architecture_t* arch) {
    printf("\n=== Meta-Meta-Reasoning Integration Demo ===\n");
    
    // Create meta-meta-reasoning loop for integration with other systems
    uint32_t integration_id = meta_meta_reasoning_create_loop(
        arch,
        "Integration reasoning between meta-cognitive systems"
    );
    
    if (integration_id == 0) {
        printf("Failed to create integration loop\n");
        return;
    }
    
    // Add patterns for integration reasoning
    meta_meta_reasoning_add_pattern(
        arch, integration_id,
        META_META_REASONING_ANALOGY,
        "Map reasoning patterns between different cognitive subsystems",
        0.7f
    );
    
    meta_meta_reasoning_add_pattern(
        arch, integration_id,
        META_META_REASONING_INDUCTION,
        "Generalize optimization strategies across recursive improvement loops", 
        0.75f
    );
    
    meta_meta_reasoning_add_pattern(
        arch, integration_id,
        META_META_REASONING_DEDUCTION,
        "Derive optimal meta-reasoning strategies from performance data",
        0.8f
    );
    
    printf("Running integrated meta-meta-reasoning cycles...\n");
    
    // Run combined cycles with recursive improvement
    for (int cycle = 0; cycle < 6; cycle++) {
        printf("\n--- Integration Cycle %d ---\n", cycle + 1);
        
        // Run meta-meta-reasoning
        bool meta_improvement = meta_meta_reasoning_run_cycle(arch, integration_id);
        
        // Run recursive improvement (if active)
        bool recursive_improvement = false;
        if (arch->recursive_improvement_active && arch->recursive_improvement_count > 0) {
            recursive_improvement = recursive_improvement_run_cycle(arch, 1);  // Use first recursive loop
        }
        
        // Measure coherence and system performance
        float meta_coherence = meta_meta_reasoning_measure_coherence(arch, integration_id);
        float system_coherence = dashboard_compute_coherence(arch);
        
        printf("Meta-improvement: %s, Recursive: %s\n",
               meta_improvement ? "YES" : "NO",
               recursive_improvement ? "YES" : "NO");
        printf("Meta-coherence: %.3f, System coherence: %.3f\n",
               meta_coherence, system_coherence);
        
        // Show synergy between systems
        if (meta_improvement && recursive_improvement) {
            printf("🔗 Synergistic improvement detected!\n");
        }
        
        usleep(100000);  // 100ms
    }
    
    printf("\n--- Integration Results ---\n");
    meta_meta_reasoning_print_status(arch);
}

int main() {
    printf("Meta-Meta-Reasoning Capabilities Demo\n");
    printf("=====================================\n");
    
    // Initialize GGML context
    struct ggml_init_params params = {
        .mem_size = 128 * 1024 * 1024,  // 128MB
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    
    struct ggml_context* ctx = ggml_init(params);
    if (!ctx) {
        printf("Failed to initialize GGML context\n");
        return 1;
    }
    
    // Initialize distributed cognitive architecture
    distributed_cognitive_architecture_t* arch = distributed_cognitive_init(ctx, "localhost");
    if (!arch) {
        printf("Failed to initialize distributed cognitive architecture\n");
        ggml_free(ctx);
        return 1;
    }
    
    // Ensure meta-meta-reasoning is active
    arch->meta_meta_reasoning_active = true;
    
    // Create a basic recursive improvement loop for integration testing
    if (arch->recursive_improvement_active) {
        uint32_t recursive_id = recursive_improvement_create_loop(
            arch,
            RECURSIVE_TARGET_REASONING_PATTERNS,
            "Basic reasoning pattern optimization"
        );
        printf("Created recursive improvement loop %u for integration testing\n", recursive_id);
    }
    
    // Print initial system status
    printf("\n=== Initial System Status ===\n");
    printf("Meta-meta-reasoning active: %s\n", arch->meta_meta_reasoning_active ? "YES" : "NO");
    printf("Recursive improvement active: %s\n", arch->recursive_improvement_active ? "YES" : "NO");
    printf("Meta-meta loops capacity: %zu\n", arch->meta_meta_reasoning_capacity);
    
    // Run demos
    demo_meta_meta_reasoning_patterns(arch);
    demo_meta_meta_reasoning_self_reflection(arch);
    demo_meta_meta_reasoning_integration(arch);
    
    // Final system status
    printf("\n=== Final System Status ===\n");
    meta_meta_reasoning_print_status(arch);
    
    if (arch->recursive_improvement_active) {
        recursive_improvement_print_status(arch);
    }
    
    // Performance summary
    float final_coherence = dashboard_compute_coherence(arch);
    printf("\n=== Performance Summary ===\n");
    printf("Final system coherence: %.3f\n", final_coherence);
    printf("Meta-meta-reasoning loops created: %zu\n", arch->meta_meta_reasoning_count);
    
    if (final_coherence > 0.8f) {
        printf("🎉 Excellent meta-meta-reasoning performance!\n");
    } else if (final_coherence > 0.7f) {
        printf("✅ Good meta-meta-reasoning performance.\n");
    } else if (final_coherence > 0.5f) {
        printf("✓ Adequate meta-meta-reasoning performance.\n");
    } else {
        printf("⚠️  Meta-meta-reasoning needs improvement.\n");
    }
    
    // Cleanup
    distributed_cognitive_free(arch);
    ggml_free(ctx);
    
    printf("\nMeta-Meta-Reasoning Demo completed successfully!\n");
    return 0;
}