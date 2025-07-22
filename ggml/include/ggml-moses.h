#pragma once

//
// MOSES (Meta-Optimizing Semantic Evolution) System
//
// This header provides a MOSES optimization system that integrates with 
// the OpenCog AtomSpace and cognitive tensors to enable evolutionary
// program synthesis and meta-optimization.
//

#include "ggml.h"
#include "ggml-opencog.h"
#include "ggml-cognitive-tensor.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// MOSES configuration constants
#define MOSES_MAX_POPULATION 256
#define MOSES_MAX_PROGRAM_SIZE 128
#define MOSES_MAX_GENERATIONS 1000
#define MOSES_MAX_FEATURES 64
#define MOSES_MAX_EXEMPLARS 32

// MOSES program node types
typedef enum {
    MOSES_NODE_CONSTANT = 1,
    MOSES_NODE_VARIABLE = 2,
    MOSES_NODE_FUNCTION = 3,
    MOSES_NODE_BOOLEAN = 4,
    MOSES_NODE_NUMERIC = 5,
    MOSES_NODE_LOGICAL_AND = 6,
    MOSES_NODE_LOGICAL_OR = 7,
    MOSES_NODE_LOGICAL_NOT = 8,
    MOSES_NODE_ARITHMETIC_ADD = 9,
    MOSES_NODE_ARITHMETIC_MUL = 10,
    MOSES_NODE_CONDITIONAL = 11
} moses_node_type_t;

// MOSES program node structure
typedef struct moses_program_node {
    moses_node_type_t type;
    char name[64];
    float value;
    
    // Tree structure
    struct moses_program_node* left;
    struct moses_program_node* right;
    struct moses_program_node* condition; // For conditional nodes
    
    // Tensor encoding
    struct ggml_tensor* tensor_encoding;
    
    // Metadata
    uint32_t depth;
    uint32_t node_id;
    float complexity_score;
} moses_program_node_t;

// MOSES program representation
typedef struct {
    moses_program_node_t* root;
    float fitness_score;
    float complexity_penalty;
    float diversity_score;
    uint32_t generation;
    uint32_t program_id;
    
    // Integration with cognitive systems
    uint64_t atomspace_atom_id;
    ggml_matula_encoding_t matula_encoding;
    
    // Performance metrics
    uint32_t evaluation_count;
    float execution_time;
    bool is_valid;
} moses_program_t;

// MOSES population structure
typedef struct {
    moses_program_t* programs;
    size_t population_size;
    size_t max_population_size;
    
    // Statistics
    float best_fitness;
    float average_fitness;
    float diversity_measure;
    uint32_t current_generation;
    
    // Selection and evolution parameters
    float mutation_rate;
    float crossover_rate;
    float elitism_rate;
    float complexity_weight;
} moses_population_t;

// MOSES fitness case structure
typedef struct {
    float* input_features;
    size_t feature_count;
    float target_output;
    float weight;
    char description[128];
} moses_fitness_case_t;

// MOSES training dataset
typedef struct {
    moses_fitness_case_t* fitness_cases;
    size_t case_count;
    size_t max_cases;
    
    // Feature metadata
    char feature_names[MOSES_MAX_FEATURES][64];
    float feature_min[MOSES_MAX_FEATURES];
    float feature_max[MOSES_MAX_FEATURES];
    size_t feature_count;
    
    // Output metadata
    float output_min;
    float output_max;
    bool is_classification;
} moses_dataset_t;

// MOSES evolutionary operators
typedef struct {
    // Mutation operators
    float (*point_mutation)(moses_program_node_t* node, float mutation_strength);
    moses_program_node_t* (*subtree_mutation)(moses_program_node_t* node, int max_depth);
    moses_program_node_t* (*hoist_mutation)(moses_program_node_t* node);
    
    // Crossover operators
    moses_program_t* (*subtree_crossover)(moses_program_t* parent1, moses_program_t* parent2);
    moses_program_t* (*uniform_crossover)(moses_program_t* parent1, moses_program_t* parent2);
    
    // Selection operators
    moses_program_t* (*tournament_selection)(moses_population_t* population, int tournament_size);
    moses_program_t* (*roulette_selection)(moses_population_t* population);
    
    // Specialized operators
    moses_program_t* (*reduce_program)(moses_program_t* program);
    moses_program_t* (*expand_program)(moses_program_t* program, ggml_cognitive_kernel_t* kernel);
} moses_evolutionary_operators_t;

// MOSES system configuration
typedef struct {
    // Evolution parameters
    uint32_t population_size;
    uint32_t max_generations;
    float target_fitness;
    float fitness_tolerance;
    
    // Operator probabilities
    float mutation_probability;
    float crossover_probability;
    float reproduction_probability;
    
    // Selection parameters
    uint32_t tournament_size;
    float elitism_fraction;
    
    // Complexity control
    uint32_t max_program_depth;
    uint32_t max_program_nodes;
    float complexity_penalty_weight;
    
    // Diversity control
    float diversity_weight;
    float novelty_threshold;
    
    // Meta-optimization parameters
    bool enable_meta_optimization;
    uint32_t meta_generations;
    float meta_learning_rate;
    
    // Integration settings
    bool use_atomspace_seeding;
    bool use_cognitive_tensor_fitness;
    bool enable_prime_structure_bias;
} moses_config_t;

// Main MOSES system structure
typedef struct {
    // Core components
    moses_population_t* population;
    moses_dataset_t* training_data;
    moses_evolutionary_operators_t operators;
    moses_config_t config;
    
    // Integration with cognitive architecture
    opencog_atomspace_t* atomspace;
    ggml_cognitive_kernel_t* cognitive_kernel;
    struct ggml_context* ctx;
    
    // System state
    bool initialized;
    uint32_t total_evaluations;
    uint32_t successful_runs;
    float best_ever_fitness;
    moses_program_t* best_program;
    
    // Performance tracking
    uint64_t start_time;
    float total_evolution_time;
    float average_fitness_trajectory[MOSES_MAX_GENERATIONS];
} moses_system_t;

// Core MOSES functions
GGML_API moses_system_t* moses_system_init(
    struct ggml_context* ctx,
    opencog_atomspace_t* atomspace,
    ggml_cognitive_kernel_t* cognitive_kernel);

GGML_API void moses_system_free(moses_system_t* moses);

GGML_API bool moses_system_configure(
    moses_system_t* moses,
    moses_config_t* config);

// Dataset management
GGML_API moses_dataset_t* moses_dataset_create(size_t max_cases, size_t feature_count);
GGML_API void moses_dataset_free(moses_dataset_t* dataset);

GGML_API bool moses_dataset_add_case(
    moses_dataset_t* dataset,
    float* input_features,
    float target_output,
    float weight);

GGML_API bool moses_dataset_load_from_file(
    moses_dataset_t* dataset,
    const char* filename);

// Program creation and manipulation
GGML_API moses_program_t* moses_program_create_random(
    moses_system_t* moses,
    int max_depth);

GGML_API moses_program_t* moses_program_create_from_atomspace(
    moses_system_t* moses,
    uint64_t atom_id);

GGML_API moses_program_t* moses_program_clone(moses_program_t* program);
GGML_API void moses_program_free(moses_program_t* program);

GGML_API float moses_program_evaluate(
    moses_system_t* moses,
    moses_program_t* program,
    float* input_features);

GGML_API float moses_program_compute_fitness(
    moses_system_t* moses,
    moses_program_t* program);

// Evolution operations
GGML_API moses_population_t* moses_population_initialize(
    moses_system_t* moses);

GGML_API void moses_population_free(moses_population_t* population);

GGML_API bool moses_evolve_generation(
    moses_system_t* moses);

GGML_API moses_program_t* moses_run_evolution(
    moses_system_t* moses,
    moses_dataset_t* training_data);

// Meta-optimization
GGML_API bool moses_meta_optimize_parameters(
    moses_system_t* moses,
    moses_dataset_t* validation_data);

GGML_API float moses_evaluate_parameter_configuration(
    moses_system_t* moses,
    moses_config_t* test_config,
    moses_dataset_t* validation_data);

// Integration with cognitive architecture
GGML_API uint64_t moses_program_to_atomspace(
    moses_system_t* moses,
    moses_program_t* program);

GGML_API struct ggml_tensor* moses_program_to_tensor(
    moses_system_t* moses,
    moses_program_t* program);

GGML_API moses_program_t* moses_program_from_tensor(
    moses_system_t* moses,
    struct ggml_tensor* tensor);

// Utility functions
GGML_API void moses_program_print(moses_program_t* program);
GGML_API void moses_program_print_tree(moses_program_node_t* node, int indent);

GGML_API char* moses_program_to_string(moses_program_t* program);
GGML_API moses_program_t* moses_program_from_string(
    moses_system_t* moses,
    const char* program_string);

GGML_API void moses_system_print_statistics(moses_system_t* moses);
GGML_API void moses_population_print_statistics(moses_population_t* population);

GGML_API bool moses_system_save_state(
    moses_system_t* moses,
    const char* filename);

GGML_API bool moses_system_load_state(
    moses_system_t* moses,
    const char* filename);

// Advanced features
GGML_API float moses_compute_program_diversity(
    moses_program_t** programs,
    size_t program_count);

GGML_API moses_program_t* moses_create_ensemble(
    moses_program_t** programs,
    size_t program_count,
    float* weights);

GGML_API bool moses_program_validate_semantics(
    moses_system_t* moses,
    moses_program_t* program);

#ifdef __cplusplus
}
#endif