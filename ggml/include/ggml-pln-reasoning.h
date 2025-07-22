#pragma once

//
// PLN (Probabilistic Logic Networks) Advanced Reasoning Engine
//
// This module implements advanced PLN reasoning capabilities
// for the distributed cognitive architecture, including:
// - Complex inference rules (Modus Ponens, Deduction, Induction)
// - Belief revision and uncertainty handling
// - Temporal reasoning and attention-weighted inference
// - Integration with cognitive tensors and attention economy
//

#include "ggml.h"
#include "ggml-opencog.h"
#include "ggml-cognitive-tensor.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum inference chain length
#define PLN_MAX_INFERENCE_CHAIN 32
#define PLN_MAX_PREMISES 8
#define PLN_MAX_HYPOTHESES 16

// PLN inference rule types
typedef enum {
    PLN_RULE_MODUS_PONENS = 1,    // P → Q, P ⊢ Q
    PLN_RULE_DEDUCTION = 2,       // A → B, B → C ⊢ A → C
    PLN_RULE_INDUCTION = 3,       // A → B, B ⊢ A
    PLN_RULE_ABDUCTION = 4,       // A → B, B ⊢ A (likely)
    PLN_RULE_SYMMETRY = 5,        // A ↔ B ⊢ B ↔ A
    PLN_RULE_INHERITANCE = 6,     // A isa B, B isa C ⊢ A isa C
    PLN_RULE_SIMILARITY = 7,      // A ~ B, B ~ C ⊢ A ~ C
    PLN_RULE_REVISION = 8,        // Belief revision rule
    PLN_RULE_CHOICE = 9,          // Choose between competing beliefs
    PLN_RULE_TEMPORAL = 10        // Temporal inference
} pln_inference_rule_t;

// PLN premise structure
typedef struct {
    uint64_t atom_id;
    opencog_truth_value_t truth_value;
    float attention_weight;
    float temporal_factor;
} pln_premise_t;

// PLN inference context
typedef struct {
    pln_inference_rule_t rule_type;
    pln_premise_t premises[PLN_MAX_PREMISES];
    size_t premise_count;
    
    // Inference parameters
    float confidence_threshold;
    float strength_threshold;
    float attention_threshold;
    
    // Temporal reasoning
    uint64_t inference_time;
    float temporal_decay_factor;
    
    // Attention-weighted reasoning
    bool use_attention_weights;
    float attention_amplification;
    
    // Results
    uint64_t conclusion_atom_id;
    opencog_truth_value_t conclusion_truth_value;
    float inference_confidence;
    
    // Chain reasoning
    uint64_t inference_chain[PLN_MAX_INFERENCE_CHAIN];
    size_t chain_length;
} pln_inference_context_t;

// PLN reasoning engine state
typedef struct {
    opencog_atomspace_t* atomspace;
    ggml_cognitive_kernel_t* cognitive_kernel;
    
    // Inference history
    pln_inference_context_t* inference_history;
    size_t history_capacity;
    size_t history_count;
    
    // Performance metrics
    uint64_t total_inferences;
    uint64_t successful_inferences;
    uint64_t failed_inferences;
    float average_confidence;
    
    // Reasoning parameters
    float default_confidence_threshold;
    float default_strength_threshold;
    float attention_boost_factor;
    float temporal_decay_rate;
    
    // Belief revision parameters
    float revision_factor;
    float choice_factor;
    
    bool initialized;
} pln_reasoning_engine_t;

// Core PLN reasoning engine functions

// Initialize PLN reasoning engine
GGML_API pln_reasoning_engine_t* pln_reasoning_engine_init(
    opencog_atomspace_t* atomspace,
    ggml_cognitive_kernel_t* cognitive_kernel);

// Free PLN reasoning engine
GGML_API void pln_reasoning_engine_free(pln_reasoning_engine_t* engine);

// Basic PLN inference operations

// Apply modus ponens: P → Q, P ⊢ Q
GGML_API bool pln_modus_ponens(
    pln_reasoning_engine_t* engine,
    uint64_t implication_atom_id,
    uint64_t premise_atom_id,
    uint64_t* conclusion_atom_id,
    opencog_truth_value_t* conclusion_tv);

// Apply deduction: A → B, B → C ⊢ A → C
GGML_API bool pln_deduction(
    pln_reasoning_engine_t* engine,
    uint64_t premise1_atom_id,
    uint64_t premise2_atom_id,
    uint64_t* conclusion_atom_id,
    opencog_truth_value_t* conclusion_tv);

// Apply induction: A → B, B ⊢ A
GGML_API bool pln_induction(
    pln_reasoning_engine_t* engine,
    uint64_t implication_atom_id,
    uint64_t conclusion_atom_id,
    uint64_t* premise_atom_id,
    opencog_truth_value_t* premise_tv);

// Apply abduction: A → B, B ⊢ A (likely explanation)
GGML_API bool pln_abduction(
    pln_reasoning_engine_t* engine,
    uint64_t implication_atom_id,
    uint64_t observation_atom_id,
    uint64_t* explanation_atom_id,
    opencog_truth_value_t* explanation_tv);

// Advanced PLN operations

// Multi-step inference chain
GGML_API bool pln_inference_chain(
    pln_reasoning_engine_t* engine,
    pln_inference_context_t* context);

// Belief revision with new evidence
GGML_API bool pln_belief_revision(
    pln_reasoning_engine_t* engine,
    uint64_t belief_atom_id,
    uint64_t evidence_atom_id,
    opencog_truth_value_t* revised_tv);

// Choice between competing beliefs
GGML_API bool pln_belief_choice(
    pln_reasoning_engine_t* engine,
    uint64_t* competing_atoms,
    size_t atom_count,
    uint64_t* chosen_atom_id,
    opencog_truth_value_t* chosen_tv);

// Attention-weighted inference
GGML_API bool pln_attention_weighted_inference(
    pln_reasoning_engine_t* engine,
    pln_inference_context_t* context);

// Temporal reasoning with decay
GGML_API bool pln_temporal_inference(
    pln_reasoning_engine_t* engine,
    pln_inference_context_t* context,
    uint64_t current_time);

// Integration with cognitive tensors

// Convert PLN inference to cognitive tensor
GGML_API struct ggml_tensor* pln_inference_to_tensor(
    struct ggml_context* ctx,
    pln_reasoning_engine_t* engine,
    pln_inference_context_t* inference);

// Apply tensor-based PLN operations
GGML_API bool pln_tensor_inference(
    pln_reasoning_engine_t* engine,
    struct ggml_tensor* premise_tensor,
    struct ggml_tensor* rule_tensor,
    struct ggml_tensor** conclusion_tensor);

// Reasoning performance analysis

// Analyze inference patterns
GGML_API void pln_analyze_inference_patterns(
    pln_reasoning_engine_t* engine);

// Get reasoning statistics
GGML_API void pln_get_statistics(
    pln_reasoning_engine_t* engine,
    float* success_rate,
    float* average_confidence,
    uint64_t* total_inferences);

// Optimize reasoning parameters
GGML_API void pln_optimize_parameters(
    pln_reasoning_engine_t* engine,
    float target_accuracy);

// Advanced reasoning utilities

// Create complex inference context
GGML_API pln_inference_context_t* pln_create_inference_context(
    pln_inference_rule_t rule_type,
    uint64_t* premise_atoms,
    size_t premise_count);

// Free inference context
GGML_API void pln_free_inference_context(pln_inference_context_t* context);

// Validate inference context
GGML_API bool pln_validate_inference_context(
    pln_reasoning_engine_t* engine,
    pln_inference_context_t* context);

// Export reasoning results
GGML_API bool pln_export_reasoning_results(
    pln_reasoning_engine_t* engine,
    const char* filename);

// Import reasoning knowledge
GGML_API bool pln_import_reasoning_knowledge(
    pln_reasoning_engine_t* engine,
    const char* filename);

// Debugging and visualization

// Print inference context
GGML_API void pln_print_inference_context(
    pln_reasoning_engine_t* engine,
    pln_inference_context_t* context);

// Print reasoning engine state
GGML_API void pln_print_engine_state(pln_reasoning_engine_t* engine);

// Visualize inference chain
GGML_API void pln_visualize_inference_chain(
    pln_reasoning_engine_t* engine,
    uint64_t* chain,
    size_t chain_length);

#ifdef __cplusplus
}
#endif