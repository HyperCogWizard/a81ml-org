#pragma once

//
// Advanced Pattern Matching Engine
//
// This module implements sophisticated pattern matching capabilities
// for the distributed cognitive architecture, including:
// - Tensor-based pattern recognition
// - Graph pattern matching in hypergraph structures
// - Temporal pattern detection
// - Attention-weighted pattern discovery
// - Integration with PLN reasoning and MOSES optimization
//

#include "ggml.h"
#include "ggml-opencog.h"
#include "ggml-cognitive-tensor.h"
#include "ggml-pln-reasoning.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Pattern matching configuration
#define PATTERN_MAX_NODES 256
#define PATTERN_MAX_EDGES 512
#define PATTERN_MAX_TEMPLATES 128
#define PATTERN_MAX_MATCHES 1024
#define PATTERN_MAX_VARIABLES 32
#define PATTERN_MAX_CONSTRAINTS 64

// Pattern types
typedef enum {
    PATTERN_TYPE_STRUCTURAL = 1,  // Graph structure patterns
    PATTERN_TYPE_SEMANTIC = 2,    // Semantic similarity patterns
    PATTERN_TYPE_TEMPORAL = 3,    // Time-based patterns
    PATTERN_TYPE_ATTENTION = 4,   // Attention-based patterns
    PATTERN_TYPE_TENSOR = 5,      // Tensor operation patterns
    PATTERN_TYPE_HYBRID = 6       // Combined pattern types
} pattern_type_t;

// Pattern matching algorithms
typedef enum {
    PATTERN_ALGO_EXACT = 1,       // Exact structural matching
    PATTERN_ALGO_FUZZY = 2,       // Fuzzy similarity matching
    PATTERN_ALGO_TENSOR_COSINE = 3, // Cosine similarity in tensor space
    PATTERN_ALGO_GRAPH_ISO = 4,   // Graph isomorphism
    PATTERN_ALGO_ATTENTION_WEIGHTED = 5, // Attention-weighted matching
    PATTERN_ALGO_PLN_GUIDED = 6,  // PLN-guided pattern matching
    PATTERN_ALGO_TEMPORAL = 7     // Temporal pattern matching
} pattern_algorithm_t;

// Pattern node structure
typedef struct pattern_node {
    uint64_t node_id;
    char label[64];
    pattern_type_t type;
    
    // OpenCog integration
    uint64_t atom_id;
    opencog_atom_type_t atom_type;
    opencog_truth_value_t truth_value;
    opencog_attention_value_t attention_value;
    
    // Tensor representation
    struct ggml_tensor* tensor_encoding;
    
    // Pattern constraints
    bool is_variable;
    char variable_name[32];
    float similarity_threshold;
    
    // Connections
    struct pattern_edge* edges;
    size_t edge_count;
    size_t edge_capacity;
} pattern_node_t;

// Pattern edge structure
typedef struct pattern_edge {
    uint64_t edge_id;
    uint64_t source_node_id;
    uint64_t target_node_id;
    
    char relation_type[64];
    float weight;
    bool is_directed;
    
    // Constraints
    float min_weight;
    float max_weight;
    bool weight_constraint_active;
} pattern_edge_t;

// Pattern template structure
typedef struct {
    uint64_t template_id;
    char name[128];
    pattern_type_t type;
    pattern_algorithm_t algorithm;
    
    // Pattern structure
    pattern_node_t* nodes;
    size_t node_count;
    size_t node_capacity;
    
    pattern_edge_t* edges;
    size_t edge_count;
    size_t edge_capacity;
    
    // Variables and constraints
    char variables[PATTERN_MAX_VARIABLES][32];
    size_t variable_count;
    
    // Matching parameters
    float similarity_threshold;
    float confidence_threshold;
    bool use_attention_weighting;
    bool use_temporal_constraints;
    
    // Performance metrics
    uint64_t match_count;
    uint64_t total_searches;
    float average_match_confidence;
    
    // Tensor representation
    struct ggml_tensor* template_tensor;
} pattern_template_t;

// Pattern match result
typedef struct {
    uint64_t match_id;
    uint64_t template_id;
    
    // Matched elements
    uint64_t* matched_atom_ids;
    size_t matched_count;
    
    // Variable bindings
    struct {
        char variable_name[32];
        uint64_t bound_atom_id;
        float binding_confidence;
    } variable_bindings[PATTERN_MAX_VARIABLES];
    size_t binding_count;
    
    // Match quality metrics
    float match_confidence;
    float structural_similarity;
    float semantic_similarity;
    float attention_weighted_score;
    
    // Temporal information
    uint64_t match_timestamp;
    uint64_t pattern_start_time;
    uint64_t pattern_end_time;
    
    // Context information
    uint64_t context_atoms[16];
    size_t context_count;
} pattern_match_t;

// Pattern matching engine
typedef struct {
    // Core components
    opencog_atomspace_t* atomspace;
    pln_reasoning_engine_t* pln_engine;
    ggml_cognitive_kernel_t* cognitive_kernel;
    struct ggml_context* ctx;
    
    // Pattern templates
    pattern_template_t* templates;
    size_t template_count;
    size_t template_capacity;
    
    // Match results
    pattern_match_t* matches;
    size_t match_count;
    size_t match_capacity;
    
    // Engine state
    bool initialized;
    bool continuous_matching;
    uint64_t last_update_time;
    
    // Performance metrics
    uint64_t total_pattern_searches;
    uint64_t successful_matches;
    float average_match_time;
    
    // Configuration
    float global_similarity_threshold;
    float global_confidence_threshold;
    bool use_parallel_matching;
    size_t max_concurrent_matches;
    
    // Caching
    struct ggml_tensor** cached_encodings;
    size_t cache_size;
    size_t cache_capacity;
} pattern_engine_t;

// Core pattern engine functions

// Initialize pattern matching engine
GGML_API pattern_engine_t* pattern_engine_init(
    struct ggml_context* ctx,
    opencog_atomspace_t* atomspace,
    pln_reasoning_engine_t* pln_engine,
    ggml_cognitive_kernel_t* cognitive_kernel);

// Free pattern matching engine
GGML_API void pattern_engine_free(pattern_engine_t* engine);

// Pattern template management

// Create pattern template
GGML_API pattern_template_t* pattern_create_template(
    pattern_engine_t* engine,
    const char* name,
    pattern_type_t type,
    pattern_algorithm_t algorithm);

// Add node to pattern template
GGML_API bool pattern_add_node(
    pattern_template_t* template,
    const char* label,
    pattern_type_t type,
    bool is_variable,
    const char* variable_name);

// Add edge to pattern template
GGML_API bool pattern_add_edge(
    pattern_template_t* template,
    uint64_t source_node_id,
    uint64_t target_node_id,
    const char* relation_type,
    float weight);

// Set pattern constraints
GGML_API bool pattern_set_constraints(
    pattern_template_t* template,
    float similarity_threshold,
    float confidence_threshold,
    bool use_attention_weighting);

// Compile pattern template to tensor
GGML_API bool pattern_compile_template(
    pattern_engine_t* engine,
    pattern_template_t* template);

// Pattern matching operations

// Find exact matches
GGML_API pattern_match_t* pattern_find_exact_matches(
    pattern_engine_t* engine,
    pattern_template_t* template,
    size_t* match_count);

// Find fuzzy matches
GGML_API pattern_match_t* pattern_find_fuzzy_matches(
    pattern_engine_t* engine,
    pattern_template_t* template,
    float similarity_threshold,
    size_t* match_count);

// Find tensor-based matches
GGML_API pattern_match_t* pattern_find_tensor_matches(
    pattern_engine_t* engine,
    pattern_template_t* template,
    struct ggml_tensor* query_tensor,
    size_t* match_count);

// Find attention-weighted matches
GGML_API pattern_match_t* pattern_find_attention_matches(
    pattern_engine_t* engine,
    pattern_template_t* template,
    float attention_threshold,
    size_t* match_count);

// Find temporal patterns
GGML_API pattern_match_t* pattern_find_temporal_matches(
    pattern_engine_t* engine,
    pattern_template_t* template,
    uint64_t time_window_start,
    uint64_t time_window_end,
    size_t* match_count);

// Advanced pattern matching

// Graph isomorphism matching
GGML_API pattern_match_t* pattern_graph_isomorphism(
    pattern_engine_t* engine,
    pattern_template_t* template,
    uint64_t* target_atoms,
    size_t target_count,
    size_t* match_count);

// PLN-guided pattern matching
GGML_API pattern_match_t* pattern_pln_guided_matching(
    pattern_engine_t* engine,
    pattern_template_t* template,
    pln_inference_context_t* reasoning_context,
    size_t* match_count);

// Multi-level pattern hierarchy
GGML_API pattern_match_t* pattern_hierarchical_matching(
    pattern_engine_t* engine,
    pattern_template_t** templates,
    size_t template_count,
    size_t* match_count);

// Continuous pattern detection
GGML_API bool pattern_start_continuous_detection(
    pattern_engine_t* engine,
    pattern_template_t* template);

GGML_API bool pattern_stop_continuous_detection(
    pattern_engine_t* engine,
    pattern_template_t* template);

// Pattern analysis and learning

// Analyze pattern frequency
GGML_API float pattern_analyze_frequency(
    pattern_engine_t* engine,
    pattern_template_t* template,
    uint64_t time_window);

// Learn patterns from data
GGML_API pattern_template_t* pattern_learn_from_matches(
    pattern_engine_t* engine,
    pattern_match_t* matches,
    size_t match_count,
    float confidence_threshold);

// Generate pattern variations
GGML_API pattern_template_t** pattern_generate_variations(
    pattern_engine_t* engine,
    pattern_template_t* base_template,
    size_t* variation_count);

// Optimize pattern templates
GGML_API bool pattern_optimize_template(
    pattern_engine_t* engine,
    pattern_template_t* template);

// Integration functions

// Convert AtomSpace to pattern graph
GGML_API bool pattern_atomspace_to_graph(
    pattern_engine_t* engine,
    uint64_t* atom_ids,
    size_t atom_count);

// Convert pattern to cognitive tensor
GGML_API struct ggml_tensor* pattern_to_tensor(
    pattern_engine_t* engine,
    pattern_template_t* template);

// Convert tensor to pattern
GGML_API pattern_template_t* pattern_from_tensor(
    pattern_engine_t* engine,
    struct ggml_tensor* tensor,
    const char* template_name);

// Apply pattern-based reasoning
GGML_API bool pattern_apply_reasoning(
    pattern_engine_t* engine,
    pattern_match_t* match,
    pln_inference_context_t* reasoning_context);

// Utility functions

// Calculate pattern similarity
GGML_API float pattern_calculate_similarity(
    pattern_engine_t* engine,
    pattern_template_t* template1,
    pattern_template_t* template2);

// Validate pattern match
GGML_API bool pattern_validate_match(
    pattern_engine_t* engine,
    pattern_match_t* match);

// Get pattern statistics
GGML_API void pattern_get_statistics(
    pattern_engine_t* engine,
    uint64_t* total_searches,
    uint64_t* successful_matches,
    float* average_match_time,
    float* success_rate);

// Export/Import patterns

// Export pattern template
GGML_API bool pattern_export_template(
    pattern_template_t* template,
    const char* filename);

// Import pattern template
GGML_API pattern_template_t* pattern_import_template(
    pattern_engine_t* engine,
    const char* filename);

// Export match results
GGML_API bool pattern_export_matches(
    pattern_match_t* matches,
    size_t match_count,
    const char* filename);

// Debugging and visualization

// Print pattern template
GGML_API void pattern_print_template(pattern_template_t* template);

// Print pattern matches
GGML_API void pattern_print_matches(
    pattern_match_t* matches,
    size_t match_count);

// Print engine statistics
GGML_API void pattern_print_engine_stats(pattern_engine_t* engine);

// Visualize pattern graph
GGML_API bool pattern_visualize_graph(
    pattern_template_t* template,
    const char* output_file);

// Free functions

// Free pattern template
GGML_API void pattern_free_template(pattern_template_t* template);

// Free pattern matches
GGML_API void pattern_free_matches(
    pattern_match_t* matches,
    size_t match_count);

#ifdef __cplusplus
}
#endif