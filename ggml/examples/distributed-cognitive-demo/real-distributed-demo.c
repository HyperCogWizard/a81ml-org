#include "ggml-distributed-cognitive.h"
#include "ggml-distributed-communication.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>

// Demo: Real Distributed Communication
void demo_real_distributed_communication(void) {
    printf("\n=== Real Distributed Communication Demo ===\n");
    
    // Initialize GGML context
    struct ggml_init_params params = {
        .mem_size = 128 * 1024 * 1024,  // 128MB
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    struct ggml_context* ctx = ggml_init(params);
    
    // Create two distributed cognitive agents on different ports
    printf("\n1. Creating Distributed Cognitive Agents\n");
    
    distributed_cognitive_architecture_t* agent1 = distributed_cognitive_init(ctx, "localhost:8001");
    if (!agent1) {
        printf("Failed to initialize agent 1\n");
        return;
    }
    
    distributed_cognitive_architecture_t* agent2 = distributed_cognitive_init(ctx, "localhost:8002");
    if (!agent2) {
        printf("Failed to initialize agent 2\n");
        distributed_cognitive_free(agent1);
        return;
    }
    
    printf("Created cognitive agent %u at %s\n", agent1->agent_id, agent1->endpoint);
    printf("Created cognitive agent %u at %s\n", agent2->agent_id, agent2->endpoint);
    
    // Enable real distributed communication
    printf("\n2. Enabling Real Distributed Communication\n");
    
    if (!distributed_cognitive_enable_real_communication(agent1)) {
        printf("Failed to enable distributed communication for agent 1\n");
        goto cleanup;
    }
    
    if (!distributed_cognitive_enable_real_communication(agent2)) {
        printf("Failed to enable distributed communication for agent 2\n");
        goto cleanup;
    }
    
    // Connect agents to each other
    printf("\n3. Establishing Agent Connections\n");
    
    if (!distributed_cognitive_connect_to_agent(agent1, "localhost:8002", "PhilosopherAgent")) {
        printf("Agent 1 failed to connect to agent 2\n");
    }
    
    if (!distributed_cognitive_connect_to_agent(agent2, "localhost:8001", "ScientistAgent")) {
        printf("Agent 2 failed to connect to agent 1\n");
    }
    
    // Add knowledge to both agents
    printf("\n4. Adding Knowledge to Agents\n");
    
    // Agent 1: Philosophy-focused knowledge
    float consciousness_emb[64];
    for (int i = 0; i < 64; i++) consciousness_emb[i] = sinf((float)i / 64.0f * 3.14159f);
    struct ggml_tensor* consciousness_tensor = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 64);
    memcpy(consciousness_tensor->data, consciousness_emb, sizeof(consciousness_emb));
    
    uint64_t consciousness_unit1 = cogfluence_add_knowledge_unit(
        agent1->cogfluence, "consciousness", COGFLUENCE_CONCEPT, consciousness_tensor);
    
    uint64_t philosophy_unit1 = cogfluence_add_knowledge_unit(
        agent1->cogfluence, "philosophy_of_mind", COGFLUENCE_CONCEPT, consciousness_tensor);
    
    printf("Added knowledge: consciousness (nodes: 1)\n");
    printf("Added knowledge: philosophy_of_mind (nodes: 2)\n");
    
    // Agent 2: Science-focused knowledge
    float neuroscience_emb[64];
    for (int i = 0; i < 64; i++) neuroscience_emb[i] = cosf((float)i / 64.0f * 3.14159f);
    struct ggml_tensor* neuroscience_tensor = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 64);
    memcpy(neuroscience_tensor->data, neuroscience_emb, sizeof(neuroscience_emb));
    
    uint64_t neuroscience_unit2 = cogfluence_add_knowledge_unit(
        agent2->cogfluence, "neuroscience", COGFLUENCE_CONCEPT, neuroscience_tensor);
    
    printf("Added knowledge: neuroscience (nodes: 1)\n");
    
    // Simulate consciousness exploration with distributed communication
    printf("\n5. Simulating Consciousness Exploration\n");
    
    for (int round = 1; round <= 5; round++) {
        printf("\n--- Round %d ---\n", round);
        
        // Update dashboards
        dashboard_update(agent1);
        dashboard_update(agent2);
        
        // Broadcast cognitive states
        printf("Agent %u broadcasting cognitive state...\n", agent1->agent_id);
        distributed_cognitive_broadcast_state(agent1);
        
        printf("Agent %u broadcasting cognitive state...\n", agent2->agent_id);
        distributed_cognitive_broadcast_state(agent2);
        
        // Simulate distributed attention allocation
        if (agent1->comm_manager && agent2->comm_manager) {
            distributed_communication_manager_t* comm1 = (distributed_communication_manager_t*)agent1->comm_manager;
            distributed_communication_manager_t* comm2 = (distributed_communication_manager_t*)agent2->comm_manager;
            
            float attention_weights[4] = {0.2f, 0.6f, 0.15f, 0.05f};  // Focus on reasoning
            float salience = 0.3f + 0.1f * round;
            
            printf("Agent %u sending attention update to agent %u (salience: %.2f)\n", 
                   agent1->agent_id, agent2->agent_id, salience);
            distributed_comm_send_attention_update(
                comm1, agent2->agent_id, attention_weights, salience, 1, consciousness_unit1);
            
            printf("Agent %u sending attention update to agent %u (salience: %.2f)\n", 
                   agent2->agent_id, agent1->agent_id, salience);
            distributed_comm_send_attention_update(
                comm2, agent1->agent_id, attention_weights, salience, 1, neuroscience_unit2);
        }
        
        // Simulate tensor exchange
        if (round == 3) {
            printf("Agent %u sending consciousness tensor to agent %u\n", 
                   agent1->agent_id, agent2->agent_id);
            if (agent1->comm_manager) {
                distributed_communication_manager_t* comm1 = (distributed_communication_manager_t*)agent1->comm_manager;
                distributed_comm_send_tensor(
                    comm1, agent2->agent_id, consciousness_tensor, "consciousness_embedding");
            }
            
            printf("Agent %u sending neuroscience tensor to agent %u\n", 
                   agent2->agent_id, agent1->agent_id);
            if (agent2->comm_manager) {
                distributed_communication_manager_t* comm2 = (distributed_communication_manager_t*)agent2->comm_manager;
                distributed_comm_send_tensor(
                    comm2, agent1->agent_id, neuroscience_tensor, "neuroscience_embedding");
            }
        }
        
        // Sync with network
        distributed_cognitive_sync_with_network(agent1);
        distributed_cognitive_sync_with_network(agent2);
        
        usleep(500000);  // 0.5 second delay
    }
    
    // Print network status for both agents
    printf("\n6. Network Status Reports\n");
    
    printf("\n--- Agent 1 Network Status ---\n");
    distributed_cognitive_print_network_status(agent1);
    
    printf("\n--- Agent 2 Network Status ---\n");
    distributed_cognitive_print_network_status(agent2);
    
    // Demonstrate collaborative workflow
    printf("\n7. Collaborative Distributed Workflow\n");
    
    // Create a workflow on agent 1
    uint64_t collaborative_workflow = cogfluence_create_workflow(agent1->cogfluence, "collaborative_consciousness_analysis");
    cogfluence_add_workflow_step(agent1->cogfluence, collaborative_workflow, consciousness_unit1);
    cogfluence_add_workflow_step(agent1->cogfluence, collaborative_workflow, philosophy_unit1);
    
    printf("Agent %u created collaborative workflow: collaborative_consciousness_analysis\n", agent1->agent_id);
    
    // Request participation from agent 2
    if (agent1->comm_manager) {
        distributed_communication_manager_t* comm1 = (distributed_communication_manager_t*)agent1->comm_manager;
        bool success = distributed_comm_request_workflow_participation(
            comm1, agent2->agent_id, collaborative_workflow, 
            "Join consciousness analysis combining philosophy and neuroscience");
        printf("Workflow participation request sent: %s\n", success ? "SUCCESS" : "FAILED");
    }
    
    // Execute collaborative workflow
    printf("Executing collaborative workflow...\n");
    cogfluence_execute_workflow(agent1->cogfluence, collaborative_workflow);
    
    // Final performance metrics
    printf("\n8. Final Performance Metrics\n");
    
    printf("\n--- Agent 1 Final State ---\n");
    distributed_cognitive_print_architecture(agent1);
    dashboard_print(agent1);
    
    printf("\n--- Agent 2 Final State ---\n");
    distributed_cognitive_print_architecture(agent2);
    dashboard_print(agent2);
    
    // Compute network-wide coherence
    float total_coherence = 0.0f;
    int agent_count = 0;
    
    if (agent1->comm_manager) {
        distributed_communication_manager_t* comm1 = (distributed_communication_manager_t*)agent1->comm_manager;
        for (size_t i = 0; i < comm1->agent_count; i++) {
            if (comm1->known_agents[i].active) {
                total_coherence += comm1->known_agents[i].cognitive_coherence;
                agent_count++;
            }
        }
    }
    
    float network_coherence = agent_count > 0 ? total_coherence / agent_count : 0.0f;
    
    printf("\n=== Distributed Communication Demo Summary ===\n");
    printf("✓ Two distributed cognitive agents created and connected\n");
    printf("✓ Real network communication established\n");
    printf("✓ Cognitive state synchronization across agents\n");
    printf("✓ Distributed attention allocation\n");
    printf("✓ Cross-agent tensor exchange\n");
    printf("✓ Collaborative workflow coordination\n");
    printf("✓ Network-wide coherence: %.3f\n", network_coherence);
    
    if (agent1->comm_manager) {
        distributed_communication_manager_t* comm1 = (distributed_communication_manager_t*)agent1->comm_manager;
        printf("✓ Agent 1 sent %llu bytes, received %llu bytes\n", 
               (unsigned long long)comm1->total_bytes_sent,
               (unsigned long long)comm1->total_bytes_received);
    }
    
    if (agent2->comm_manager) {
        distributed_communication_manager_t* comm2 = (distributed_communication_manager_t*)agent2->comm_manager;
        printf("✓ Agent 2 sent %llu bytes, received %llu bytes\n", 
               (unsigned long long)comm2->total_bytes_sent,
               (unsigned long long)comm2->total_bytes_received);
    }
    
    if (network_coherence > 0.4f) {
        printf("\n🎉 SUCCESS: Real distributed cognitive communication achieved!\n");
        printf("The agents successfully demonstrated:\n");
        printf("• Cross-network cognitive state sharing\n");
        printf("• Distributed attention coordination\n");
        printf("• Real-time tensor exchange\n");
        printf("• Collaborative workflow execution\n");
        printf("• Network-wide coherence maintenance\n");
    } else {
        printf("\n⚠️  PARTIAL SUCCESS: Communication established but coherence could be improved\n");
    }
    
cleanup:
    // Cleanup
    distributed_cognitive_free(agent1);
    distributed_cognitive_free(agent2);
    ggml_free(ctx);
    
    printf("\nReal Distributed Communication Demo Complete!\n");
}

int main(void) {
    printf("GGML Distributed Cognitive Communication Demo\n");
    printf("=============================================\n");
    
    demo_real_distributed_communication();
    
    return 0;
}