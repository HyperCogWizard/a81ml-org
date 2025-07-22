#pragma once

//
// Real Distributed Communication System
//
// This module implements true distributed communication capabilities
// for the cognitive agent network, including:
// - Network-based agent communication via TCP/UDP
// - Message routing and discovery protocols
// - Distributed coordination and consensus
// - Integration with ggml-rpc for tensor streaming
// - Cognitive message types and protocols
//

#include "ggml.h"
#include "ggml-rpc.h"
#include "ggml-distributed-cognitive.h"
#include "ggml-opencog.h"
#include "ggml-cognitive-tensor.h"
#include "ggml-pln-reasoning.h"
#include "ggml-pattern-matching.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Network configuration
#define DIST_COMM_MAX_AGENTS 256
#define DIST_COMM_MAX_CONNECTIONS 1024
#define DIST_COMM_MAX_MESSAGE_SIZE 65536
#define DIST_COMM_MAX_ROUTING_HOPS 16
#define DIST_COMM_DEFAULT_PORT 8000
#define DIST_COMM_DISCOVERY_PORT 8001
#define DIST_COMM_HEARTBEAT_INTERVAL 30 // seconds

// Additional configuration from master integration
#define DISTRIBUTED_COMM_MAX_NETWORKS 8
#define DISTRIBUTED_COMM_BUFFER_SIZE 8192

// Message types
typedef enum {
    DIST_MSG_HEARTBEAT = 1,       // Agent alive signal
    DIST_MSG_DISCOVERY = 2,       // Agent discovery
    DIST_MSG_COGNITIVE_TENSOR = 3, // Cognitive tensor data
    DIST_MSG_PLN_INFERENCE = 4,   // PLN reasoning request/response
    DIST_MSG_PATTERN_MATCH = 5,   // Pattern matching request
    DIST_MSG_ATTENTION_UPDATE = 6, // ECAN attention spreading
    DIST_MSG_KNOWLEDGE_SYNC = 7,  // Knowledge base synchronization
    DIST_MSG_TASK_REQUEST = 8,    // Task delegation
    DIST_MSG_TASK_RESPONSE = 9,   // Task completion result
    DIST_MSG_COORDINATION = 10,   // Multi-agent coordination
    DIST_MSG_CONSENSUS = 11,      // Consensus protocol
    DIST_MSG_ERROR = 12,          // Error notification
    DIST_MSG_DEBUG = 13,          // Debug information
    // Additional message types from master integration
    AGENT_MSG_REGISTER = 14,      // Agent registration
    AGENT_MSG_UNREGISTER = 15,    // Agent deregistration
    AGENT_MSG_COGNITIVE_STATE = 16, // Cognitive state updates
    AGENT_MSG_TENSOR_EXCHANGE = 17, // Direct tensor exchange
    AGENT_MSG_WORKFLOW_REQUEST = 18, // Workflow delegation
    AGENT_MSG_MEMBRANE_SYNC = 19   // Membrane synchronization
} dist_message_type_t;

// Communication protocols
typedef enum {
    DIST_PROTOCOL_TCP = 1,        // Reliable TCP communication
    DIST_PROTOCOL_UDP = 2,        // Fast UDP communication
    DIST_PROTOCOL_GGML_RPC = 3,   // ggml-rpc protocol
    DIST_PROTOCOL_MULTICAST = 4,  // UDP multicast
    DIST_PROTOCOL_WEBSOCKET = 5   // WebSocket (future)
} dist_protocol_t;

// Agent states
typedef enum {
    DIST_AGENT_UNKNOWN = 0,
    DIST_AGENT_CONNECTING = 1,
    DIST_AGENT_CONNECTED = 2,
    DIST_AGENT_ACTIVE = 3,
    DIST_AGENT_IDLE = 4,
    DIST_AGENT_BUSY = 5,
    DIST_AGENT_DISCONNECTED = 6,
    DIST_AGENT_ERROR = 7
} dist_agent_state_t;

// Cognitive state packet for network transmission (integrated from master)
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

// Attention update message (integrated from master)
typedef struct {
    uint32_t agent_id;
    uint32_t target_concept_id;
    float attention_delta;
    uint32_t update_type;  // 0=increment, 1=set, 2=decay
    uint32_t timestamp;
} attention_update_packet_t;

// Message header structure
typedef struct {
    uint32_t magic;               // Protocol magic number
    uint32_t version;             // Protocol version
    dist_message_type_t type;     // Message type
    uint64_t message_id;          // Unique message ID
    uint64_t sender_id;           // Sender agent ID
    uint64_t receiver_id;         // Receiver agent ID (0 = broadcast)
    uint32_t payload_size;        // Payload size in bytes
    uint32_t compression_flags;   // Compression settings
    uint64_t timestamp;           // Message timestamp
    uint32_t hop_count;           // Routing hop count
    uint32_t priority;            // Message priority
    uint64_t correlation_id;      // For request/response correlation
    uint32_t checksum;            // Message integrity check
} dist_message_header_t;

// Message structure
typedef struct {
    dist_message_header_t header;
    void* payload;
    size_t payload_capacity;
    
    // Routing information
    uint64_t next_hop_agent_id;
    uint64_t route_path[DIST_COMM_MAX_ROUTING_HOPS];
    size_t route_length;
    
    // QoS information
    uint64_t send_time;
    uint64_t receive_time;
    float reliability_requirement;
    float latency_requirement;
} dist_message_t;

// Agent information
typedef struct {
    uint64_t agent_id;
    char hostname[256];
    uint16_t port;
    dist_agent_state_t state;
    dist_protocol_t preferred_protocol;
    
    // Capabilities
    bool supports_pln_reasoning;
    bool supports_pattern_matching;
    bool supports_tensor_operations;
    bool supports_coordination;
    
    // Performance metrics
    float average_response_time;
    float reliability_score;
    uint64_t message_count_sent;
    uint64_t message_count_received;
    uint64_t last_heartbeat_time;
    
    // Cognitive profile
    float attention_capacity;
    float reasoning_capacity;
    float memory_capacity;
    
    // Network information
    int socket_fd;
    uint32_t connection_flags;
    uint64_t last_activity_time;
} dist_agent_info_t;

// Connection management
typedef struct {
    uint64_t connection_id;
    uint64_t local_agent_id;
    uint64_t remote_agent_id;
    dist_protocol_t protocol;
    
    int socket_fd;
    bool is_active;
    bool is_reliable;
    
    // Statistics
    uint64_t bytes_sent;
    uint64_t bytes_received;
    uint64_t messages_sent;
    uint64_t messages_received;
    float average_latency;
    
    // Buffer management
    uint8_t* send_buffer;
    uint8_t* receive_buffer;
    size_t send_buffer_size;
    size_t receive_buffer_size;
    size_t send_buffer_used;
    size_t receive_buffer_used;
} dist_connection_t;

// Distributed communication engine
typedef struct {
    // Local agent information
    uint64_t local_agent_id;
    char local_hostname[256];
    uint16_t local_port;
    dist_agent_state_t local_state;
    
    // Network infrastructure
    int listen_socket_tcp;
    int listen_socket_udp;
    int discovery_socket;
    
    // Known agents registry
    dist_agent_info_t* agents;
    size_t agent_count;
    size_t agent_capacity;
    
    // Active connections
    dist_connection_t* connections;
    size_t connection_count;
    size_t connection_capacity;
    
    // Message queues
    dist_message_t** outgoing_queue;
    dist_message_t** incoming_queue;
    size_t outgoing_queue_size;
    size_t incoming_queue_size;
    size_t max_queue_size;
    
    // Routing table
    struct {
        uint64_t destination_agent_id;
        uint64_t next_hop_agent_id;
        uint32_t hop_count;
        float route_quality;
        uint64_t last_updated;
    } routing_table[DIST_COMM_MAX_AGENTS];
    size_t routing_table_size;
    
    // Integration components
    opencog_atomspace_t* atomspace;
    pln_reasoning_engine_t* pln_engine;
    pattern_engine_t* pattern_engine;
    ggml_cognitive_kernel_t* cognitive_kernel;
    struct ggml_context* ctx;
    
    // Engine state
    bool initialized;
    bool is_running;
    bool discovery_enabled;
    bool auto_connect;
    
    // Configuration
    uint32_t max_connections;
    uint32_t heartbeat_interval;
    uint32_t discovery_interval;
    uint32_t message_timeout;
    float connection_quality_threshold;
    
    // Statistics
    uint64_t total_messages_sent;
    uint64_t total_messages_received;
    uint64_t total_bytes_sent;
    uint64_t total_bytes_received;
    uint64_t connection_attempts;
    uint64_t successful_connections;
    
    // Threading (simplified representation)
    bool network_thread_running;
    bool discovery_thread_running;
    bool maintenance_thread_running;
} dist_comm_engine_t;

// Core distributed communication functions

// Initialize distributed communication engine
GGML_API dist_comm_engine_t* dist_comm_init(
    uint64_t agent_id,
    const char* hostname,
    uint16_t port,
    struct ggml_context* ctx);

// Free distributed communication engine
GGML_API void dist_comm_free(dist_comm_engine_t* engine);

// Start communication services
GGML_API bool dist_comm_start(dist_comm_engine_t* engine);

// Stop communication services
GGML_API bool dist_comm_stop(dist_comm_engine_t* engine);

// Agent management

// Register agent capabilities
GGML_API bool dist_comm_register_capabilities(
    dist_comm_engine_t* engine,
    bool supports_pln,
    bool supports_patterns,
    bool supports_tensors,
    bool supports_coordination);

// Discover agents on network
GGML_API bool dist_comm_discover_agents(
    dist_comm_engine_t* engine,
    const char* multicast_address);

// Connect to remote agent
GGML_API bool dist_comm_connect_agent(
    dist_comm_engine_t* engine,
    uint64_t remote_agent_id,
    const char* hostname,
    uint16_t port,
    dist_protocol_t protocol);

// Disconnect from agent
GGML_API bool dist_comm_disconnect_agent(
    dist_comm_engine_t* engine,
    uint64_t remote_agent_id);

// Message handling

// Create message
GGML_API dist_message_t* dist_comm_create_message(
    dist_message_type_t type,
    uint64_t sender_id,
    uint64_t receiver_id,
    const void* payload,
    size_t payload_size);

// Free message
GGML_API void dist_comm_free_message(dist_message_t* message);

// Send message
GGML_API bool dist_comm_send_message(
    dist_comm_engine_t* engine,
    dist_message_t* message);

// Send broadcast message
GGML_API bool dist_comm_broadcast_message(
    dist_comm_engine_t* engine,
    dist_message_t* message);

// Receive message (non-blocking)
GGML_API dist_message_t* dist_comm_receive_message(
    dist_comm_engine_t* engine);

// Send cognitive tensor
GGML_API bool dist_comm_send_cognitive_tensor(
    dist_comm_engine_t* engine,
    uint64_t receiver_id,
    struct ggml_tensor* tensor,
    uint32_t attention_level);

// Send PLN inference request
GGML_API bool dist_comm_send_pln_request(
    dist_comm_engine_t* engine,
    uint64_t receiver_id,
    pln_inference_context_t* inference_context);

// Send pattern matching request
GGML_API bool dist_comm_send_pattern_request(
    dist_comm_engine_t* engine,
    uint64_t receiver_id,
    pattern_template_t* template);

// High-level cognitive communication

// Request distributed reasoning
GGML_API bool dist_comm_request_reasoning(
    dist_comm_engine_t* engine,
    uint64_t* target_agents,
    size_t agent_count,
    const char* reasoning_task,
    void** results,
    size_t* result_count);

// Coordinate multi-agent task
GGML_API bool dist_comm_coordinate_task(
    dist_comm_engine_t* engine,
    uint64_t* participant_agents,
    size_t participant_count,
    const char* task_description,
    void* task_parameters);

// Synchronize knowledge bases
GGML_API bool dist_comm_sync_knowledge(
    dist_comm_engine_t* engine,
    uint64_t remote_agent_id,
    bool bidirectional);

// Achieve consensus on belief
GGML_API bool dist_comm_achieve_consensus(
    dist_comm_engine_t* engine,
    uint64_t* participant_agents,
    size_t participant_count,
    const char* belief_topic,
    opencog_truth_value_t* consensus_truth_value);

// Routing and topology

// Update routing table
GGML_API bool dist_comm_update_routing(dist_comm_engine_t* engine);

// Find route to agent
GGML_API uint64_t* dist_comm_find_route(
    dist_comm_engine_t* engine,
    uint64_t destination_agent_id,
    size_t* route_length);

// Get network topology
GGML_API bool dist_comm_get_topology(
    dist_comm_engine_t* engine,
    uint64_t** agent_ids,
    size_t* agent_count);

// Quality of Service

// Set message priority
GGML_API bool dist_comm_set_message_priority(
    dist_message_t* message,
    uint32_t priority);

// Set reliability requirements
GGML_API bool dist_comm_set_reliability(
    dist_message_t* message,
    float reliability_requirement);

// Set latency requirements
GGML_API bool dist_comm_set_latency(
    dist_message_t* message,
    float max_latency_ms);

// Statistics and monitoring

// Get agent statistics
GGML_API bool dist_comm_get_agent_stats(
    dist_comm_engine_t* engine,
    uint64_t agent_id,
    dist_agent_info_t* stats);

// Get connection statistics
GGML_API bool dist_comm_get_connection_stats(
    dist_comm_engine_t* engine,
    uint64_t connection_id,
    dist_connection_t* stats);

// Get network performance metrics
GGML_API bool dist_comm_get_network_metrics(
    dist_comm_engine_t* engine,
    float* average_latency,
    float* message_loss_rate,
    float* bandwidth_utilization);

// Configuration and control

// Set discovery parameters
GGML_API bool dist_comm_configure_discovery(
    dist_comm_engine_t* engine,
    bool enable_discovery,
    uint32_t discovery_interval,
    const char* multicast_group);

// Set connection parameters
GGML_API bool dist_comm_configure_connections(
    dist_comm_engine_t* engine,
    uint32_t max_connections,
    uint32_t heartbeat_interval,
    float quality_threshold);

// Set message queue parameters
GGML_API bool dist_comm_configure_queues(
    dist_comm_engine_t* engine,
    size_t max_queue_size,
    uint32_t message_timeout);

// Integration functions

// Connect PLN reasoning engine
GGML_API bool dist_comm_connect_pln_engine(
    dist_comm_engine_t* engine,
    pln_reasoning_engine_t* pln_engine);

// Connect pattern matching engine
GGML_API bool dist_comm_connect_pattern_engine(
    dist_comm_engine_t* engine,
    pattern_engine_t* pattern_engine);

// Connect cognitive kernel
GGML_API bool dist_comm_connect_cognitive_kernel(
    dist_comm_engine_t* engine,
    ggml_cognitive_kernel_t* cognitive_kernel);

// Utility functions

// Print engine status
GGML_API void dist_comm_print_status(dist_comm_engine_t* engine);

// Print agent list
GGML_API void dist_comm_print_agents(dist_comm_engine_t* engine);

// Print connection list
GGML_API void dist_comm_print_connections(dist_comm_engine_t* engine);

// Print routing table
GGML_API void dist_comm_print_routing_table(dist_comm_engine_t* engine);

// Export network topology
GGML_API bool dist_comm_export_topology(
    dist_comm_engine_t* engine,
    const char* filename);

// Debugging functions

// Enable debug mode
GGML_API void dist_comm_enable_debug(
    dist_comm_engine_t* engine,
    bool enable);

// Send debug message
GGML_API bool dist_comm_send_debug_message(
    dist_comm_engine_t* engine,
    uint64_t receiver_id,
    const char* debug_info);

// Trace message path
GGML_API bool dist_comm_trace_message(
    dist_comm_engine_t* engine,
    uint64_t message_id);

#ifdef __cplusplus
}
#endif