#include "ggml-cognitive-tensor.h"
#include "ggml.h"
#include <stdio.h>
#include <math.h>

// Simple test of pattern matching functions
int main() {
    printf("Testing advanced pattern matching compilation...\n");
    
    // Test configuration creation
    ggml_pattern_match_config_t config = ggml_pattern_match_config_default();
    printf("Default config created: struct_weight=%.2f\n", config.structure_weight);
    
    printf("Test completed successfully!\n");
    return 0;
}