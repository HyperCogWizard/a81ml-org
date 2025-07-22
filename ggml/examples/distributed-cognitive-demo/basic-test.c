#include "ggml-distributed-cognitive.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

int main(void) {
    printf("Testing real distributed communication implementation...\n");
    
    // Initialize GGML context
    struct ggml_init_params params = {
        .mem_size = 64 * 1024 * 1024,  // 64MB
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    struct ggml_context* ctx = ggml_init(params);
    
    // Test distributed cognitive architecture initialization
    distributed_cognitive_architecture_t* arch = distributed_cognitive_init(ctx, "localhost:8001");
    if (!arch) {
        printf("FAILED: Could not initialize distributed cognitive architecture\n");
        return 1;
    }
    
    printf("SUCCESS: Distributed cognitive architecture initialized\n");
    printf("Agent ID: %u\n", arch->agent_id);
    printf("Endpoint: %s\n", arch->endpoint);
    printf("Real distributed mode: %s\n", arch->real_distributed_mode ? "ENABLED" : "DISABLED");
    
    // Test dashboard operations
    dashboard_update(arch);
    dashboard_print(arch);
    
    // Test basic coherence computation
    float coherence = dashboard_compute_coherence(arch);
    printf("System coherence: %.3f\n", coherence);
    
    // Test P-System membrane creation
    uint32_t membrane_id = psystem_create_membrane(arch, "test_membrane", MEMBRANE_ELEMENTARY, 0);
    printf("Created membrane ID: %u\n", membrane_id);
    
    // Test self-optimization loop
    uint32_t loop_id = optimization_create_loop(arch, "test_system", "test_param", 1.0f, 2.0f);
    printf("Created optimization loop ID: %u\n", loop_id);
    
    // Test basic functionality without RPC
    printf("Network status check:\n");
    distributed_cognitive_print_network_status(arch);
    
    // Run basic test suite
    printf("\nRunning test suite...\n");
    bool tests_passed = distributed_cognitive_run_test_suite(arch);
    printf("Tests %s\n", tests_passed ? "PASSED" : "FAILED");
    
    // Cleanup
    distributed_cognitive_free(arch);
    ggml_free(ctx);
    
    printf("\nBasic distributed communication test completed successfully!\n");
    return 0;
}