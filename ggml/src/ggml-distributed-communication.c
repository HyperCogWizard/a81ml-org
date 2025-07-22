#include "ggml-distributed-communication.h"
#include "ggml-rpc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#ifdef _WIN32
#  define WIN32_LEAN_AND_MEAN
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <arpa/inet.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <netdb.h>
#  include <unistd.h>
#endif

// Message header for all distributed communication
typedef struct {
    agent_message_type_t msg_type;
    uint32_t msg_id;
    uint32_t from_agent_id;
    uint32_t to_agent_id;
    uint32_t data_size;
    uint32_t timestamp;
} message_header_t;

// Generate unique message ID
static uint32_t generate_message_id(void) {
    static uint32_t counter = 1;
    return counter++;
}

// Parse endpoint string into host and port
static bool parse_endpoint(const char* endpoint, char* host, int* port) {
    char* colon = strchr(endpoint, ':');
    if (!colon) return false;
    
    size_t host_len = colon - endpoint;
    if (host_len >= 256) return false;
    
    strncpy(host, endpoint, host_len);
    host[host_len] = '\0';
    *port = atoi(colon + 1);
    
    return *port > 0 && *port < 65536;
}

// Initialize distributed communication manager
distributed_communication_manager_t* distributed_comm_init(
    struct ggml_context* ctx,
    const char* local_endpoint,
    uint32_t agent_id) {
    
    distributed_communication_manager_t* comm = malloc(sizeof(distributed_communication_manager_t));
    if (!comm) return NULL;
    
    comm->ctx = ctx;
    comm->rpc_backend = NULL;
    strncpy(comm->local_endpoint, local_endpoint, sizeof(comm->local_endpoint) - 1);
    comm->local_endpoint[sizeof(comm->local_endpoint) - 1] = '\0';
    comm->local_agent_id = agent_id;
    
    // Initialize agent registry
    comm->agent_capacity = DISTRIBUTED_COMM_MAX_AGENTS;
    comm->known_agents = calloc(comm->agent_capacity, sizeof(distributed_agent_info_t));
    comm->agent_count = 0;
    
    // Initialize network topology
    comm->network_capacity = DISTRIBUTED_COMM_MAX_NETWORKS;
    comm->networks = calloc(comm->network_capacity, sizeof(cognitive_network_t));
    comm->network_count = 0;
    
    // Initialize communication state
    comm->server_active = false;
    comm->discovery_active = false;
    comm->message_sequence = 0;
    comm->total_bytes_sent = 0;
    comm->total_bytes_received = 0;
    
    // Initialize performance metrics
    comm->successful_connections = 0;
    comm->failed_connections = 0;
    comm->network_latency_avg = 0.0f;
    comm->message_success_rate = 1.0f;
    
    printf("Distributed Communication Manager initialized for agent %u at %s\n",
           agent_id, local_endpoint);
    
    return comm;
}

// Free distributed communication manager
void distributed_comm_free(distributed_communication_manager_t* comm) {
    if (!comm) return;
    
    // Free agent registry
    free(comm->known_agents);
    
    // Free networks
    for (size_t i = 0; i < comm->network_count; i++) {
        if (comm->networks[i].agents) {
            free(comm->networks[i].agents);
        }
    }
    free(comm->networks);
    
    free(comm);
}

// Start communication server
bool distributed_comm_start_server(distributed_communication_manager_t* comm) {
    if (!comm) return false;
    
    char host[256];
    int port;
    if (!parse_endpoint(comm->local_endpoint, host, &port)) {
        printf("Failed to parse endpoint: %s\n", comm->local_endpoint);
        return false;
    }
    
    // Initialize RPC backend for networking (if available)
    comm->rpc_backend = NULL;  // For now, don't use RPC to avoid linking issues
    /*
    comm->rpc_backend = ggml_backend_rpc_init(comm->local_endpoint);
    if (!comm->rpc_backend) {
        printf("Failed to initialize RPC backend for %s\n", comm->local_endpoint);
        return false;
    }
    */
    
    comm->server_active = true;
    printf("Distributed communication server started at %s\n", comm->local_endpoint);
    return true;
}

// Register remote agent
bool distributed_comm_register_agent(
    distributed_communication_manager_t* comm,
    const char* remote_endpoint,
    const char* agent_name) {
    
    if (!comm || comm->agent_count >= comm->agent_capacity) return false;
    
    // Generate unique agent ID based on endpoint
    uint32_t remote_agent_id = 0;
    for (const char* p = remote_endpoint; *p; p++) {
        remote_agent_id = remote_agent_id * 31 + *p;
    }
    
    // Check if agent already registered
    for (size_t i = 0; i < comm->agent_count; i++) {
        if (comm->known_agents[i].agent_id == remote_agent_id) {
            printf("Agent %u already registered\n", remote_agent_id);
            return true;
        }
    }
    
    // Add new agent
    distributed_agent_info_t* agent = &comm->known_agents[comm->agent_count];
    agent->agent_id = remote_agent_id;
    strncpy(agent->endpoint, remote_endpoint, sizeof(agent->endpoint) - 1);
    agent->endpoint[sizeof(agent->endpoint) - 1] = '\0';
    strncpy(agent->agent_name, agent_name, sizeof(agent->agent_name) - 1);
    agent->agent_name[sizeof(agent->agent_name) - 1] = '\0';
    agent->capabilities = 0;
    agent->last_heartbeat = (uint32_t)time(NULL);
    agent->active = true;
    agent->cognitive_coherence = 0.5f;
    agent->message_count = 0;
    
    comm->agent_count++;
    comm->successful_connections++;
    
    printf("Registered agent %u (%s) at %s\n", remote_agent_id, agent_name, remote_endpoint);
    return true;
}

// Find agent by ID
distributed_agent_info_t* distributed_comm_find_agent(
    distributed_communication_manager_t* comm,
    uint32_t agent_id) {
    
    if (!comm) return NULL;
    
    for (size_t i = 0; i < comm->agent_count; i++) {
        if (comm->known_agents[i].agent_id == agent_id) {
            return &comm->known_agents[i];
        }
    }
    return NULL;
}

// Send cognitive state to specific agent
bool distributed_comm_send_cognitive_state(
    distributed_communication_manager_t* comm,
    uint32_t target_agent_id,
    distributed_cognitive_architecture_t* arch) {
    
    if (!comm || !arch) return false;
    
    distributed_agent_info_t* target = distributed_comm_find_agent(comm, target_agent_id);
    if (!target || !target->active) return false;
    
    // Create cognitive state packet
    cognitive_state_packet_t state_packet = {0};
    state_packet.agent_id = comm->local_agent_id;
    state_packet.timestamp = (uint32_t)time(NULL);
    state_packet.coherence_level = dashboard_compute_coherence(arch);
    
    // Copy attention values from dashboard
    if (arch->dashboard) {
        memcpy(state_packet.attention_values, arch->dashboard->attention_distribution, 
               sizeof(state_packet.attention_values));
        state_packet.active_workflows = arch->dashboard->active_workflows;
        state_packet.cognitive_load = arch->dashboard->cognitive_load;
    }
    
    strncpy(state_packet.endpoint, comm->local_endpoint, sizeof(state_packet.endpoint) - 1);
    
    // Create message header
    message_header_t header = {0};
    header.msg_type = AGENT_MSG_COGNITIVE_STATE;
    header.msg_id = generate_message_id();
    header.from_agent_id = comm->local_agent_id;
    header.to_agent_id = target_agent_id;
    header.data_size = sizeof(cognitive_state_packet_t);
    header.timestamp = (uint32_t)time(NULL);
    
    // In a real implementation, this would use the RPC system to send the data
    // For now, we'll simulate the sending
    printf("Sending cognitive state from agent %u to agent %u (coherence: %.3f)\n",
           comm->local_agent_id, target_agent_id, state_packet.coherence_level);
    
    comm->total_bytes_sent += sizeof(header) + sizeof(state_packet);
    target->message_count++;
    
    return true;
}

// Broadcast cognitive state to all known agents
bool distributed_comm_broadcast_cognitive_state(
    distributed_communication_manager_t* comm,
    distributed_cognitive_architecture_t* arch) {
    
    if (!comm || !arch) return false;
    
    bool success = true;
    for (size_t i = 0; i < comm->agent_count; i++) {
        if (comm->known_agents[i].active && 
            comm->known_agents[i].agent_id != comm->local_agent_id) {
            bool result = distributed_comm_send_cognitive_state(
                comm, comm->known_agents[i].agent_id, arch);
            success = success && result;
        }
    }
    
    return success;
}

// Send attention update to target agent
bool distributed_comm_send_attention_update(
    distributed_communication_manager_t* comm,
    uint32_t target_agent_id,
    float attention_weights[4],
    float salience,
    uint32_t membrane_id,
    uint64_t workflow_id) {
    
    if (!comm) return false;
    
    distributed_agent_info_t* target = distributed_comm_find_agent(comm, target_agent_id);
    if (!target || !target->active) return false;
    
    // Create attention update packet
    attention_update_packet_t update_packet = {0};
    update_packet.agent_id = comm->local_agent_id;
    update_packet.target_agent_id = target_agent_id;
    memcpy(update_packet.attention_allocation, attention_weights, sizeof(update_packet.attention_allocation));
    update_packet.salience_score = salience;
    update_packet.membrane_id = membrane_id;
    update_packet.workflow_id = workflow_id;
    
    printf("Sending attention update from agent %u to agent %u (salience: %.3f)\n",
           comm->local_agent_id, target_agent_id, salience);
    
    comm->total_bytes_sent += sizeof(attention_update_packet_t);
    target->message_count++;
    
    return true;
}

// Send tensor to target agent
bool distributed_comm_send_tensor(
    distributed_communication_manager_t* comm,
    uint32_t target_agent_id,
    struct ggml_tensor* tensor,
    const char* tensor_name) {
    
    if (!comm || !tensor || !tensor_name) return false;
    
    distributed_agent_info_t* target = distributed_comm_find_agent(comm, target_agent_id);
    if (!target || !target->active) return false;
    
    // Create tensor exchange packet
    tensor_exchange_packet_t exchange_packet = {0};
    exchange_packet.agent_id = comm->local_agent_id;
    exchange_packet.target_agent_id = target_agent_id;
    exchange_packet.tensor_id = (uint64_t)tensor;  // Use pointer as ID for now
    
    // Copy tensor dimensions
    for (int i = 0; i < 4 && i < GGML_MAX_DIMS; i++) {
        exchange_packet.tensor_dims[i] = (uint32_t)tensor->ne[i];
    }
    
    exchange_packet.tensor_type = (uint32_t)tensor->type;
    exchange_packet.data_size = (uint32_t)ggml_nbytes(tensor);
    strncpy(exchange_packet.tensor_name, tensor_name, sizeof(exchange_packet.tensor_name) - 1);
    
    printf("Sending tensor '%s' from agent %u to agent %u (size: %u bytes)\n",
           tensor_name, comm->local_agent_id, target_agent_id, exchange_packet.data_size);
    
    comm->total_bytes_sent += sizeof(exchange_packet) + exchange_packet.data_size;
    target->message_count++;
    
    return true;
}

// Send heartbeat to target agent
bool distributed_comm_send_heartbeat(
    distributed_communication_manager_t* comm,
    uint32_t target_agent_id) {
    
    if (!comm) return false;
    
    distributed_agent_info_t* target = distributed_comm_find_agent(comm, target_agent_id);
    if (!target) return false;
    
    // Create message header for heartbeat
    message_header_t header = {0};
    header.msg_type = AGENT_MSG_HEARTBEAT;
    header.msg_id = generate_message_id();
    header.from_agent_id = comm->local_agent_id;
    header.to_agent_id = target_agent_id;
    header.data_size = 0;
    header.timestamp = (uint32_t)time(NULL);
    
    printf("Sending heartbeat from agent %u to agent %u\n",
           comm->local_agent_id, target_agent_id);
    
    comm->total_bytes_sent += sizeof(header);
    target->last_heartbeat = header.timestamp;
    
    return true;
}

// Update heartbeats for all agents
void distributed_comm_update_heartbeats(distributed_communication_manager_t* comm) {
    if (!comm) return;
    
    uint32_t current_time = (uint32_t)time(NULL);
    
    for (size_t i = 0; i < comm->agent_count; i++) {
        if (comm->known_agents[i].active) {
            distributed_comm_send_heartbeat(comm, comm->known_agents[i].agent_id);
        }
        
        // Mark agents as inactive if no heartbeat for 60 seconds
        if (current_time - comm->known_agents[i].last_heartbeat > 60) {
            comm->known_agents[i].active = false;
            printf("Agent %u marked as inactive (no heartbeat)\n", 
                   comm->known_agents[i].agent_id);
        }
    }
}

// Create new cognitive network
uint32_t distributed_comm_create_network(
    distributed_communication_manager_t* comm,
    const char* network_name) {
    
    if (!comm || comm->network_count >= comm->network_capacity) return 0;
    
    cognitive_network_t* network = &comm->networks[comm->network_count];
    network->network_id = comm->network_count + 1;
    strncpy(network->network_name, network_name, sizeof(network->network_name) - 1);
    network->network_name[sizeof(network->network_name) - 1] = '\0';
    
    network->agent_capacity = 32;
    network->agents = calloc(network->agent_capacity, sizeof(distributed_agent_info_t));
    network->agent_count = 0;
    network->network_coherence = 0.0f;
    network->total_messages = 0;
    
    comm->network_count++;
    
    printf("Created cognitive network %u: %s\n", network->network_id, network_name);
    return network->network_id;
}

// Compute network coherence
float distributed_comm_compute_network_coherence(
    distributed_communication_manager_t* comm,
    uint32_t network_id) {
    
    if (!comm || network_id == 0 || network_id > comm->network_count) return 0.0f;
    
    cognitive_network_t* network = &comm->networks[network_id - 1];
    if (network->agent_count == 0) return 0.0f;
    
    float total_coherence = 0.0f;
    size_t active_agents = 0;
    
    for (size_t i = 0; i < network->agent_count; i++) {
        if (network->agents[i].active) {
            total_coherence += network->agents[i].cognitive_coherence;
            active_agents++;
        }
    }
    
    float coherence = active_agents > 0 ? total_coherence / active_agents : 0.0f;
    network->network_coherence = coherence;
    
    return coherence;
}

// Print communication statistics
void distributed_comm_print_statistics(distributed_communication_manager_t* comm) {
    if (!comm) return;
    
    printf("\n=== Distributed Communication Statistics ===\n");
    printf("Local agent ID: %u\n", comm->local_agent_id);
    printf("Local endpoint: %s\n", comm->local_endpoint);
    printf("Server active: %s\n", comm->server_active ? "Yes" : "No");
    printf("Known agents: %zu / %zu\n", comm->agent_count, comm->agent_capacity);
    printf("Active networks: %zu / %zu\n", comm->network_count, comm->network_capacity);
    printf("Successful connections: %u\n", comm->successful_connections);
    printf("Failed connections: %u\n", comm->failed_connections);
    printf("Total bytes sent: %llu\n", (unsigned long long)comm->total_bytes_sent);
    printf("Total bytes received: %llu\n", (unsigned long long)comm->total_bytes_received);
    printf("Average network latency: %.3f ms\n", comm->network_latency_avg);
    printf("Message success rate: %.1f%%\n", comm->message_success_rate * 100.0f);
    
    printf("\nRegistered Agents:\n");
    for (size_t i = 0; i < comm->agent_count; i++) {
        distributed_agent_info_t* agent = &comm->known_agents[i];
        printf("  Agent %u (%s): %s - %s (coherence: %.3f, messages: %u)\n",
               agent->agent_id, agent->agent_name, agent->endpoint,
               agent->active ? "ACTIVE" : "INACTIVE",
               agent->cognitive_coherence, agent->message_count);
    }
    
    printf("\nCognitive Networks:\n");
    for (size_t i = 0; i < comm->network_count; i++) {
        cognitive_network_t* network = &comm->networks[i];
        printf("  Network %u (%s): %zu agents, coherence: %.3f, messages: %llu\n",
               network->network_id, network->network_name, network->agent_count,
               network->network_coherence, (unsigned long long)network->total_messages);
    }
    
    printf("============================================\n");
}

// Request workflow participation from target agent  
bool distributed_comm_request_workflow_participation(
    distributed_communication_manager_t* comm,
    uint32_t target_agent_id,
    uint64_t workflow_id,
    const char* workflow_description) {
    
    if (!comm || !workflow_description) return false;
    
    distributed_agent_info_t* target = distributed_comm_find_agent(comm, target_agent_id);
    if (!target || !target->active) return false;
    
    // Create message header for workflow request
    message_header_t header = {0};
    header.msg_type = AGENT_MSG_WORKFLOW_REQUEST;
    header.msg_id = generate_message_id();
    header.from_agent_id = comm->local_agent_id;
    header.to_agent_id = target_agent_id;
    header.data_size = strlen(workflow_description) + 1;
    header.timestamp = (uint32_t)time(NULL);
    
    printf("Requesting workflow participation from agent %u for workflow %llu: %s\n",
           target_agent_id, (unsigned long long)workflow_id, workflow_description);
    
    comm->total_bytes_sent += sizeof(header) + header.data_size;
    target->message_count++;
    
    return true;
}

// Run connectivity test
bool distributed_comm_run_connectivity_test(distributed_communication_manager_t* comm) {
    if (!comm) return false;
    
    printf("\n=== Connectivity Test ===\n");
    printf("Testing connectivity to %zu registered agents...\n", comm->agent_count);
    
    bool all_connected = true;
    for (size_t i = 0; i < comm->agent_count; i++) {
        distributed_agent_info_t* agent = &comm->known_agents[i];
        if (agent->agent_id != comm->local_agent_id) {
            // In a real implementation, this would test actual network connectivity
            bool connected = agent->active && (time(NULL) - agent->last_heartbeat < 30);
            printf("  Agent %u (%s): %s\n", 
                   agent->agent_id, agent->endpoint, 
                   connected ? "CONNECTED" : "DISCONNECTED");
            all_connected = all_connected && connected;
        }
    }
    
    printf("Connectivity test %s\n", all_connected ? "PASSED" : "FAILED");
    return all_connected;
}