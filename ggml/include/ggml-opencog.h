#pragma once

//
// OpenCog AtomSpace Integration Layer
//
// This header provides a simplified OpenCog AtomSpace interface
// that integrates with GGML tensors and Cogfluence knowledge units
// to enable distributed cognitive reasoning.
//

#include "ggml.h"
#include "ggml-cogfluence.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// AtomSpace limits
#define OPENCOG_MAX_ATOMS 2048
#define OPENCOG_MAX_LINKS 4096
#define OPENCOG_MAX_ATOM_NAME 256

// OpenCog atom types (simplified)
typedef enum {
    OPENCOG_CONCEPT_NODE = 1,
    OPENCOG_PREDICATE_NODE = 2,
    OPENCOG_VARIABLE_NODE = 3,
    OPENCOG_INHERITANCE_LINK = 4,
    OPENCOG_EVALUATION_LINK = 5,
    OPENCOG_IMPLICATION_LINK = 6,
    OPENCOG_SIMILARITY_LINK = 7,
    OPENCOG_MEMBER_LINK = 8
} opencog_atom_type_t;

// PLN Truth Value structure
typedef struct {
    float strength;         // Truth value strength [0, 1]
    float confidence;       // Confidence level [0, 1]
    float count;           // Evidence count
} opencog_truth_value_t;

// ECAN Attention Value structure
typedef struct {
    float sti;             // Short-term importance [-1, 1]
    float lti;             // Long-term importance [0, 1]
    float vlti;            // Very long-term importance [0, 1]
} opencog_attention_value_t;

// OpenCog Atom structure
typedef struct {
    uint64_t atom_id;
    char name[OPENCOG_MAX_ATOM_NAME];
    opencog_atom_type_t type;
    
    // PLN truth value
    opencog_truth_value_t truth_value;
    
    // ECAN attention value
    opencog_attention_value_t attention_value;
    
    // Tensor representation
    struct ggml_tensor* tensor_encoding;
    
    // Cogfluence mapping
    uint64_t cogfluence_unit_id;
    
    // Outgoing links (for compound atoms)
    uint64_t* outgoing;
    size_t outgoing_count;
    size_t outgoing_capacity;
    
    // Incoming links (reverse index)
    uint64_t* incoming;
    size_t incoming_count;
    size_t incoming_capacity;
    
    // Metadata
    uint64_t creation_time;
    uint64_t last_access;
    bool is_deleted;
} opencog_atom_t;

// OpenCog AtomSpace structure
typedef struct {
    struct ggml_context* ctx;
    
    // Atom storage
    opencog_atom_t* atoms;
    size_t atom_count;
    size_t atom_capacity;
    
    // AtomSpace state
    bool initialized;
    uint64_t next_atom_id;
    
    // ECAN parameters
    float attention_decay_rate;
    float attention_threshold;
    float importance_diffusion_rate;
    
    // PLN parameters
    float default_strength;
    float default_confidence;
    
    // Performance metrics
    uint64_t total_inferences;
    uint64_t successful_inferences;
    float reasoning_accuracy;
    
    // Integration with Cogfluence
    cogfluence_system_t* cogfluence_system;
} opencog_atomspace_t;

// Core AtomSpace functions
GGML_API opencog_atomspace_t* opencog_atomspace_init(struct ggml_context* ctx);
GGML_API void opencog_atomspace_free(opencog_atomspace_t* atomspace);

// Atom management
GGML_API uint64_t opencog_add_node(
    opencog_atomspace_t* atomspace,
    opencog_atom_type_t type,
    const char* name);

GGML_API uint64_t opencog_add_link(
    opencog_atomspace_t* atomspace,
    opencog_atom_type_t type,
    uint64_t* outgoing,
    size_t outgoing_count);

GGML_API opencog_atom_t* opencog_get_atom(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id);

GGML_API bool opencog_delete_atom(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id);

// Truth value operations
GGML_API void opencog_set_truth_value(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id,
    float strength,
    float confidence);

GGML_API opencog_truth_value_t opencog_get_truth_value(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id);

GGML_API opencog_truth_value_t opencog_pln_and(
    opencog_truth_value_t tv1,
    opencog_truth_value_t tv2);

GGML_API opencog_truth_value_t opencog_pln_or(
    opencog_truth_value_t tv1,
    opencog_truth_value_t tv2);

GGML_API opencog_truth_value_t opencog_pln_not(
    opencog_truth_value_t tv);

GGML_API opencog_truth_value_t opencog_pln_implication(
    opencog_truth_value_t premise,
    opencog_truth_value_t conclusion);

// Enhanced PLN inference rules
GGML_API opencog_truth_value_t opencog_pln_modus_ponens(
    opencog_truth_value_t premise_a,
    opencog_truth_value_t implication_ab);

GGML_API opencog_truth_value_t opencog_pln_deduction(
    opencog_truth_value_t ab_implication,
    opencog_truth_value_t bc_implication);

GGML_API opencog_truth_value_t opencog_pln_induction(
    opencog_truth_value_t specific_case,
    opencog_truth_value_t general_pattern);

GGML_API opencog_truth_value_t opencog_pln_abduction(
    opencog_truth_value_t implication_ab,
    opencog_truth_value_t conclusion_b);

GGML_API opencog_truth_value_t opencog_pln_similarity_inference(
    opencog_truth_value_t similarity_ab,
    opencog_truth_value_t implication_bc);

// Attention value operations (ECAN)
GGML_API void opencog_set_attention_value(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id,
    float sti,
    float lti,
    float vlti);

GGML_API opencog_attention_value_t opencog_get_attention_value(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id);

GGML_API void opencog_update_attention_values(opencog_atomspace_t* atomspace);

GGML_API void opencog_spread_attention(
    opencog_atomspace_t* atomspace,
    uint64_t source_atom_id,
    float amount);

// Reasoning operations (simplified PLN)
GGML_API bool opencog_infer_inheritance(
    opencog_atomspace_t* atomspace,
    uint64_t concept_a,
    uint64_t concept_b,
    uint64_t concept_c);

GGML_API bool opencog_infer_similarity(
    opencog_atomspace_t* atomspace,
    uint64_t concept_a,
    uint64_t concept_b);

GGML_API float opencog_compute_similarity(
    opencog_atomspace_t* atomspace,
    uint64_t atom1_id,
    uint64_t atom2_id);

// Integration with Cogfluence
GGML_API bool opencog_link_cogfluence(
    opencog_atomspace_t* atomspace,
    cogfluence_system_t* cogfluence_system);

GGML_API uint64_t opencog_from_cogfluence_unit(
    opencog_atomspace_t* atomspace,
    cogfluence_knowledge_unit_t* unit);

GGML_API bool opencog_to_cogfluence_unit(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id,
    cogfluence_system_t* cogfluence_system);

// Integration with GGML tensors
GGML_API struct ggml_tensor* opencog_atom_to_tensor(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id);

GGML_API uint64_t opencog_tensor_to_atom(
    opencog_atomspace_t* atomspace,
    struct ggml_tensor* tensor,
    const char* name);

// Query operations
GGML_API uint64_t* opencog_query_by_type(
    opencog_atomspace_t* atomspace,
    opencog_atom_type_t type,
    size_t* result_count);

GGML_API uint64_t* opencog_query_by_name(
    opencog_atomspace_t* atomspace,
    const char* name,
    size_t* result_count);

GGML_API uint64_t* opencog_query_incoming(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id,
    size_t* result_count);

GGML_API uint64_t* opencog_query_outgoing(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id,
    size_t* result_count);

// Utility functions
GGML_API void opencog_print_atom(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id);

GGML_API void opencog_print_atomspace_statistics(opencog_atomspace_t* atomspace);

GGML_API void opencog_save_atomspace(
    opencog_atomspace_t* atomspace,
    const char* filename);

GGML_API bool opencog_load_atomspace(
    opencog_atomspace_t* atomspace,
    const char* filename);

// PLN Reasoning Engine Functions
GGML_API bool opencog_pln_forward_chain(
    opencog_atomspace_t* atomspace,
    uint64_t premise_atoms[],
    size_t premise_count,
    void* rule);

GGML_API bool opencog_pln_backward_chain(
    opencog_atomspace_t* atomspace,
    uint64_t goal_atom_id,
    opencog_truth_value_t desired_tv);

GGML_API size_t opencog_pln_find_patterns(
    opencog_atomspace_t* atomspace,
    opencog_atom_type_t pattern_type,
    uint64_t* matching_atoms,
    size_t max_matches);

// PLN reasoning session management
typedef struct {
    opencog_atomspace_t* atomspace;
    uint64_t inference_count;
    uint64_t successful_count;
    float accuracy_threshold;
    bool forward_chaining_enabled;
    bool backward_chaining_enabled;
} opencog_pln_session_t;

GGML_API opencog_pln_session_t* opencog_pln_session_init(
    opencog_atomspace_t* atomspace,
    float accuracy_threshold);

GGML_API bool opencog_pln_inference_cycle(opencog_pln_session_t* session);

GGML_API void opencog_pln_session_free(opencog_pln_session_t* session);

#ifdef __cplusplus
}
#endif