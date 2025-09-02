#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifndef GGML_API
#define GGML_API extern
#endif

// Constants
#define DISTRIBUTED_COGNITIVE_MAX_MEMBRANES 16

// Forward declarations
struct ggml_context;
struct ggml_backend;
struct ggml_tensor;

struct ggml_init_params {
    size_t mem_size;
    void* mem_buffer;
    bool no_alloc;
};

struct ggml_context* ggml_init(struct ggml_init_params params);
void ggml_free(struct ggml_context* ctx);

typedef struct {
    int dummy;
} cogfluence_system_t;

typedef struct {
    int dummy;
} opencog_atomspace_t;

typedef struct {
    int dummy;
} ggml_cognitive_kernel_t;

typedef struct {
    int dummy;
} dist_comm_engine_t;

typedef struct {
    char name[64];
} cogfluence_knowledge_unit_t;

// Stub functions
cogfluence_system_t* cogfluence_init(struct ggml_context* ctx);
void cogfluence_free(cogfluence_system_t* sys);
opencog_atomspace_t* opencog_atomspace_init(struct ggml_context* ctx);
void opencog_atomspace_free(opencog_atomspace_t* as);
ggml_cognitive_kernel_t* ggml_cognitive_kernel_init(struct ggml_context* ctx, int a, int b, int c);
void ggml_cognitive_kernel_free(ggml_cognitive_kernel_t* kernel);
dist_comm_engine_t* dist_comm_init(uint32_t id, const char* host, int port, struct ggml_context* ctx);
void dist_comm_free(dist_comm_engine_t* engine);
void opencog_link_cogfluence(opencog_atomspace_t* as, cogfluence_system_t* cf);
cogfluence_knowledge_unit_t* cogfluence_get_knowledge_unit(cogfluence_system_t* sys, uint64_t id);
uint64_t opencog_from_cogfluence_unit(opencog_atomspace_t* as, cogfluence_knowledge_unit_t* unit);

typedef enum {
    MEMBRANE_ELEMENTARY = 1,
    MEMBRANE_TISSUE = 2,
    MEMBRANE_ORGANISM = 3,
    MEMBRANE_ENVIRONMENT = 4
} membrane_type_t;

typedef struct {
    uint32_t membrane_id;
    char name[64];
    membrane_type_t type;
    uint32_t parent_membrane_id;
    uint32_t* child_membranes;
    size_t child_count;
    size_t child_capacity;
    uint64_t* cogfluence_units;
    size_t cogfluence_unit_count;
    uint64_t* opencog_atoms;
    size_t opencog_atom_count;
    struct ggml_tensor* evolution_rules;
    struct ggml_tensor* communication_rules;
    float permeability;
    float energy_level;
    bool active;
    uint64_t evolution_cycles;
    float efficiency_score;
} psystem_membrane_t;

typedef struct {
    float global_coherence;
    float cognitive_load;
    float attention_distribution[4];
    uint64_t total_operations;
    uint64_t successful_operations;
    float success_rate;
    uint32_t active_agents;
    uint32_t active_workflows;
    uint32_t active_membranes;
    float tensor_memory_usage;
    float tensor_computation_load;
    float* activation_flows;
    size_t activation_flow_count;
    float* membrane_depths;
    size_t membrane_depth_count;
    float* performance_history;
    size_t history_length;
    size_t history_capacity;
} metacognitive_dashboard_t;

typedef struct {
    char target_system[64];
    char target_parameter[64];
    float current_value;
    float target_value;
    float learning_rate;
    float momentum;
    float gradient;
    float previous_gradient;
    float baseline_performance;
    float current_performance;
    uint64_t optimization_cycles;
    float min_value;
    float max_value;
    bool converged;
} self_optimization_loop_t;

typedef enum {
    RECURSIVE_TARGET_OPTIMIZATION_ALGORITHM = 1,
    RECURSIVE_TARGET_LEARNING_RATE = 2,
    RECURSIVE_TARGET_ATTENTION_ALLOCATION = 3,
    RECURSIVE_TARGET_REASONING_PATTERNS = 4,
    RECURSIVE_TARGET_MEMORY_ORGANIZATION = 5
} recursive_improvement_target_t;

typedef struct {
    uint32_t improvement_id;
    recursive_improvement_target_t target_type;
    char target_description[128];
    uint32_t meta_loop_id;
    float meta_performance_history[10];
    size_t meta_history_count;
    uint32_t recursion_depth;
    uint32_t max_recursion_depth;
    float original_parameters[16];
    float current_parameters[16];
    float best_parameters[16];
    size_t parameter_count;
    float baseline_meta_performance;
    float current_meta_performance;
    float best_meta_performance;
    uint64_t improvement_cycles;
    bool bootstrapping_active;
    float bootstrap_multiplier;
    uint32_t bootstrap_iterations;
    bool active;
    bool converged;
    bool self_modifying;
} recursive_improvement_loop_t;

typedef struct {
    char change_description[256];
    float performance_before;
    float performance_after;
    float performance_delta;
    uint64_t timestamp;
    bool successful;
} architecture_evolution_record_t;

typedef struct {
    char module_name[64];
    char module_type[64];
    uint32_t module_id;
    float performance_contribution;
    float creation_threshold;
    bool active;
    uint64_t creation_time;
    uint64_t last_update_time;
} dynamic_cognitive_module_t;

typedef struct {
    struct ggml_context* ctx;
    struct ggml_backend* backend;
    cogfluence_system_t* cogfluence;
    opencog_atomspace_t* atomspace;
    ggml_cognitive_kernel_t* cognitive_kernel;
    void* comm_manager;
    bool real_distributed_mode;
    psystem_membrane_t* membranes;
    size_t membrane_count;
    size_t membrane_capacity;
    metacognitive_dashboard_t* dashboard;
    self_optimization_loop_t* optimization_loops;
    size_t optimization_loop_count;
    size_t optimization_loop_capacity;
    recursive_improvement_loop_t* recursive_improvement_loops;
    size_t recursive_improvement_count;
    size_t recursive_improvement_capacity;
    architecture_evolution_record_t* evolution_history;
    size_t evolution_history_count;
    size_t evolution_history_capacity;
    dynamic_cognitive_module_t* dynamic_modules;
    size_t dynamic_module_count;
    size_t dynamic_module_capacity;
    bool initialized;
    bool self_optimization_active;
    bool recursive_improvement_active;
    uint64_t system_time;
    uint64_t total_transductions;
    uint64_t successful_transductions;
    float system_efficiency;
    char endpoint[256];
    uint32_t agent_id;
} distributed_cognitive_architecture_t;

// Function declarations
GGML_API distributed_cognitive_architecture_t* distributed_cognitive_init(struct ggml_context* ctx, const char* endpoint);
GGML_API void distributed_cognitive_free(distributed_cognitive_architecture_t* arch);
GGML_API float distributed_cognitive_benchmark_performance(distributed_cognitive_architecture_t* arch);
GGML_API bool recursive_improvement_adapt_architecture(distributed_cognitive_architecture_t* arch, float performance_feedback);
GGML_API uint32_t psystem_create_membrane(distributed_cognitive_architecture_t* arch, const char* name, membrane_type_t type, uint32_t parent_id);
GGML_API bool architecture_evolution_create_module(distributed_cognitive_architecture_t* arch, const char* module_name, const char* module_type, float performance_threshold);
GGML_API bool architecture_evolution_remove_module(distributed_cognitive_architecture_t* arch, const char* module_name);
GGML_API bool architecture_evolution_modify_topology(distributed_cognitive_architecture_t* arch, const char* topology_change, float performance_feedback);
GGML_API bool architecture_evolution_learn_from_history(distributed_cognitive_architecture_t* arch, float current_performance);
GGML_API void architecture_evolution_track_change(distributed_cognitive_architecture_t* arch, const char* change_description, float performance_before, float performance_after);
