#pragma once

//
// Real Distributed Communication Layer for Cognitive Architecture
//
// This header provides the networking infrastructure for real distributed
// cognitive agents to communicate across network boundaries, extending the
// existing RPC system for cognitive state synchronization.
//

#include "ggml.h"
#include "ggml-rpc.h"
#include "ggml-distributed-cognitive.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Maximum limits for distributed communication
#define DISTRIBUTED_COMM_MAX_AGENTS 64
#define DISTRIBUTED_COMM_MAX_NETWORKS 8
#define DISTRIBUTED_COMM_BUFFER_SIZE 8192

// Distributed agent communication types
typedef enum {
    AGENT_MSG_REGISTER = 1,
    AGENT_MSG_UNREGISTER = 2,
    AGENT_MSG_COGNITIVE_STATE = 3,
    AGENT_MSG_ATTENTION_UPDATE = 4,
    AGENT_MSG_TENSOR_EXCHANGE = 5,
    AGENT_MSG_WORKFLOW_REQUEST = 6,
    AGENT_MSG_MEMBRANE_SYNC = 7,
    AGENT_MSG_HEARTBEAT = 8
} agent_message_type_t;

// Cognitive state packet for network transmission
typedef struct {
    uint32_t agent_id;
    uint32_t timestamp;
    float coherence_level;
    float attention_values[4];  // Memory, Reasoning, Communication, Self-modification
    uint32_t active_workflows;
    uint32_t tensor_count;
    float cognitive_load;
    char endpoint[256];
} cognitive_state_packet_t;

// Attention update message
typedef struct {
    uint32_t agent_id;
    uint32_t target_agent_id;
    float attention_allocation[4];
    float salience_score;
    uint32_t membrane_id;
    uint64_t workflow_id;
} attention_update_packet_t;

// Tensor exchange message
typedef struct {
    uint32_t agent_id;
    uint32_t target_agent_id;
    uint64_t tensor_id;
    uint32_t tensor_dims[4];
    uint32_t tensor_type;
    uint32_t data_size;
    char tensor_name[64];
    // Data follows this structure
} tensor_exchange_packet_t;

// Agent registration info
typedef struct {
    uint32_t agent_id;
    char endpoint[256];
    char agent_name[128];
    uint32_t capabilities;
    uint32_t last_heartbeat;
    bool active;
    float cognitive_coherence;
    uint32_t message_count;
} distributed_agent_info_t;

// Network topology node
typedef struct {
    uint32_t network_id;
    char network_name[128];
    distributed_agent_info_t* agents;
    size_t agent_count;
    size_t agent_capacity;
    float network_coherence;
    uint64_t total_messages;
} cognitive_network_t;

// Distributed communication manager
typedef struct {
    // Core networking
    struct ggml_context* ctx;
    ggml_backend_t rpc_backend;
    char local_endpoint[256];
    uint32_t local_agent_id;
    
    // Agent registry
    distributed_agent_info_t* known_agents;
    size_t agent_count;
    size_t agent_capacity;
    
    // Network topology
    cognitive_network_t* networks;
    size_t network_count;
    size_t network_capacity;
    
    // Communication state
    bool server_active;
    bool discovery_active;
    uint32_t message_sequence;
    uint64_t total_bytes_sent;
    uint64_t total_bytes_received;
    
    // Performance metrics
    uint32_t successful_connections;
    uint32_t failed_connections;
    float network_latency_avg;
    float message_success_rate;
} distributed_communication_manager_t;

// Core communication manager functions
GGML_API distributed_communication_manager_t* distributed_comm_init(
    struct ggml_context* ctx,
    const char* local_endpoint,
    uint32_t agent_id);

GGML_API void distributed_comm_free(
    distributed_communication_manager_t* comm);

// Agent discovery and registration
GGML_API bool distributed_comm_start_server(
    distributed_communication_manager_t* comm);

GGML_API bool distributed_comm_register_agent(
    distributed_communication_manager_t* comm,
    const char* remote_endpoint,
    const char* agent_name);

GGML_API bool distributed_comm_unregister_agent(
    distributed_communication_manager_t* comm,
    uint32_t agent_id);

GGML_API distributed_agent_info_t* distributed_comm_find_agent(
    distributed_communication_manager_t* comm,
    uint32_t agent_id);

// Cognitive state synchronization
GGML_API bool distributed_comm_broadcast_cognitive_state(
    distributed_communication_manager_t* comm,
    distributed_cognitive_architecture_t* arch);

GGML_API bool distributed_comm_send_cognitive_state(
    distributed_communication_manager_t* comm,
    uint32_t target_agent_id,
    distributed_cognitive_architecture_t* arch);

GGML_API bool distributed_comm_receive_cognitive_state(
    distributed_communication_manager_t* comm,
    cognitive_state_packet_t* state_packet);

// Attention allocation across network
GGML_API bool distributed_comm_send_attention_update(
    distributed_communication_manager_t* comm,
    uint32_t target_agent_id,
    float attention_weights[4],
    float salience,
    uint32_t membrane_id,
    uint64_t workflow_id);

GGML_API bool distributed_comm_broadcast_attention_update(
    distributed_communication_manager_t* comm,
    float attention_weights[4],
    float salience,
    uint32_t membrane_id,
    uint64_t workflow_id);

// Tensor exchange across network
GGML_API bool distributed_comm_send_tensor(
    distributed_communication_manager_t* comm,
    uint32_t target_agent_id,
    struct ggml_tensor* tensor,
    const char* tensor_name);

GGML_API struct ggml_tensor* distributed_comm_receive_tensor(
    distributed_communication_manager_t* comm,
    uint32_t from_agent_id,
    const char* expected_name);

// Multi-agent workflow coordination
GGML_API bool distributed_comm_request_workflow_participation(
    distributed_communication_manager_t* comm,
    uint32_t target_agent_id,
    uint64_t workflow_id,
    const char* workflow_description);

GGML_API bool distributed_comm_coordinate_distributed_workflow(
    distributed_communication_manager_t* comm,
    uint64_t workflow_id,
    uint32_t* participant_agents,
    size_t participant_count);

// Membrane synchronization
GGML_API bool distributed_comm_sync_membrane_state(
    distributed_communication_manager_t* comm,
    uint32_t membrane_id,
    psystem_membrane_t* membrane);

GGML_API bool distributed_comm_request_membrane_sync(
    distributed_communication_manager_t* comm,
    uint32_t target_agent_id,
    uint32_t membrane_id);

// Network topology management
GGML_API uint32_t distributed_comm_create_network(
    distributed_communication_manager_t* comm,
    const char* network_name);

GGML_API bool distributed_comm_join_network(
    distributed_communication_manager_t* comm,
    uint32_t network_id,
    uint32_t agent_id);

GGML_API bool distributed_comm_leave_network(
    distributed_communication_manager_t* comm,
    uint32_t network_id,
    uint32_t agent_id);

GGML_API float distributed_comm_compute_network_coherence(
    distributed_communication_manager_t* comm,
    uint32_t network_id);

// Heartbeat and health monitoring
GGML_API bool distributed_comm_send_heartbeat(
    distributed_communication_manager_t* comm,
    uint32_t target_agent_id);

GGML_API void distributed_comm_update_heartbeats(
    distributed_communication_manager_t* comm);

GGML_API void distributed_comm_cleanup_inactive_agents(
    distributed_communication_manager_t* comm);

// Performance monitoring
GGML_API void distributed_comm_print_statistics(
    distributed_communication_manager_t* comm);

GGML_API float distributed_comm_get_network_latency(
    distributed_communication_manager_t* comm,
    uint32_t target_agent_id);

GGML_API bool distributed_comm_run_connectivity_test(
    distributed_communication_manager_t* comm);

// Integration with cognitive architecture
GGML_API bool distributed_cognitive_process_network_messages(
    distributed_cognitive_architecture_t* arch,
    distributed_communication_manager_t* comm);

#ifdef __cplusplus
}
#endif