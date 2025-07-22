#include "ggml.h"
#include "ggml-moses.h"
#include "ggml-opencog.h"
#include "ggml-cognitive-tensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// Simple demo function: f(x1, x2) = x1^2 + x2^2 - symbolic regression target
float target_function(float x1, float x2) {
    return x1 * x1 + x2 * x2;
}

// Demo: MOSES symbolic regression
void demo_moses_symbolic_regression(void) {
    printf("\n=== MOSES Symbolic Regression Demo ===\n");
    
    // Initialize GGML context
    struct ggml_init_params params = {
        .mem_size   = 1024 * 1024 * 16, // 16 MB
        .mem_buffer = NULL,
        .no_alloc   = false,
    };
    struct ggml_context* ctx = ggml_init(params);
    if (!ctx) {
        printf("Failed to initialize GGML context\n");
        return;
    }
    
    // Initialize cognitive components (with smaller sizes to avoid performance issues)
    opencog_atomspace_t* atomspace = opencog_atomspace_init(ctx);
    ggml_cognitive_kernel_t* kernel = ggml_cognitive_kernel_init(ctx, 4, 4, 4);
    
    // Initialize MOSES system
    moses_system_t* moses = moses_system_init(ctx, atomspace, kernel);
    if (!moses) {
        printf("Failed to initialize MOSES system\n");
        ggml_free(ctx);
        return;
    }
    
    // Configure MOSES parameters (smaller for demo)
    moses_config_t config = {
        .population_size = 10,
        .max_generations = 5,
        .target_fitness = 0.8f,
        .fitness_tolerance = 0.001f,
        
        .mutation_probability = 0.1f,
        .crossover_probability = 0.7f,
        .reproduction_probability = 0.2f,
        
        .tournament_size = 3,
        .elitism_fraction = 0.1f,
        
        .max_program_depth = 6,
        .max_program_nodes = 32,
        .complexity_penalty_weight = 0.01f,
        
        .diversity_weight = 0.05f,
        .novelty_threshold = 0.8f,
        
        .enable_meta_optimization = false,
        .meta_generations = 5,
        .meta_learning_rate = 0.01f,
        
        .use_atomspace_seeding = true,
        .use_cognitive_tensor_fitness = true,
        .enable_prime_structure_bias = true
    };
    
    moses_system_configure(moses, &config);
    
    // Create training dataset
    moses_dataset_t* dataset = moses_dataset_create(100, 2);
    if (!dataset) {
        printf("Failed to create dataset\n");
        moses_system_free(moses);
        ggml_free(ctx);
        return;
    }
    
    // Generate training data for f(x1, x2) = x1^2 + x2^2 (fewer cases for demo)
    printf("Generating training data...\n");
    srand((unsigned int)time(NULL));
    
    for (int i = 0; i < 20; i++) {
        float x1 = ((float)rand() / RAND_MAX) * 4.0f - 2.0f; // Range [-2, 2]
        float x2 = ((float)rand() / RAND_MAX) * 4.0f - 2.0f; // Range [-2, 2]
        float y = target_function(x1, x2);
        
        float inputs[2] = {x1, x2};
        moses_dataset_add_case(dataset, inputs, y, 1.0f);
    }
    
    printf("Dataset created with %zu training cases\n", dataset->case_count);
    printf("Target function: f(x1, x2) = x1^2 + x2^2\n");
    
    // Add some initial knowledge to AtomSpace
    uint64_t x1_atom = opencog_add_node(atomspace, OPENCOG_VARIABLE_NODE, "x1");
    uint64_t x2_atom = opencog_add_node(atomspace, OPENCOG_VARIABLE_NODE, "x2");
    uint64_t square_atom = opencog_add_node(atomspace, OPENCOG_PREDICATE_NODE, "square");
    uint64_t add_atom = opencog_add_node(atomspace, OPENCOG_PREDICATE_NODE, "add");
    
    printf("Added initial knowledge to AtomSpace:\n");
    printf("  Variables: x1 (ID %lu), x2 (ID %lu)\n", x1_atom, x2_atom);
    printf("  Operations: square (ID %lu), add (ID %lu)\n", square_atom, add_atom);
    
    // Run MOSES evolution
    printf("\nStarting MOSES evolution...\n");
    moses_program_t* best_program = moses_run_evolution(moses, dataset);
    
    if (best_program) {
        printf("\n=== Evolution Results ===\n");
        moses_program_print(best_program);
        
        // Add best program to AtomSpace
        uint64_t program_atom = moses_program_to_atomspace(moses, best_program);
        
        // Test the best program on some examples
        printf("\n=== Testing Best Program ===\n");
        float test_cases[][2] = {
            {1.0f, 1.0f},
            {2.0f, 0.0f},
            {0.0f, 2.0f},
            {-1.0f, 1.0f},
            {1.5f, -0.5f}
        };
        
        for (int i = 0; i < 5; i++) {
            float x1 = test_cases[i][0];
            float x2 = test_cases[i][1];
            float expected = target_function(x1, x2);
            float predicted = moses_program_evaluate(moses, best_program, test_cases[i]);
            float error = fabsf(predicted - expected);
            
            printf("  Input: (%.1f, %.1f) -> Expected: %.2f, Predicted: %.2f, Error: %.4f\n",
                   x1, x2, expected, predicted, error);
        }
        
        moses_program_free(best_program);
    } else {
        printf("No successful program evolved\n");
    }
    
    // Print system statistics
    moses_system_print_statistics(moses);
    
    // Print AtomSpace statistics
    opencog_print_atomspace_statistics(atomspace);
    
    // Cleanup
    moses_dataset_free(dataset);
    moses_system_free(moses);
    opencog_atomspace_free(atomspace);
    ggml_cognitive_kernel_free(kernel);
    ggml_free(ctx);
    
    printf("\nMOSES symbolic regression demo completed.\n");
}

// Demo: MOSES integration with cognitive tensors
void demo_moses_cognitive_integration(void) {
    printf("\n=== MOSES Cognitive Integration Demo ===\n");
    
    // Initialize components
    struct ggml_init_params params = {
        .mem_size   = 1024 * 1024 * 8,
        .mem_buffer = NULL,
        .no_alloc   = false,
    };
    struct ggml_context* ctx = ggml_init(params);
    if (!ctx) return;
    
    ggml_cognitive_kernel_t* kernel = ggml_cognitive_kernel_init(ctx, 5, 4, 4);
    opencog_atomspace_t* atomspace = opencog_atomspace_init(ctx);
    moses_system_t* moses = moses_system_init(ctx, atomspace, kernel);
    
    if (!moses) {
        ggml_free(ctx);
        return;
    }
    
    // Create a simple program and show tensor encoding
    moses_program_t* program = moses_program_create_random(moses, 3);
    if (program) {
        printf("Created random program (ID %u)\n", program->program_id);
        
        // Convert to tensor representation
        struct ggml_tensor* program_tensor = moses_program_to_tensor(moses, program);
        if (program_tensor) {
            printf("Program tensor encoding created\n");
            float* data = (float*)program_tensor->data;
            printf("  Tensor data: [");
            for (int i = 0; i < 8; i++) {
                printf("%.2f", data[i]);
                if (i < 7) printf(", ");
            }
            printf("]\n");
        }
        
        // Add to AtomSpace
        uint64_t atom_id = moses_program_to_atomspace(moses, program);
        if (atom_id > 0) {
            printf("Program added to AtomSpace with ID %lu\n", atom_id);
            
            // Show AtomSpace representation
            opencog_print_atom(atomspace, atom_id);
        }
        
        // Show Matula encoding
        printf("Matula encoding:\n");
        printf("  Matula value: %u\n", program->matula_encoding.matula_value);
        printf("  System level: %u\n", program->matula_encoding.system_level);
        printf("  Breadth index: %u\n", program->matula_encoding.breadth_index);
        printf("  Depth index: %u\n", program->matula_encoding.depth_index);
        
        moses_program_free(program);
    }
    
    // Cleanup
    moses_system_free(moses);
    opencog_atomspace_free(atomspace);
    ggml_cognitive_kernel_free(kernel);
    ggml_free(ctx);
    
    printf("Cognitive integration demo completed.\n");
}

int main(void) {
    printf("MOSES (Meta-Optimizing Semantic Evolution) System Demo\n");
    printf("=====================================================\n");
    
    // Run symbolic regression demo
    demo_moses_symbolic_regression();
    
    // Run cognitive integration demo
    demo_moses_cognitive_integration();
    
    printf("\nAll demos completed successfully!\n");
    return 0;
}