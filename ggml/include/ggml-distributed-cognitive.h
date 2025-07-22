#pragma once

//
// Distributed Cognitive Architecture Integration
//
// This header provides the transduction pipelines and integration layer
// between Cogfluence, OpenCog, and GGML systems for distributed
// cognitive processing with meta-cognitive awareness.
//

#include "ggml.h"
#include "ggml-cognitive-tensor.h"
#include "ggml-cogfluence.h"
#include "ggml-opencog.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum limits for the integrated system
#define DISTRIBUTED_COGNITIVE_MAX_AGENTS 32
#define DISTRIBUTED_COGNITIVE_MAX_WORKFLOWS 128
#define DISTRIBUTED_COGNITIVE_MAX_MEMBRANES 16

// P-System membrane types
typedef enum {
    MEMBRANE_ELEMENTARY = 1,
    MEMBRANE_TISSUE = 2,
    MEMBRANE_ORGANISM = 3,
    MEMBRANE_ENVIRONMENT = 4
} membrane_type_t;

// P-System membrane structure
typedef struct {
    uint32_t membrane_id;
    char name[64];
    membrane_type_t type;
    
    // Nested structure
    uint32_t parent_membrane_id;
    uint32_t* child_membranes;
    size_t child_count;
    size_t child_capacity;
    
    // Membrane contents
    uint64_t* cogfluence_units;
    size_t cogfluence_unit_count;
    uint64_t* opencog_atoms;
    size_t opencog_atom_count;
    
    // Membrane rules
    struct ggml_tensor* evolution_rules;
    struct ggml_tensor* communication_rules;
    
    // State
    float permeability;
    float energy_level;
    bool active;
    
    // Performance metrics
    uint64_t evolution_cycles;
    float efficiency_score;
} psystem_membrane_t;

// Meta-cognitive dashboard data
typedef struct {
    // System state
    float global_coherence;
    float cognitive_load;
    float attention_distribution[4];  // Memory, Reasoning, Communication, Self-modification
    
    // Performance metrics
    uint64_t total_operations;
    uint64_t successful_operations;
    float success_rate;
    
    // Network topology
    uint32_t active_agents;
    uint32_t active_workflows;
    uint32_t active_membranes;
    
    // Tensor statistics
    float tensor_memory_usage;
    float tensor_computation_load;
    
    // Visualization data
    float* activation_flows;
    size_t activation_flow_count;
    float* membrane_depths;
    size_t membrane_depth_count;
    
    // Time series data
    float* performance_history;
    size_t history_length;
    size_t history_capacity;
} metacognitive_dashboard_t;

// Self-optimization feedback loop
typedef struct {
    // Current optimization target
    char target_system[64];
    char target_parameter[64];
    
    // Optimization state
    float current_value;
    float target_value;
    float learning_rate;
    float momentum;
    
    // Gradient information
    float gradient;
    float previous_gradient;
    
    // Performance tracking
    float baseline_performance;
    float current_performance;
    uint64_t optimization_cycles;
    
    // Constraints
    float min_value;
    float max_value;
    bool converged;
} self_optimization_loop_t;

// Distributed cognitive architecture
typedef struct {
    // Core systems
    struct ggml_context* ctx;
    struct ggml_backend* backend;
    cogfluence_system_t* cogfluence;
    opencog_atomspace_t* atomspace;
    ggml_cognitive_kernel_t* cognitive_kernel;
    
    // Distributed communication
    void* comm_manager;  // Opaque pointer to avoid circular dependency
    bool real_distributed_mode;
    
    // P-System membranes
    psystem_membrane_t* membranes;
    size_t membrane_count;
    size_t membrane_capacity;
    
    // Meta-cognitive dashboard
    metacognitive_dashboard_t* dashboard;
    
    // Self-optimization loops
    self_optimization_loop_t* optimization_loops;
    size_t optimization_loop_count;
    size_t optimization_loop_capacity;
    
    // System state
    bool initialized;
    bool self_optimization_active;
    uint64_t system_time;
    
    // Performance metrics
    uint64_t total_transductions;
    uint64_t successful_transductions;
    float system_efficiency;
    
    // Network configuration
    char endpoint[256];
    uint32_t agent_id;
} distributed_cognitive_architecture_t;

// Core architecture functions
GGML_API distributed_cognitive_architecture_t* distributed_cognitive_init(
    struct ggml_context* ctx,
    const char* endpoint);

GGML_API void distributed_cognitive_free(
    distributed_cognitive_architecture_t* arch);

// Transduction pipeline functions
GGML_API bool transduction_cogfluence_to_opencog(
    distributed_cognitive_architecture_t* arch,
    uint64_t cogfluence_unit_id);

GGML_API bool transduction_opencog_to_ggml(
    distributed_cognitive_architecture_t* arch,
    uint64_t opencog_atom_id);

GGML_API bool transduction_ggml_to_cogfluence(
    distributed_cognitive_architecture_t* arch,
    struct ggml_tensor* tensor,
    const char* unit_name);

GGML_API bool transduction_full_pipeline(
    distributed_cognitive_architecture_t* arch,
    const char* input_data,
    char* output_data,
    size_t output_size);

// P-System membrane management
GGML_API uint32_t psystem_create_membrane(
    distributed_cognitive_architecture_t* arch,
    const char* name,
    membrane_type_t type,
    uint32_t parent_id);

GGML_API bool psystem_add_to_membrane(
    distributed_cognitive_architecture_t* arch,
    uint32_t membrane_id,
    uint64_t cogfluence_unit_id,
    uint64_t opencog_atom_id);

GGML_API bool psystem_evolve_membrane(
    distributed_cognitive_architecture_t* arch,
    uint32_t membrane_id);

GGML_API float psystem_compute_membrane_depth(
    distributed_cognitive_architecture_t* arch,
    uint32_t membrane_id);

// Meta-cognitive dashboard functions
GGML_API void dashboard_update(
    distributed_cognitive_architecture_t* arch);

GGML_API void dashboard_print(
    distributed_cognitive_architecture_t* arch);

GGML_API bool dashboard_export_visualization(
    distributed_cognitive_architecture_t* arch,
    const char* filename);

GGML_API float dashboard_compute_coherence(
    distributed_cognitive_architecture_t* arch);

// Self-optimization functions
GGML_API uint32_t optimization_create_loop(
    distributed_cognitive_architecture_t* arch,
    const char* target_system,
    const char* target_parameter,
    float initial_value,
    float target_value);

GGML_API bool optimization_update_loop(
    distributed_cognitive_architecture_t* arch,
    uint32_t loop_id,
    float current_performance);

GGML_API bool optimization_run_cycle(
    distributed_cognitive_architecture_t* arch);

GGML_API void optimization_print_status(
    distributed_cognitive_architecture_t* arch);

// Recursive workflow adaptation
GGML_API bool workflow_adapt_recursive(
    distributed_cognitive_architecture_t* arch,
    uint64_t workflow_id,
    float performance_feedback);

GGML_API bool workflow_generate_variant(
    distributed_cognitive_architecture_t* arch,
    uint64_t base_workflow_id,
    const char* variant_name);

// System integration functions
GGML_API bool system_synchronize_state(
    distributed_cognitive_architecture_t* arch);

GGML_API bool system_validate_consistency(
    distributed_cognitive_architecture_t* arch);

GGML_API float system_compute_efficiency(
    distributed_cognitive_architecture_t* arch);

// Attention allocation integration
GGML_API bool attention_allocate_distributed(
    distributed_cognitive_architecture_t* arch,
    float memory_weight,
    float reasoning_weight,
    float communication_weight,
    float self_modification_weight);

GGML_API void attention_update_ecan_weights(
    distributed_cognitive_architecture_t* arch);

// Dynamic tensor memory hooks
GGML_API struct ggml_tensor* dynamic_tensor_create(
    distributed_cognitive_architecture_t* arch,
    const char* name,
    int64_t* shape,
    size_t shape_dims);

GGML_API bool dynamic_tensor_reshape(
    distributed_cognitive_architecture_t* arch,
    struct ggml_tensor* tensor,
    int64_t* new_shape,
    size_t new_shape_dims);

GGML_API bool dynamic_tensor_optimize_layout(
    distributed_cognitive_architecture_t* arch,
    struct ggml_tensor* tensor);

// Hypergraph-tensor memory functions
GGML_API struct ggml_tensor* hypergraph_tensor_encode(
    distributed_cognitive_architecture_t* arch,
    uint64_t* node_ids,
    size_t node_count,
    uint64_t* edge_ids,
    size_t edge_count);

GGML_API bool hypergraph_tensor_decode(
    distributed_cognitive_architecture_t* arch,
    struct ggml_tensor* tensor,
    uint64_t** node_ids,
    size_t* node_count,
    uint64_t** edge_ids,
    size_t* edge_count);

// Distributed communication functions
GGML_API bool distributed_cognitive_enable_real_communication(
    distributed_cognitive_architecture_t* arch);

GGML_API bool distributed_cognitive_connect_to_agent(
    distributed_cognitive_architecture_t* arch,
    const char* remote_endpoint,
    const char* agent_name);

GGML_API bool distributed_cognitive_broadcast_state(
    distributed_cognitive_architecture_t* arch);

GGML_API bool distributed_cognitive_sync_with_network(
    distributed_cognitive_architecture_t* arch);

GGML_API void distributed_cognitive_print_network_status(
    distributed_cognitive_architecture_t* arch);

// Utility functions
GGML_API void distributed_cognitive_print_architecture(
    distributed_cognitive_architecture_t* arch);

GGML_API bool distributed_cognitive_run_test_suite(
    distributed_cognitive_architecture_t* arch);

GGML_API float distributed_cognitive_benchmark_performance(
    distributed_cognitive_architecture_t* arch);

#ifdef __cplusplus
}
#endif