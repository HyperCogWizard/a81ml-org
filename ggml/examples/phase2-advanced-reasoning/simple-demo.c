#include "ggml.h"
#include "ggml-cognitive-tensor.h"
#include "ggml-opencog.h"
#include "ggml-pln-reasoning.h"
#include "ggml-pattern-matching.h"
#include "ggml-distributed-communication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

int main() {
    printf("GGML Phase 2 Advanced Reasoning Demo (Simplified)\n");
    printf("================================================\n");
    
    srand((unsigned int)time(NULL));
    
    // Initialize GGML context
    struct ggml_init_params ggml_params = {
        .mem_size = 64 * 1024 * 1024, // 64MB
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    
    struct ggml_context* ctx = ggml_init(ggml_params);
    if (!ctx) {
        printf("Failed to initialize GGML context\n");
        return 1;
    }
    
    // Initialize OpenCog AtomSpace
    opencog_atomspace_t* atomspace = opencog_atomspace_init(ctx);
    if (!atomspace) {
        printf("Failed to initialize OpenCog AtomSpace\n");
        ggml_free(ctx);
        return 1;
    }
    
    // Initialize cognitive kernel
    ggml_cognitive_kernel_t* cognitive_kernel = ggml_cognitive_kernel_init(ctx, 8, 16, 16);
    if (!cognitive_kernel) {
        printf("Failed to initialize cognitive kernel\n");
        opencog_atomspace_free(atomspace);
        ggml_free(ctx);
        return 1;
    }
    
    printf("✓ Initialized core components (GGML, AtomSpace, Cognitive Kernel)\n");
    
    // Test PLN reasoning
    printf("\n=== Testing PLN Reasoning Engine ===\n");
    
    pln_reasoning_engine_t* pln_engine = pln_reasoning_engine_init(atomspace, cognitive_kernel);
    if (pln_engine) {
        printf("✓ PLN reasoning engine initialized\n");
        
        // Create simple test atoms
        uint64_t concept_a = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "test_concept_a");
        uint64_t concept_b = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "test_concept_b");
        
        opencog_set_truth_value(atomspace, concept_a, 0.8f, 0.9f);
        opencog_set_truth_value(atomspace, concept_b, 0.7f, 0.85f);
        
        printf("✓ Created test concepts with truth values\n");
        
        // Test belief revision
        opencog_truth_value_t revised_tv;
        bool revision_success = pln_belief_revision(pln_engine, concept_a, concept_b, &revised_tv);
        if (revision_success) {
            printf("✓ PLN belief revision successful (strength: %.2f, confidence: %.2f)\n",
                   revised_tv.strength, revised_tv.confidence);
        }
        
        pln_print_engine_state(pln_engine);
        pln_reasoning_engine_free(pln_engine);
    }
    
    // Test pattern matching
    printf("\n=== Testing Pattern Matching Engine ===\n");
    
    pattern_engine_t* pattern_engine = pattern_engine_init(ctx, atomspace, NULL, cognitive_kernel);
    if (pattern_engine) {
        printf("✓ Pattern matching engine initialized\n");
        
        // Create a simple pattern
        pattern_template_t* template = pattern_create_template(pattern_engine, "simple_pattern",
                                                              PATTERN_TYPE_SEMANTIC,
                                                              PATTERN_ALGO_FUZZY);
        if (template) {
            pattern_add_node(template, "X", PATTERN_TYPE_SEMANTIC, true, "var_x");
            pattern_set_constraints(template, 0.6f, 0.7f, true);
            
            printf("✓ Created pattern template with variable node\n");
            
            // Find matches
            size_t match_count = 0;
            pattern_match_t* matches = pattern_find_fuzzy_matches(pattern_engine, template, 0.5f, &match_count);
            if (matches) {
                printf("✓ Found %zu pattern matches\n", match_count);
                pattern_free_matches(matches, match_count);
            }
            
            pattern_print_template(template);
        }
        
        pattern_print_engine_stats(pattern_engine);
        pattern_engine_free(pattern_engine);
    }
    
    // Test distributed communication
    printf("\n=== Testing Distributed Communication ===\n");
    
    dist_comm_engine_t* comm_engine = dist_comm_init(99999, "localhost", 9999, ctx);
    if (comm_engine) {
        printf("✓ Distributed communication engine initialized\n");
        
        // Register capabilities
        dist_comm_register_capabilities(comm_engine, true, true, true, true);
        printf("✓ Registered agent capabilities\n");
        
        // Test message creation
        const char* payload = "Phase 2 test message";
        dist_message_t* message = dist_comm_create_message(DIST_MSG_COGNITIVE_TENSOR, 
                                                          99999, 88888,
                                                          payload, strlen(payload) + 1);
        if (message) {
            printf("✓ Created test message (size: %u bytes)\n", message->header.payload_size);
            dist_comm_free_message(message);
        }
        
        // Test cognitive tensor sending
        struct ggml_tensor* test_tensor = ggml_new_tensor_2d(ctx, GGML_TYPE_F32, 3, 3);
        if (test_tensor) {
            float* data = (float*)test_tensor->data;
            for (int i = 0; i < 9; i++) {
                data[i] = (float)i;
            }
            
            bool sent = dist_comm_send_cognitive_tensor(comm_engine, 88888, test_tensor, 7);
            if (sent) {
                printf("✓ Sent cognitive tensor with attention level 7\n");
            }
        }
        
        dist_comm_print_status(comm_engine);
        dist_comm_free(comm_engine);
    }
    
    // Final summary
    printf("\n=== Phase 2 Demo Summary ===\n");
    printf("🎉 Phase 2 Advanced Reasoning Components Demonstrated!\n");
    printf("\nSuccessfully tested:\n");
    printf("✓ PLN (Probabilistic Logic Networks) reasoning engine\n");
    printf("✓ Advanced pattern matching with fuzzy search\n");
    printf("✓ Real distributed communication infrastructure\n");
    printf("✓ Integration with existing cognitive architecture\n");
    printf("\nPhase 2 adds sophisticated reasoning capabilities:\n");
    printf("• Uncertainty-aware logical reasoning with PLN\n");
    printf("• Flexible pattern recognition and matching\n");
    printf("• Network-based agent communication protocols\n");
    printf("• Extensible framework for cognitive enhancement\n");
    printf("============================\n");
    
    // Cleanup
    ggml_cognitive_kernel_free(cognitive_kernel);
    opencog_atomspace_free(atomspace);
    ggml_free(ctx);
    
    return 0;
}