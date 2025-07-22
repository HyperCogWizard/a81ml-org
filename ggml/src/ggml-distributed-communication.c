#include "ggml-distributed-communication.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#endif

// Protocol magic number
#define DIST_COMM_MAGIC 0x434F474E  // 'COGN'
#define DIST_COMM_VERSION 1

// Initialize distributed communication engine
dist_comm_engine_t* dist_comm_init(
    uint64_t agent_id,
    const char* hostname,
    uint16_t port,
    struct ggml_context* ctx) {
    
    if (!hostname || !ctx) return NULL;
    
    dist_comm_engine_t* engine = malloc(sizeof(dist_comm_engine_t));
    if (!engine) return NULL;
    
    memset(engine, 0, sizeof(dist_comm_engine_t));
    
    // Initialize basic information
    engine->local_agent_id = agent_id;
    strncpy(engine->local_hostname, hostname, sizeof(engine->local_hostname) - 1);
    engine->local_port = port;
    engine->local_state = DIST_AGENT_CONNECTING;
    engine->ctx = ctx;
    
    // Initialize agent registry
    engine->agent_capacity = DIST_COMM_MAX_AGENTS;
    engine->agents = calloc(engine->agent_capacity, sizeof(dist_agent_info_t));
    if (!engine->agents) {
        free(engine);
        return NULL;
    }
    
    // Initialize connection management
    engine->connection_capacity = DIST_COMM_MAX_CONNECTIONS;
    engine->connections = calloc(engine->connection_capacity, sizeof(dist_connection_t));
    if (!engine->connections) {
        free(engine->agents);
        free(engine);
        return NULL;
    }
    
    // Initialize message queues
    engine->max_queue_size = 1000;
    engine->outgoing_queue = calloc(engine->max_queue_size, sizeof(dist_message_t*));
    engine->incoming_queue = calloc(engine->max_queue_size, sizeof(dist_message_t*));
    if (!engine->outgoing_queue || !engine->incoming_queue) {
        free(engine->connections);
        free(engine->agents);
        if (engine->outgoing_queue) free(engine->outgoing_queue);
        if (engine->incoming_queue) free(engine->incoming_queue);
        free(engine);
        return NULL;
    }
    
    // Initialize sockets to invalid values
    engine->listen_socket_tcp = -1;
    engine->listen_socket_udp = -1;
    engine->discovery_socket = -1;
    
    // Set default configuration
    engine->max_connections = 64;
    engine->heartbeat_interval = DIST_COMM_HEARTBEAT_INTERVAL;
    engine->discovery_interval = 60; // seconds
    engine->message_timeout = 30000; // 30 seconds in ms
    engine->connection_quality_threshold = 0.7f;
    engine->discovery_enabled = true;
    engine->auto_connect = true;
    
    engine->initialized = true;
    
    printf("Distributed communication engine initialized for agent %lu at %s:%u\n",
           agent_id, hostname, port);
    
    return engine;
}

// Free distributed communication engine
void dist_comm_free(dist_comm_engine_t* engine) {
    if (!engine) return;
    
    // Stop services if running
    if (engine->is_running) {
        dist_comm_stop(engine);
    }
    
    // Free message queues
    if (engine->outgoing_queue) {
        for (size_t i = 0; i < engine->outgoing_queue_size; i++) {
            if (engine->outgoing_queue[i]) {
                dist_comm_free_message(engine->outgoing_queue[i]);
            }
        }
        free(engine->outgoing_queue);
    }
    
    if (engine->incoming_queue) {
        for (size_t i = 0; i < engine->incoming_queue_size; i++) {
            if (engine->incoming_queue[i]) {
                dist_comm_free_message(engine->incoming_queue[i]);
            }
        }
        free(engine->incoming_queue);
    }
    
    // Free connections
    if (engine->connections) {
        for (size_t i = 0; i < engine->connection_count; i++) {
            dist_connection_t* conn = &engine->connections[i];
            if (conn->socket_fd >= 0) {
#ifdef _WIN32
                closesocket(conn->socket_fd);
#else
                close(conn->socket_fd);
#endif
            }
            if (conn->send_buffer) free(conn->send_buffer);
            if (conn->receive_buffer) free(conn->receive_buffer);
        }
        free(engine->connections);
    }
    
    // Free agents
    if (engine->agents) {
        free(engine->agents);
    }
    
    // Close sockets
    if (engine->listen_socket_tcp >= 0) {
#ifdef _WIN32
        closesocket(engine->listen_socket_tcp);
#else
        close(engine->listen_socket_tcp);
#endif
    }
    
    if (engine->listen_socket_udp >= 0) {
#ifdef _WIN32
        closesocket(engine->listen_socket_udp);
#else
        close(engine->listen_socket_udp);
#endif
    }
    
    if (engine->discovery_socket >= 0) {
#ifdef _WIN32
        closesocket(engine->discovery_socket);
#else
        close(engine->discovery_socket);
#endif
    }
    
#ifdef _WIN32
    WSACleanup();
#endif
    
    free(engine);
}

// Initialize networking (platform-specific)
static bool init_networking() {
#ifdef _WIN32
    WSADATA wsaData;
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
    return true; // Unix-like systems don't need special initialization
#endif
}

// Create and bind TCP socket
static int create_tcp_socket(const char* hostname, uint16_t port) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;
    
    // Set socket options
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    
    // Bind socket
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (strcmp(hostname, "localhost") == 0) {
        addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        if (inet_pton(AF_INET, hostname, &addr.sin_addr) <= 0) {
            // Try hostname resolution
            struct hostent* he = gethostbyname(hostname);
            if (!he) {
#ifdef _WIN32
                closesocket(sock);
#else
                close(sock);
#endif
                return -1;
            }
            memcpy(&addr.sin_addr, he->h_addr_list[0], he->h_length);
        }
    }
    
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        return -1;
    }
    
    return sock;
}

// Create and bind UDP socket
static int create_udp_socket(const char* hostname, uint16_t port) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return -1;
    
    // Set socket options
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof(opt));
    
    // Bind socket
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
#ifdef _WIN32
        closesocket(sock);
#else
        close(sock);
#endif
        return -1;
    }
    
    return sock;
}

// Start communication services
bool dist_comm_start(dist_comm_engine_t* engine) {
    if (!engine || engine->is_running) return false;
    
    if (!init_networking()) {
        printf("Failed to initialize networking\n");
        return false;
    }
    
    // Create TCP listening socket
    engine->listen_socket_tcp = create_tcp_socket(engine->local_hostname, engine->local_port);
    if (engine->listen_socket_tcp < 0) {
        printf("Failed to create TCP socket on %s:%u\n", 
               engine->local_hostname, engine->local_port);
        return false;
    }
    
    // Start listening
    if (listen(engine->listen_socket_tcp, 10) < 0) {
        printf("Failed to listen on TCP socket\n");
#ifdef _WIN32
        closesocket(engine->listen_socket_tcp);
#else
        close(engine->listen_socket_tcp);
#endif
        engine->listen_socket_tcp = -1;
        return false;
    }
    
    // Create UDP socket
    engine->listen_socket_udp = create_udp_socket(engine->local_hostname, engine->local_port + 1);
    if (engine->listen_socket_udp < 0) {
        printf("Warning: Failed to create UDP socket\n");
        // Continue without UDP - not critical
    }
    
    // Create discovery socket
    if (engine->discovery_enabled) {
        engine->discovery_socket = create_udp_socket("0.0.0.0", DIST_COMM_DISCOVERY_PORT);
        if (engine->discovery_socket < 0) {
            printf("Warning: Failed to create discovery socket\n");
        }
    }
    
    engine->is_running = true;
    engine->local_state = DIST_AGENT_ACTIVE;
    
    printf("Communication services started for agent %lu on %s:%u\n",
           engine->local_agent_id, engine->local_hostname, engine->local_port);
    
    return true;
}

// Stop communication services
bool dist_comm_stop(dist_comm_engine_t* engine) {
    if (!engine || !engine->is_running) return false;
    
    engine->is_running = false;
    engine->local_state = DIST_AGENT_DISCONNECTED;
    
    // Close all connections
    for (size_t i = 0; i < engine->connection_count; i++) {
        dist_connection_t* conn = &engine->connections[i];
        if (conn->socket_fd >= 0) {
#ifdef _WIN32
            closesocket(conn->socket_fd);
#else
            close(conn->socket_fd);
#endif
            conn->socket_fd = -1;
            conn->is_active = false;
        }
    }
    
    // Close listening sockets
    if (engine->listen_socket_tcp >= 0) {
#ifdef _WIN32
        closesocket(engine->listen_socket_tcp);
#else
        close(engine->listen_socket_tcp);
#endif
        engine->listen_socket_tcp = -1;
    }
    
    if (engine->listen_socket_udp >= 0) {
#ifdef _WIN32
        closesocket(engine->listen_socket_udp);
#else
        close(engine->listen_socket_udp);
#endif
        engine->listen_socket_udp = -1;
    }
    
    if (engine->discovery_socket >= 0) {
#ifdef _WIN32
        closesocket(engine->discovery_socket);
#else
        close(engine->discovery_socket);
#endif
        engine->discovery_socket = -1;
    }
    
    printf("Communication services stopped for agent %lu\n", engine->local_agent_id);
    
    return true;
}

// Register agent capabilities
bool dist_comm_register_capabilities(
    dist_comm_engine_t* engine,
    bool supports_pln,
    bool supports_patterns,
    bool supports_tensors,
    bool supports_coordination) {
    
    if (!engine) return false;
    
    // Find or create local agent entry
    dist_agent_info_t* local_agent = NULL;
    for (size_t i = 0; i < engine->agent_count; i++) {
        if (engine->agents[i].agent_id == engine->local_agent_id) {
            local_agent = &engine->agents[i];
            break;
        }
    }
    
    if (!local_agent && engine->agent_count < engine->agent_capacity) {
        local_agent = &engine->agents[engine->agent_count++];
        local_agent->agent_id = engine->local_agent_id;
        strncpy(local_agent->hostname, engine->local_hostname, sizeof(local_agent->hostname) - 1);
        local_agent->port = engine->local_port;
        local_agent->state = engine->local_state;
        local_agent->preferred_protocol = DIST_PROTOCOL_TCP;
    }
    
    if (local_agent) {
        local_agent->supports_pln_reasoning = supports_pln;
        local_agent->supports_pattern_matching = supports_patterns;
        local_agent->supports_tensor_operations = supports_tensors;
        local_agent->supports_coordination = supports_coordination;
        
        // Set default cognitive profile
        local_agent->attention_capacity = 1.0f;
        local_agent->reasoning_capacity = 1.0f;
        local_agent->memory_capacity = 1.0f;
        
        printf("Registered capabilities for agent %lu: PLN=%d, Patterns=%d, Tensors=%d, Coordination=%d\n",
               engine->local_agent_id, supports_pln, supports_patterns, supports_tensors, supports_coordination);
        
        return true;
    }
    
    return false;
}

// Create message
dist_message_t* dist_comm_create_message(
    dist_message_type_t type,
    uint64_t sender_id,
    uint64_t receiver_id,
    const void* payload,
    size_t payload_size) {
    
    dist_message_t* message = malloc(sizeof(dist_message_t));
    if (!message) return NULL;
    
    memset(message, 0, sizeof(dist_message_t));
    
    // Initialize header
    message->header.magic = DIST_COMM_MAGIC;
    message->header.version = DIST_COMM_VERSION;
    message->header.type = type;
    message->header.message_id = (uint64_t)time(NULL) * 1000000 + rand() % 1000000; // Simple ID generation
    message->header.sender_id = sender_id;
    message->header.receiver_id = receiver_id;
    message->header.payload_size = (uint32_t)payload_size;
    message->header.timestamp = (uint64_t)time(NULL);
    message->header.hop_count = 0;
    message->header.priority = 5; // Medium priority
    message->header.correlation_id = 0;
    
    // Copy payload
    if (payload && payload_size > 0) {
        message->payload_capacity = payload_size;
        message->payload = malloc(payload_size);
        if (message->payload) {
            memcpy(message->payload, payload, payload_size);
        } else {
            free(message);
            return NULL;
        }
    }
    
    // Initialize QoS parameters
    message->reliability_requirement = 0.9f;
    message->latency_requirement = 1000.0f; // 1 second default
    message->send_time = (uint64_t)time(NULL);
    
    return message;
}

// Free message
void dist_comm_free_message(dist_message_t* message) {
    if (!message) return;
    
    if (message->payload) {
        free(message->payload);
    }
    
    free(message);
}

// Send message (simplified implementation)
bool dist_comm_send_message(
    dist_comm_engine_t* engine,
    dist_message_t* message) {
    
    if (!engine || !message || !engine->is_running) return false;
    
    // Find connection to target agent
    dist_connection_t* connection = NULL;
    for (size_t i = 0; i < engine->connection_count; i++) {
        if (engine->connections[i].remote_agent_id == message->header.receiver_id &&
            engine->connections[i].is_active) {
            connection = &engine->connections[i];
            break;
        }
    }
    
    if (!connection) {
        // Try to establish connection (simplified)
        printf("No active connection to agent %lu - message queued\n", message->header.receiver_id);
        
        // Add to outgoing queue
        if (engine->outgoing_queue_size < engine->max_queue_size) {
            engine->outgoing_queue[engine->outgoing_queue_size++] = message;
            return true;
        }
        
        return false;
    }
    
    // Simulate successful send
    connection->messages_sent++;
    connection->bytes_sent += sizeof(dist_message_header_t) + message->header.payload_size;
    
    engine->total_messages_sent++;
    engine->total_bytes_sent += sizeof(dist_message_header_t) + message->header.payload_size;
    
    printf("Sent message type %d from agent %lu to agent %lu (size: %u bytes)\n",
           message->header.type, message->header.sender_id, 
           message->header.receiver_id, message->header.payload_size);
    
    return true;
}

// Send broadcast message
bool dist_comm_broadcast_message(
    dist_comm_engine_t* engine,
    dist_message_t* message) {
    
    if (!engine || !message || !engine->is_running) return false;
    
    message->header.receiver_id = 0; // Broadcast ID
    
    size_t sent_count = 0;
    
    // Send to all active connections
    for (size_t i = 0; i < engine->connection_count; i++) {
        if (engine->connections[i].is_active) {
            // Create copy of message for each recipient
            dist_message_t* copy = dist_comm_create_message(
                message->header.type,
                message->header.sender_id,
                engine->connections[i].remote_agent_id,
                message->payload,
                message->header.payload_size);
            
            if (copy && dist_comm_send_message(engine, copy)) {
                sent_count++;
            }
            
            if (copy) {
                dist_comm_free_message(copy);
            }
        }
    }
    
    printf("Broadcast message type %d to %zu agents\n", message->header.type, sent_count);
    
    return sent_count > 0;
}

// Receive message (simplified non-blocking implementation)
dist_message_t* dist_comm_receive_message(dist_comm_engine_t* engine) {
    if (!engine || !engine->is_running || engine->incoming_queue_size == 0) {
        return NULL;
    }
    
    // Return first message from queue
    dist_message_t* message = engine->incoming_queue[0];
    
    // Shift queue
    for (size_t i = 1; i < engine->incoming_queue_size; i++) {
        engine->incoming_queue[i - 1] = engine->incoming_queue[i];
    }
    engine->incoming_queue_size--;
    
    if (message) {
        engine->total_messages_received++;
        message->receive_time = (uint64_t)time(NULL);
    }
    
    return message;
}

// Send cognitive tensor
bool dist_comm_send_cognitive_tensor(
    dist_comm_engine_t* engine,
    uint64_t receiver_id,
    struct ggml_tensor* tensor,
    uint32_t attention_level) {
    
    if (!engine || !tensor) return false;
    
    // Create tensor payload (simplified)
    struct {
        uint32_t attention_level;
        int64_t dimensions[4];
        uint32_t element_count;
        uint32_t data_size;
        // Followed by tensor data
    } tensor_payload;
    
    tensor_payload.attention_level = attention_level;
    tensor_payload.dimensions[0] = tensor->ne[0];
    tensor_payload.dimensions[1] = tensor->ne[1];
    tensor_payload.dimensions[2] = tensor->ne[2];
    tensor_payload.dimensions[3] = tensor->ne[3];
    tensor_payload.element_count = (uint32_t)ggml_nelements(tensor);
    tensor_payload.data_size = (uint32_t)ggml_nbytes(tensor);
    
    // Create message with tensor metadata (actual tensor data would follow)
    dist_message_t* message = dist_comm_create_message(
        DIST_MSG_COGNITIVE_TENSOR,
        engine->local_agent_id,
        receiver_id,
        &tensor_payload,
        sizeof(tensor_payload));
    
    if (!message) return false;
    
    bool success = dist_comm_send_message(engine, message);
    dist_comm_free_message(message);
    
    if (success) {
        printf("Sent cognitive tensor to agent %lu (attention: %u, elements: %u)\n",
               receiver_id, attention_level, tensor_payload.element_count);
    }
    
    return success;
}

// Connect integration components
bool dist_comm_connect_pln_engine(
    dist_comm_engine_t* engine,
    pln_reasoning_engine_t* pln_engine) {
    
    if (!engine) return false;
    
    engine->pln_engine = pln_engine;
    
    printf("Connected PLN reasoning engine to distributed communication\n");
    
    return true;
}

bool dist_comm_connect_pattern_engine(
    dist_comm_engine_t* engine,
    pattern_engine_t* pattern_engine) {
    
    if (!engine) return false;
    
    engine->pattern_engine = pattern_engine;
    
    printf("Connected pattern matching engine to distributed communication\n");
    
    return true;
}

bool dist_comm_connect_cognitive_kernel(
    dist_comm_engine_t* engine,
    ggml_cognitive_kernel_t* cognitive_kernel) {
    
    if (!engine) return false;
    
    engine->cognitive_kernel = cognitive_kernel;
    
    printf("Connected cognitive kernel to distributed communication\n");
    
    return true;
}

// Get network performance metrics
bool dist_comm_get_network_metrics(
    dist_comm_engine_t* engine,
    float* average_latency,
    float* message_loss_rate,
    float* bandwidth_utilization) {
    
    if (!engine) return false;
    
    // Calculate metrics from connection statistics
    float total_latency = 0.0f;
    size_t active_connections = 0;
    
    for (size_t i = 0; i < engine->connection_count; i++) {
        if (engine->connections[i].is_active) {
            total_latency += engine->connections[i].average_latency;
            active_connections++;
        }
    }
    
    if (average_latency) {
        *average_latency = (active_connections > 0) ? 
            total_latency / (float)active_connections : 0.0f;
    }
    
    if (message_loss_rate) {
        // Simplified calculation
        uint64_t total_sent = engine->total_messages_sent;
        uint64_t total_received = engine->total_messages_received;
        *message_loss_rate = (total_sent > 0) ? 
            (1.0f - (float)total_received / (float)total_sent) : 0.0f;
    }
    
    if (bandwidth_utilization) {
        // Simplified bandwidth calculation
        *bandwidth_utilization = 0.5f; // Placeholder
    }
    
    return true;
}

// Print engine status
void dist_comm_print_status(dist_comm_engine_t* engine) {
    if (!engine) return;
    
    printf("\n=== Distributed Communication Engine Status ===\n");
    printf("Agent ID: %lu\n", engine->local_agent_id);
    printf("Hostname: %s:%u\n", engine->local_hostname, engine->local_port);
    printf("State: %d\n", engine->local_state);
    printf("Running: %s\n", engine->is_running ? "Yes" : "No");
    printf("Known agents: %zu/%zu\n", engine->agent_count, engine->agent_capacity);
    printf("Active connections: %zu/%zu\n", engine->connection_count, engine->connection_capacity);
    printf("Outgoing queue: %zu/%zu\n", engine->outgoing_queue_size, engine->max_queue_size);
    printf("Incoming queue: %zu/%zu\n", engine->incoming_queue_size, engine->max_queue_size);
    printf("Total messages sent: %lu\n", engine->total_messages_sent);
    printf("Total messages received: %lu\n", engine->total_messages_received);
    printf("Total bytes sent: %lu\n", engine->total_bytes_sent);
    printf("Total bytes received: %lu\n", engine->total_bytes_received);
    printf("Connection attempts: %lu\n", engine->connection_attempts);
    printf("Successful connections: %lu\n", engine->successful_connections);
    
    float success_rate = (engine->connection_attempts > 0) ?
        (float)engine->successful_connections / (float)engine->connection_attempts : 0.0f;
    printf("Connection success rate: %.2f%%\n", success_rate * 100.0f);
    
    printf("Discovery enabled: %s\n", engine->discovery_enabled ? "Yes" : "No");
    printf("Auto-connect: %s\n", engine->auto_connect ? "Yes" : "No");
    printf("==============================================\n\n");
}

// Print agent list
void dist_comm_print_agents(dist_comm_engine_t* engine) {
    if (!engine) return;
    
    printf("\n=== Known Agents ===\n");
    printf("Total agents: %zu\n\n", engine->agent_count);
    
    for (size_t i = 0; i < engine->agent_count; i++) {
        dist_agent_info_t* agent = &engine->agents[i];
        
        printf("Agent %zu:\n", i + 1);
        printf("  ID: %lu\n", agent->agent_id);
        printf("  Address: %s:%u\n", agent->hostname, agent->port);
        printf("  State: %d\n", agent->state);
        printf("  Protocol: %d\n", agent->preferred_protocol);
        printf("  Capabilities: PLN=%s, Patterns=%s, Tensors=%s, Coordination=%s\n",
               agent->supports_pln_reasoning ? "Yes" : "No",
               agent->supports_pattern_matching ? "Yes" : "No",
               agent->supports_tensor_operations ? "Yes" : "No",
               agent->supports_coordination ? "Yes" : "No");
        printf("  Messages sent: %lu\n", agent->message_count_sent);
        printf("  Messages received: %lu\n", agent->message_count_received);
        printf("  Average response time: %.2f ms\n", agent->average_response_time);
        printf("  Reliability score: %.2f\n", agent->reliability_score);
        printf("  Last heartbeat: %lu\n", agent->last_heartbeat_time);
        printf("\n");
    }
    
    printf("===================\n\n");
}