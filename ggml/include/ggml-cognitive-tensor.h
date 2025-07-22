#pragma once

//
// Neural-Symbolic Tensor Architecture: Matula-Goebel Prime Offset
//
// This header implements the corrected cognitive kernel tensor architecture
// incorporating the fundamental prime offset principle, wherein single-skin
// parenthetical expressions map to prime indices with unit displacement.
//

#include "ggml.h"
#include <stdint.h>
#include <stdbool.h>
#include <complex.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum system levels supported
#define GGML_COGNITIVE_MAX_SYSTEMS 16
#define GGML_COGNITIVE_MAX_BREADTH 32
#define GGML_COGNITIVE_MAX_DEPTH 32
#define GGML_COGNITIVE_MAX_PRIMES 1024

// Cognitive tensor data types
typedef float _Complex ggml_complex_t;

// Prime offset encoding structure
typedef struct {
    uint32_t primes[GGML_COGNITIVE_MAX_PRIMES];
    size_t prime_count;
    bool initialized;
} ggml_prime_lookup_t;

// Matula-Goebel encoding structure
typedef struct {
    uint32_t matula_value;
    uint32_t system_level;
    uint32_t breadth_index;
    uint32_t depth_index;
    ggml_complex_t phase;
} ggml_matula_encoding_t;

// Cognitive kernel tensor structure
typedef struct {
    // Primary 4-mode tensor [System × Breadth × Depth × Phase]
    struct ggml_tensor* cognitive_kernel;     // Complex128 encoding
    
    // Auxiliary hypergraph structures
    struct ggml_tensor* prime_lookup;         // [max_prime_idx] → prime values
    struct ggml_tensor* matula_embedding;     // [max_matula] → tensor coordinates
    struct ggml_tensor* factorization_graph;  // Sparse tensor for factor decomposition
    
    // Quantum phase encoding matrices
    struct ggml_tensor* phase_interference;   // [B×D] → phase patterns
    struct ggml_tensor* superposition_states; // Multiple tree states in superposition
    
    // Metadata
    uint32_t max_systems;
    uint32_t max_breadth;
    uint32_t max_depth;
    ggml_prime_lookup_t prime_cache;
} ggml_cognitive_kernel_t;

// Tree tensor structure for quantum superposition
typedef struct {
    uint32_t matula_value;
    ggml_complex_t phase;
    bool is_prime;
    bool has_single_skin;
    float probability_amplitude;
} ggml_tree_tensor_t;

// Prime-structured attention head configuration
typedef struct {
    uint32_t prime_dims[8];  // {2,3,5,7,11,13,17,19}
    struct ggml_tensor* attention_weights;
    struct ggml_tensor* prime_projections;
} ggml_prime_attention_t;

// Core functions

// Initialize cognitive kernel tensor
GGML_API ggml_cognitive_kernel_t* ggml_cognitive_kernel_init(
    struct ggml_context* ctx,
    uint32_t max_systems,
    uint32_t max_breadth,
    uint32_t max_depth);

// Cleanup cognitive kernel tensor
GGML_API void ggml_cognitive_kernel_free(ggml_cognitive_kernel_t* kernel);

// Prime offset functions
GGML_API void ggml_init_prime_lookup(ggml_prime_lookup_t* lookup);
GGML_API uint32_t ggml_prime_offset(uint32_t n);  // p(n) = (n+1)-th prime
GGML_API uint32_t ggml_nth_prime(uint32_t n);
GGML_API bool ggml_is_prime(uint32_t n);

// Matula-Goebel encoding functions
GGML_API ggml_matula_encoding_t ggml_encode_tree(
    const char* tree_expression,
    ggml_prime_lookup_t* prime_cache);

GGML_API uint32_t ggml_matula_decode_system_level(uint32_t matula_value);
GGML_API void ggml_matula_factorize(
    uint32_t matula_value,
    uint32_t* factors,
    size_t* factor_count);

// Cognitive kernel tensor operations
GGML_API struct ggml_tensor* ggml_cognitive_kernel_encode(
    struct ggml_context* ctx,
    ggml_cognitive_kernel_t* kernel,
    const char* tree_expression);

GGML_API struct ggml_tensor* ggml_cognitive_kernel_superposition(
    struct ggml_context* ctx,
    ggml_cognitive_kernel_t* kernel,
    ggml_tree_tensor_t* tree_states,
    size_t state_count);

// Prime-structured attention
GGML_API struct ggml_tensor* ggml_prime_attention(
    struct ggml_context* ctx,
    struct ggml_tensor* query,    // [n_tokens, d_model]
    struct ggml_tensor* key,      // [n_tokens, d_model]
    struct ggml_tensor* value,    // [n_tokens, d_model]
    ggml_prime_attention_t* prime_config);

// Quantum phase encoding
GGML_API ggml_complex_t ggml_quantum_phase_encode(
    uint32_t matula_value,
    float phase_parameter);

GGML_API struct ggml_tensor* ggml_phase_interference_pattern(
    struct ggml_context* ctx,
    ggml_cognitive_kernel_t* kernel,
    uint32_t breadth,
    uint32_t depth);

// Hypergraph composition operations
GGML_API ggml_tree_tensor_t ggml_tree_tensor_compose(
    ggml_tree_tensor_t t1,
    ggml_tree_tensor_t t2);

GGML_API struct ggml_tensor* ggml_hypergraph_compose(
    struct ggml_context* ctx,
    struct ggml_tensor** tensors,
    size_t tensor_count);

// Utility functions
GGML_API float ggml_cognitive_tensor_norm(
    struct ggml_tensor* tensor,
    const char* norm_type);

GGML_API float ggml_cognitive_tensor_similarity(
    struct ggml_tensor* a,
    struct ggml_tensor* b);

GGML_API void ggml_cognitive_tensor_print_stats(
    ggml_cognitive_kernel_t* kernel);

// Advanced Pattern Matching API

// Pattern matching result structure
typedef struct {
    float structural_similarity;     // Matula-Goebel structure match
    float semantic_similarity;      // Embedding space similarity  
    float confidence_score;         // Match confidence (0.0-1.0)
    float phase_coherence;          // Quantum phase alignment
    uint32_t match_type;            // Type of pattern match found
    bool is_exact_match;            // Perfect structural match
    bool is_fuzzy_match;            // Approximate match within threshold
} ggml_pattern_match_result_t;

// Pattern matching configuration
typedef struct {
    float structure_weight;         // Weight for structural similarity
    float semantic_weight;          // Weight for semantic similarity
    float phase_weight;             // Weight for phase coherence
    float fuzzy_threshold;          // Threshold for fuzzy matching
    bool enable_recursive;          // Enable recursive pattern matching
    bool enable_hierarchical;       // Enable multi-level matching
    uint32_t max_recursion_depth;   // Maximum recursion depth
} ggml_pattern_match_config_t;

// Pattern matching types
enum ggml_pattern_match_type {
    PATTERN_MATCH_EXACT = 1,        // Exact structural match
    PATTERN_MATCH_STRUCTURAL = 2,   // Structural similarity match
    PATTERN_MATCH_SEMANTIC = 3,     // Semantic similarity match
    PATTERN_MATCH_PHASE = 4,        // Phase coherence match
    PATTERN_MATCH_FUZZY = 5,        // Fuzzy approximate match
    PATTERN_MATCH_RECURSIVE = 6,    // Recursive hierarchical match
    PATTERN_MATCH_NONE = 0          // No significant match found
};

// Advanced pattern matching functions

// Structure-aware pattern matching using Matula-Goebel encoding
GGML_API ggml_pattern_match_result_t ggml_pattern_match_structural(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel,
    ggml_pattern_match_config_t* config);

// Probabilistic pattern matching with confidence scores
GGML_API ggml_pattern_match_result_t ggml_pattern_match_probabilistic(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel,
    float* confidence_map,
    size_t map_size);

// Multi-level pattern matching (syntactic, semantic, pragmatic)
GGML_API ggml_pattern_match_result_t ggml_pattern_match_multilevel(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel,
    uint32_t match_levels);

// Recursive pattern matching for hierarchical structures  
GGML_API ggml_pattern_match_result_t ggml_pattern_match_recursive(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel,
    uint32_t current_depth,
    uint32_t max_depth);

// Fuzzy pattern matching with threshold controls
GGML_API ggml_pattern_match_result_t ggml_pattern_match_fuzzy(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel,
    float threshold,
    float tolerance);

// Phase-coherence pattern matching using quantum phases
GGML_API ggml_pattern_match_result_t ggml_pattern_match_phase_coherence(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel);

// Unified advanced pattern matching interface
GGML_API ggml_pattern_match_result_t ggml_pattern_match_advanced(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel,
    ggml_pattern_match_config_t* config);

// Pattern matching utility functions
GGML_API ggml_pattern_match_config_t ggml_pattern_match_config_default(void);
GGML_API void ggml_pattern_match_result_print(ggml_pattern_match_result_t* result);
GGML_API float ggml_pattern_match_combine_scores(
    float structural, float semantic, float phase,
    ggml_pattern_match_config_t* config);

#ifdef __cplusplus
}
#endif