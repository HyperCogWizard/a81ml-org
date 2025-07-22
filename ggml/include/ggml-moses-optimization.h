#pragma once

//
// MOSES (Meta-Optimizing Semantic Evolution System)
//
// This module implements the MOSES program evolution system
// for the distributed cognitive architecture, including:
// - Program representation and evolution
// - Fitness evaluation and selection
// - Population management and breeding
// - Integration with cognitive tensors and PLN reasoning
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

// MOSES configuration constants
#define MOSES_MAX_POPULATION 256
#define MOSES_MAX_PROGRAM_SIZE 1024
#define MOSES_MAX_GENERATIONS 1000
#define MOSES_MAX_VARIABLES 32
#define MOSES_MAX_OPERATORS 64

// Program representation types
typedef enum {
    MOSES_OP_LITERAL = 1,     // Constant value
    MOSES_OP_VARIABLE = 2,    // Input variable
    MOSES_OP_AND = 3,         // Logical AND
    MOSES_OP_OR = 4,          // Logical OR
    MOSES_OP_NOT = 5,         // Logical NOT
    MOSES_OP_PLUS = 6,        // Addition
    MOSES_OP_MINUS = 7,       // Subtraction
    MOSES_OP_MULTIPLY = 8,    // Multiplication
    MOSES_OP_DIVIDE = 9,      // Division
    MOSES_OP_IF_THEN_ELSE = 10, // Conditional
    MOSES_OP_GREATER = 11,    // Greater than
    MOSES_OP_LESS = 12,       // Less than
    MOSES_OP_EQUAL = 13,      // Equal
    MOSES_OP_SIN = 14,        // Sine function
    MOSES_OP_COS = 15,        // Cosine function
    MOSES_OP_EXP = 16,        // Exponential
    MOSES_OP_LOG = 17,        // Logarithm
    MOSES_OP_PLN_AND = 18,    // PLN AND operation
    MOSES_OP_PLN_OR = 19,     // PLN OR operation
    MOSES_OP_PLN_NOT = 20,    // PLN NOT operation
    MOSES_OP_ATTENTION = 21,  // Attention operation
    MOSES_OP_MEMORY = 22      // Memory operation
} moses_operator_t;

// Program node structure
typedef struct moses_node {
    moses_operator_t op;
    float value;                    // For literals
    int variable_index;             // For variables
    struct moses_node* left;        // Left child
    struct moses_node* right;       // Right child
    struct moses_node* condition;   // For IF_THEN_ELSE
    
    // Cognitive extensions
    uint64_t atom_id;              // Associated OpenCog atom
    opencog_truth_value_t truth_value; // PLN truth value
    float attention_value;         // ECAN attention
} moses_node_t;

// Program structure
typedef struct {
    moses_node_t* root;
    size_t node_count;
    size_t max_depth;
    
    // Performance metrics
    float fitness;
    float complexity;
    float cognitive_fitness;       // Fitness based on cognitive metrics
    
    // Generation info
    uint32_t generation;
    uint32_t individual_id;
    
    // Parent information
    uint32_t parent1_id;
    uint32_t parent2_id;
    
    // Tensor representation
    struct ggml_tensor* tensor_encoding;
} moses_program_t;

// Population structure
typedef struct {
    moses_program_t* individuals;
    size_t population_size;
    size_t max_population_size;
    
    // Evolution parameters
    float mutation_rate;
    float crossover_rate;
    float elite_ratio;
    
    // Generation tracking
    uint32_t current_generation;
    uint32_t max_generations;
    
    // Best individual tracking
    moses_program_t* best_individual;
    float best_fitness;
    
    // Diversity metrics
    float population_diversity;
    float average_fitness;
} moses_population_t;

// Fitness evaluation context
typedef struct {
    // Training data
    float** input_data;
    float* target_output;
    size_t data_size;
    size_t input_dimension;
    
    // Cognitive context
    opencog_atomspace_t* atomspace;
    pln_reasoning_engine_t* pln_engine;
    ggml_cognitive_kernel_t* cognitive_kernel;
    
    // Evaluation parameters
    float accuracy_weight;
    float complexity_weight;
    float cognitive_weight;
    
    // Performance tracking
    uint64_t evaluations_count;
    float best_seen_fitness;
} moses_fitness_context_t;

// MOSES evolution engine
typedef struct {
    moses_population_t* population;
    moses_fitness_context_t* fitness_context;
    
    // Evolution state
    bool initialized;
    bool evolution_active;
    uint32_t total_generations;
    
    // Statistics
    uint64_t total_evaluations;
    uint64_t total_mutations;
    uint64_t total_crossovers;
    float convergence_rate;
    
    // Integration components
    opencog_atomspace_t* atomspace;
    pln_reasoning_engine_t* pln_engine;
    ggml_cognitive_kernel_t* cognitive_kernel;
    struct ggml_context* ctx;
    
    // Evolution parameters
    float target_fitness;
    bool use_cognitive_fitness;
    bool use_attention_weighting;
} moses_engine_t;

// Core MOSES engine functions

// Initialize MOSES engine
GGML_API moses_engine_t* moses_engine_init(
    struct ggml_context* ctx,
    opencog_atomspace_t* atomspace,
    pln_reasoning_engine_t* pln_engine,
    ggml_cognitive_kernel_t* cognitive_kernel);

// Free MOSES engine
GGML_API void moses_engine_free(moses_engine_t* engine);

// Program management

// Create random program
GGML_API moses_program_t* moses_create_random_program(
    moses_engine_t* engine,
    size_t max_depth,
    size_t num_variables);

// Clone program
GGML_API moses_program_t* moses_clone_program(moses_program_t* program);

// Free program
GGML_API void moses_free_program(moses_program_t* program);

// Evaluate program
GGML_API float moses_evaluate_program(
    moses_engine_t* engine,
    moses_program_t* program,
    float* inputs,
    size_t input_size);

// Population management

// Initialize population
GGML_API bool moses_initialize_population(
    moses_engine_t* engine,
    size_t population_size,
    size_t max_depth,
    size_t num_variables);

// Evaluate population fitness
GGML_API void moses_evaluate_population(moses_engine_t* engine);

// Selection and breeding

// Tournament selection
GGML_API moses_program_t* moses_tournament_selection(
    moses_engine_t* engine,
    size_t tournament_size);

// Roulette wheel selection
GGML_API moses_program_t* moses_roulette_selection(moses_engine_t* engine);

// Crossover operation
GGML_API moses_program_t* moses_crossover(
    moses_engine_t* engine,
    moses_program_t* parent1,
    moses_program_t* parent2);

// Mutation operation
GGML_API moses_program_t* moses_mutate(
    moses_engine_t* engine,
    moses_program_t* program);

// Evolution operations

// Run single generation
GGML_API bool moses_evolve_generation(moses_engine_t* engine);

// Run full evolution
GGML_API bool moses_evolve(
    moses_engine_t* engine,
    size_t max_generations,
    float target_fitness);

// Check convergence
GGML_API bool moses_check_convergence(moses_engine_t* engine);

// Cognitive integration

// Convert program to OpenCog representation
GGML_API uint64_t moses_program_to_atom(
    moses_engine_t* engine,
    moses_program_t* program);

// Create program from OpenCog atom
GGML_API moses_program_t* moses_atom_to_program(
    moses_engine_t* engine,
    uint64_t atom_id);

// Evaluate cognitive fitness
GGML_API float moses_evaluate_cognitive_fitness(
    moses_engine_t* engine,
    moses_program_t* program);

// Apply PLN reasoning to program evolution
GGML_API bool moses_pln_guided_evolution(
    moses_engine_t* engine,
    moses_program_t* program);

// Tensor operations

// Convert program to tensor
GGML_API struct ggml_tensor* moses_program_to_tensor(
    struct ggml_context* ctx,
    moses_program_t* program);

// Create program from tensor
GGML_API moses_program_t* moses_tensor_to_program(
    moses_engine_t* engine,
    struct ggml_tensor* tensor);

// Tensor-based crossover
GGML_API moses_program_t* moses_tensor_crossover(
    moses_engine_t* engine,
    struct ggml_tensor* parent1_tensor,
    struct ggml_tensor* parent2_tensor);

// Analysis and optimization

// Analyze population diversity
GGML_API float moses_calculate_diversity(moses_engine_t* engine);

// Get evolution statistics
GGML_API void moses_get_statistics(
    moses_engine_t* engine,
    float* best_fitness,
    float* average_fitness,
    float* diversity,
    uint32_t* generation);

// Optimize evolution parameters
GGML_API void moses_optimize_parameters(moses_engine_t* engine);

// Export/Import

// Export best program
GGML_API bool moses_export_best_program(
    moses_engine_t* engine,
    const char* filename);

// Import program
GGML_API moses_program_t* moses_import_program(
    moses_engine_t* engine,
    const char* filename);

// Utility functions

// Print program structure
GGML_API void moses_print_program(
    moses_program_t* program,
    int indent);

// Print population statistics
GGML_API void moses_print_population_stats(moses_engine_t* engine);

// Print evolution progress
GGML_API void moses_print_evolution_progress(moses_engine_t* engine);

// Validate program structure
GGML_API bool moses_validate_program(moses_program_t* program);

// Calculate program complexity
GGML_API float moses_calculate_complexity(moses_program_t* program);

// Advanced features

// Multi-objective optimization
GGML_API bool moses_multi_objective_evolve(
    moses_engine_t* engine,
    float* objective_weights,
    size_t num_objectives);

// Parallel evolution
GGML_API bool moses_parallel_evolve(
    moses_engine_t* engine,
    size_t num_threads);

// Adaptive parameter adjustment
GGML_API void moses_adaptive_parameters(moses_engine_t* engine);

// Niching and speciation
GGML_API bool moses_apply_niching(moses_engine_t* engine);

#ifdef __cplusplus
}
#endif