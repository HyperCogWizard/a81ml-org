#include "cognitive-agent.h"
#include "ggml-opencog.h"
#include "ggml.h"
#include <stdio.h>
#include <unistd.h>

// Enhanced demo showing PLN reasoning integration with cognitive agents
void demo_pln_philosophical_reasoning(void) {
    printf("\n=== PLN Philosophical Reasoning Demo ===\n");
    
    // Initialize GGML context for PLN AtomSpace
    struct ggml_init_params params = {
        .mem_size = 2 * 1024 * 1024,
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    struct ggml_context* ctx = ggml_init(params);
    
    // Create cognitive agents with PLN reasoning capabilities
    cognitive_agent* philosopher = create_cognitive_agent("localhost:8001");
    cognitive_agent* logician = create_cognitive_agent("localhost:8002");
    
    // Initialize PLN AtomSpaces for each agent
    opencog_atomspace_t* phil_atomspace = opencog_atomspace_init(ctx);
    opencog_atomspace_t* logic_atomspace = opencog_atomspace_init(ctx);
    
    printf("\nInitializing philosophical knowledge bases...\n");
    
    // Philosopher's knowledge: consciousness and self-awareness
    uint64_t consciousness_id = opencog_add_node(phil_atomspace, OPENCOG_CONCEPT_NODE, "Consciousness");
    uint64_t self_awareness_id = opencog_add_node(phil_atomspace, OPENCOG_CONCEPT_NODE, "SelfAwareness");
    uint64_t qualia_id = opencog_add_node(phil_atomspace, OPENCOG_CONCEPT_NODE, "Qualia");
    uint64_t thinking_id = opencog_add_node(phil_atomspace, OPENCOG_CONCEPT_NODE, "Thinking");
    
    // Set initial truth values based on philosophical intuitions
    opencog_set_truth_value(phil_atomspace, consciousness_id, 0.9f, 0.8f);
    opencog_set_truth_value(phil_atomspace, self_awareness_id, 0.85f, 0.9f);
    opencog_set_truth_value(phil_atomspace, qualia_id, 0.7f, 0.6f); // Uncertain concept
    opencog_set_truth_value(phil_atomspace, thinking_id, 0.95f, 0.95f);
    
    // Create philosophical implications
    uint64_t thinking_consciousness[2] = {thinking_id, consciousness_id};
    uint64_t thinking_consciousness_link = opencog_add_link(phil_atomspace, OPENCOG_IMPLICATION_LINK, 
                                                           thinking_consciousness, 2);
    opencog_set_truth_value(phil_atomspace, thinking_consciousness_link, 0.8f, 0.85f);
    
    uint64_t consciousness_selfaware[2] = {consciousness_id, self_awareness_id};
    uint64_t consciousness_selfaware_link = opencog_add_link(phil_atomspace, OPENCOG_IMPLICATION_LINK, 
                                                            consciousness_selfaware, 2);
    opencog_set_truth_value(phil_atomspace, consciousness_selfaware_link, 0.9f, 0.8f);
    
    printf("✓ Philosopher knowledge: Thinking → Consciousness → SelfAwareness\n");
    
    // Logician's knowledge: formal reasoning about mind
    uint64_t logic_thinking_id = opencog_add_node(logic_atomspace, OPENCOG_CONCEPT_NODE, "Thinking");
    uint64_t information_processing_id = opencog_add_node(logic_atomspace, OPENCOG_CONCEPT_NODE, "InformationProcessing");
    uint64_t computation_id = opencog_add_node(logic_atomspace, OPENCOG_CONCEPT_NODE, "Computation");
    uint64_t algorithm_id = opencog_add_node(logic_atomspace, OPENCOG_CONCEPT_NODE, "Algorithm");
    
    // Set logical truth values
    opencog_set_truth_value(logic_atomspace, logic_thinking_id, 0.95f, 0.95f);
    opencog_set_truth_value(logic_atomspace, information_processing_id, 0.9f, 0.95f);
    opencog_set_truth_value(logic_atomspace, computation_id, 0.95f, 0.9f);
    opencog_set_truth_value(logic_atomspace, algorithm_id, 0.98f, 0.95f);
    
    // Create logical implications
    uint64_t thinking_processing[2] = {logic_thinking_id, information_processing_id};
    uint64_t thinking_processing_link = opencog_add_link(logic_atomspace, OPENCOG_IMPLICATION_LINK, 
                                                        thinking_processing, 2);
    opencog_set_truth_value(logic_atomspace, thinking_processing_link, 0.9f, 0.9f);
    
    uint64_t processing_computation[2] = {information_processing_id, computation_id};
    uint64_t processing_computation_link = opencog_add_link(logic_atomspace, OPENCOG_IMPLICATION_LINK, 
                                                           processing_computation, 2);
    opencog_set_truth_value(logic_atomspace, processing_computation_link, 0.85f, 0.9f);
    
    printf("✓ Logician knowledge: Thinking → InfoProcessing → Computation\n");
    
    // Initialize PLN reasoning sessions
    opencog_pln_session_t* phil_session = opencog_pln_session_init(phil_atomspace, 0.7f);
    opencog_pln_session_t* logic_session = opencog_pln_session_init(logic_atomspace, 0.75f);
    
    printf("\nRunning PLN reasoning cycles...\n");
    
    // Philosopher reasons about consciousness
    printf("\n--- Philosopher's reasoning ---\n");
    for (int i = 0; i < 3; i++) {
        bool made_inference = opencog_pln_inference_cycle(phil_session);
        printf("Cycle %d: %s\n", i + 1, made_inference ? "New philosophical insight" : "Reflection on existing beliefs");
        
        if (made_inference) {
            // Apply attention to newly inferred concepts
            allocate_attention(philosopher->attention, 0.4f, COGNITIVE_TYPE_REASONING);
        }
    }
    
    // Logician applies formal reasoning
    printf("\n--- Logician's reasoning ---\n");
    for (int i = 0; i < 3; i++) {
        bool made_inference = opencog_pln_inference_cycle(logic_session);
        printf("Cycle %d: %s\n", i + 1, made_inference ? "Logical deduction made" : "Formal verification");
        
        if (made_inference) {
            allocate_attention(logician->attention, 0.5f, COGNITIVE_TYPE_REASONING);
        }
    }
    
    // Cross-agent knowledge exchange with PLN reasoning
    printf("\n--- Cross-agent PLN knowledge exchange ---\n");
    
    // Find patterns in philosopher's atomspace that might interest logician
    uint64_t phil_implications[10];
    size_t phil_impl_count = opencog_pln_find_patterns(phil_atomspace, OPENCOG_IMPLICATION_LINK, 
                                                       phil_implications, 10);
    
    printf("Philosopher found %zu implication patterns\n", phil_impl_count);
    
    // Get truth values for cross-reasoning
    opencog_truth_value_t thinking_consciousness_tv = opencog_get_truth_value(phil_atomspace, thinking_consciousness_link);
    opencog_truth_value_t consciousness_selfaware_tv = opencog_get_truth_value(phil_atomspace, consciousness_selfaware_link);
    
    // Test PLN deduction across knowledge bases
    if (phil_impl_count > 0) {
        // Apply PLN deduction: (Thinking→Consciousness, Consciousness→SelfAware) ⊢ (Thinking→SelfAware)
        opencog_truth_value_t thinking_selfaware_tv = opencog_pln_deduction(thinking_consciousness_tv, consciousness_selfaware_tv);
        
        printf("PLN Deduction result: Thinking → SelfAwareness (%.2f, %.2f)\n", 
               thinking_selfaware_tv.strength, thinking_selfaware_tv.confidence);
        
        // Create the inferred link
        uint64_t thinking_selfaware[2] = {thinking_id, self_awareness_id};
        uint64_t thinking_selfaware_link = opencog_add_link(phil_atomspace, OPENCOG_IMPLICATION_LINK, 
                                                           thinking_selfaware, 2);
        opencog_set_truth_value(phil_atomspace, thinking_selfaware_link, 
                                thinking_selfaware_tv.strength, thinking_selfaware_tv.confidence);
    }
    
    // Test PLN similarity inference between agent knowledge bases
    opencog_truth_value_t phil_thinking_tv = opencog_get_truth_value(phil_atomspace, thinking_id);
    opencog_truth_value_t logic_thinking_tv = opencog_get_truth_value(logic_atomspace, logic_thinking_id);
    
    // Create similarity relation
    opencog_truth_value_t thinking_similarity = {0.9f, 0.8f, 10.0f}; // High similarity between concepts
    opencog_truth_value_t processing_consciousness_tv = {0.6f, 0.5f, 5.0f}; // Tentative connection
    
    opencog_truth_value_t similarity_inference = opencog_pln_similarity_inference(thinking_similarity, processing_consciousness_tv);
    printf("PLN Similarity inference: Info processing relates to consciousness (%.2f, %.2f)\n",
           similarity_inference.strength, similarity_inference.confidence);
    
    // Simulate collaborative reasoning
    printf("\n--- Collaborative reasoning session ---\n");
    
    // Agents exchange cognitive tensors with PLN-enhanced reasoning
    struct ggml_tensor* phil_insight = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 256);
    float* phil_data = (float*)phil_insight->data;
    for (int i = 0; i < 256; i++) {
        phil_data[i] = thinking_consciousness_tv.strength * sinf((float)i / 256.0f * 6.28f);
    }
    
    struct ggml_tensor* logic_insight = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 256);
    float* logic_data = (float*)logic_insight->data;
    for (int i = 0; i < 256; i++) {
        logic_data[i] = processing_computation_link > 0 ? cosf((float)i / 256.0f * 3.14f) : 0.0f;
    }
    
    // Send PLN-enhanced insights
    send_cognitive_tensor(philosopher, logician->agent_id, phil_insight, thinking_consciousness_tv.confidence);
    send_cognitive_tensor(logician, philosopher->agent_id, logic_insight, 0.85f);
    
    // Process insights with PLN reasoning
    cognitive_tensor_packet phil_msg = {0};
    phil_msg.cognitive_type = COGNITIVE_TYPE_REASONING;
    phil_msg.attention_weight = thinking_consciousness_tv.confidence;
    phil_msg.source_agent_id = philosopher->agent_id;
    phil_msg.target_agent_id = logician->agent_id;
    phil_msg.salience_score = thinking_consciousness_tv.strength;
    
    process_incoming_tensor(logician, &phil_msg);
    
    cognitive_tensor_packet logic_msg = {0};
    logic_msg.cognitive_type = COGNITIVE_TYPE_REASONING;
    logic_msg.attention_weight = 0.85f;
    logic_msg.source_agent_id = logician->agent_id;
    logic_msg.target_agent_id = philosopher->agent_id;
    logic_msg.salience_score = 0.8f;
    
    process_incoming_tensor(philosopher, &logic_msg);
    
    // Final reasoning states
    printf("\n--- Final PLN reasoning statistics ---\n");
    opencog_print_atomspace_statistics(phil_atomspace);
    opencog_print_atomspace_statistics(logic_atomspace);
    
    printf("\nAgent cognitive states:\n");
    printf("Philosopher - Inferences: %lu, Attention on reasoning: %.2f\n", 
           philosopher->reasoning->inferences_made, philosopher->attention->reasoning_allocation);
    printf("Logician - Inferences: %lu, Attention on reasoning: %.2f\n", 
           logician->reasoning->inferences_made, logician->attention->reasoning_allocation);
    
    // Cleanup
    opencog_pln_session_free(phil_session);
    opencog_pln_session_free(logic_session);
    opencog_atomspace_free(phil_atomspace);
    opencog_atomspace_free(logic_atomspace);
    cleanup_cognitive_agent(philosopher);
    cleanup_cognitive_agent(logician);
    ggml_free(ctx);
    
    printf("\nPLN philosophical reasoning demo completed.\n");
}

// Demo showing PLN integration with scientific reasoning
void demo_pln_scientific_reasoning(void) {
    printf("\n=== PLN Scientific Reasoning Demo ===\n");
    
    // Initialize GGML context
    struct ggml_init_params params = {
        .mem_size = 1024 * 1024,
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    struct ggml_context* ctx = ggml_init(params);
    
    // Create scientific researcher agent
    cognitive_agent* researcher = create_cognitive_agent("localhost:9001");
    opencog_atomspace_t* research_atomspace = opencog_atomspace_init(ctx);
    
    printf("\nBuilding scientific knowledge base...\n");
    
    // Create scientific concepts
    uint64_t hypothesis_id = opencog_add_node(research_atomspace, OPENCOG_CONCEPT_NODE, "Hypothesis");
    uint64_t experiment_id = opencog_add_node(research_atomspace, OPENCOG_CONCEPT_NODE, "Experiment");
    uint64_t evidence_id = opencog_add_node(research_atomspace, OPENCOG_CONCEPT_NODE, "Evidence");
    uint64_t theory_id = opencog_add_node(research_atomspace, OPENCOG_CONCEPT_NODE, "Theory");
    uint64_t prediction_id = opencog_add_node(research_atomspace, OPENCOG_CONCEPT_NODE, "Prediction");
    
    // Set initial scientific truth values
    opencog_set_truth_value(research_atomspace, hypothesis_id, 0.6f, 0.5f); // Tentative
    opencog_set_truth_value(research_atomspace, experiment_id, 0.9f, 0.95f); // Reliable method
    opencog_set_truth_value(research_atomspace, evidence_id, 0.8f, 0.9f);    // Strong but not perfect
    opencog_set_truth_value(research_atomspace, theory_id, 0.7f, 0.6f);      // Developing
    opencog_set_truth_value(research_atomspace, prediction_id, 0.5f, 0.4f);  // Uncertain
    
    // Create scientific reasoning chains
    uint64_t hypothesis_experiment[2] = {hypothesis_id, experiment_id};
    uint64_t hyp_exp_link = opencog_add_link(research_atomspace, OPENCOG_IMPLICATION_LINK, 
                                            hypothesis_experiment, 2);
    opencog_set_truth_value(research_atomspace, hyp_exp_link, 0.8f, 0.9f); // Strong methodology
    
    uint64_t experiment_evidence[2] = {experiment_id, evidence_id};
    uint64_t exp_ev_link = opencog_add_link(research_atomspace, OPENCOG_IMPLICATION_LINK, 
                                           experiment_evidence, 2);
    opencog_set_truth_value(research_atomspace, exp_ev_link, 0.85f, 0.9f);
    
    uint64_t evidence_theory[2] = {evidence_id, theory_id};
    uint64_t ev_theory_link = opencog_add_link(research_atomspace, OPENCOG_IMPLICATION_LINK, 
                                              evidence_theory, 2);
    opencog_set_truth_value(research_atomspace, ev_theory_link, 0.75f, 0.8f);
    
    uint64_t theory_prediction[2] = {theory_id, prediction_id};
    uint64_t theory_pred_link = opencog_add_link(research_atomspace, OPENCOG_IMPLICATION_LINK, 
                                                theory_prediction, 2);
    opencog_set_truth_value(research_atomspace, theory_pred_link, 0.7f, 0.75f);
    
    printf("✓ Scientific reasoning chain: Hypothesis → Experiment → Evidence → Theory → Prediction\n");
    
    // Initialize PLN reasoning session
    opencog_pln_session_t* research_session = opencog_pln_session_init(research_atomspace, 0.6f);
    
    printf("\nSimulating scientific reasoning process...\n");
    
    // Apply PLN deduction for scientific method
    opencog_truth_value_t hyp_exp_tv = opencog_get_truth_value(research_atomspace, hyp_exp_link);
    opencog_truth_value_t exp_ev_tv = opencog_get_truth_value(research_atomspace, exp_ev_link);
    
    // Deduction: (Hypothesis→Experiment, Experiment→Evidence) ⊢ (Hypothesis→Evidence)
    opencog_truth_value_t hyp_evidence_tv = opencog_pln_deduction(hyp_exp_tv, exp_ev_tv);
    printf("PLN Deduction: Hypothesis directly supported by evidence (%.2f, %.2f)\n",
           hyp_evidence_tv.strength, hyp_evidence_tv.confidence);
    
    // Chain reasoning: Evidence → Theory → Prediction
    opencog_truth_value_t ev_theory_tv = opencog_get_truth_value(research_atomspace, ev_theory_link);
    opencog_truth_value_t theory_pred_tv = opencog_get_truth_value(research_atomspace, theory_pred_link);
    
    opencog_truth_value_t ev_prediction_tv = opencog_pln_deduction(ev_theory_tv, theory_pred_tv);
    printf("PLN Deduction: Evidence supports predictions (%.2f, %.2f)\n",
           ev_prediction_tv.strength, ev_prediction_tv.confidence);
    
    // Test PLN induction for hypothesis strengthening
    opencog_truth_value_t evidence_tv = opencog_get_truth_value(research_atomspace, evidence_id);
    opencog_truth_value_t hypothesis_tv = opencog_get_truth_value(research_atomspace, hypothesis_id);
    
    opencog_truth_value_t strengthened_hyp = opencog_pln_induction(evidence_tv, hypothesis_tv);
    printf("PLN Induction: Hypothesis strengthened by evidence (%.2f, %.2f)\n",
           strengthened_hyp.strength, strengthened_hyp.confidence);
    
    // Update hypothesis with strengthened truth value
    opencog_set_truth_value(research_atomspace, hypothesis_id, 
                            strengthened_hyp.strength, strengthened_hyp.confidence);
    
    // Run PLN inference cycles for scientific discovery
    printf("\nRunning scientific PLN reasoning cycles...\n");
    for (int i = 0; i < 5; i++) {
        bool made_inference = opencog_pln_inference_cycle(research_session);
        printf("Research cycle %d: %s\n", i + 1, 
               made_inference ? "New scientific insight discovered" : "Consolidating existing knowledge");
        
        if (made_inference) {
            allocate_attention(researcher->attention, 0.6f, COGNITIVE_TYPE_REASONING);
        }
    }
    
    // Test backward chaining for prediction validation
    printf("\nTesting prediction validation via backward chaining...\n");
    opencog_truth_value_t desired_prediction = {0.8f, 0.8f, 10.0f};
    bool validation_success = opencog_pln_backward_chain(research_atomspace, prediction_id, desired_prediction);
    
    opencog_truth_value_t final_prediction = opencog_get_truth_value(research_atomspace, prediction_id);
    printf("Prediction validation %s: Final truth value (%.2f, %.2f)\n",
           validation_success ? "succeeded" : "failed",
           final_prediction.strength, final_prediction.confidence);
    
    // Final scientific reasoning statistics
    printf("\n--- Scientific PLN reasoning results ---\n");
    opencog_print_atomspace_statistics(research_atomspace);
    
    printf("Research agent state:\n");
    printf("- Total inferences: %lu\n", researcher->reasoning->inferences_made);
    printf("- Reasoning attention: %.2f\n", researcher->attention->reasoning_allocation);
    printf("- Memory utilization: %zu concepts\n", researcher->memory->node_count);
    
    // Cleanup
    opencog_pln_session_free(research_session);
    opencog_atomspace_free(research_atomspace);
    cleanup_cognitive_agent(researcher);
    ggml_free(ctx);
    
    printf("\nPLN scientific reasoning demo completed.\n");
}

int main(void) {
    printf("Enhanced Cognitive Agents with PLN Reasoning Engine\n");
    printf("==================================================\n");
    
    printf("This demonstration showcases the integration of Probabilistic Logic Networks (PLN)\n");
    printf("with the distributed cognitive architecture, enabling sophisticated reasoning\n");
    printf("capabilities including deduction, induction, abduction, and collaborative inference.\n");
    
    // Run enhanced demonstrations
    demo_pln_philosophical_reasoning();
    demo_pln_scientific_reasoning();
    
    printf("\n==================================================\n");
    printf("PLN reasoning engine integration completed successfully!\n");
    printf("\nKey PLN features demonstrated:\n");
    printf("✓ Advanced inference rules (modus ponens, deduction, induction, abduction)\n");
    printf("✓ Forward and backward chaining mechanisms\n");
    printf("✓ Truth value propagation and uncertainty handling\n");
    printf("✓ Pattern matching and rule discovery\n");
    printf("✓ Integration with cognitive agent attention systems\n");
    printf("✓ Cross-agent knowledge exchange with PLN reasoning\n");
    printf("✓ Scientific method modeling with probabilistic logic\n");
    printf("✓ Collaborative reasoning between specialized agents\n");
    
    return 0;
}