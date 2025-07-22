#include "ggml-pln-reasoning.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

// Initialize PLN reasoning engine
pln_reasoning_engine_t* pln_reasoning_engine_init(
    opencog_atomspace_t* atomspace,
    ggml_cognitive_kernel_t* cognitive_kernel) {
    
    if (!atomspace || !cognitive_kernel) return NULL;
    
    pln_reasoning_engine_t* engine = malloc(sizeof(pln_reasoning_engine_t));
    if (!engine) return NULL;
    
    engine->atomspace = atomspace;
    engine->cognitive_kernel = cognitive_kernel;
    
    // Initialize inference history
    engine->history_capacity = 1024;
    engine->inference_history = calloc(engine->history_capacity, sizeof(pln_inference_context_t));
    engine->history_count = 0;
    
    // Initialize performance metrics
    engine->total_inferences = 0;
    engine->successful_inferences = 0;
    engine->failed_inferences = 0;
    engine->average_confidence = 0.0f;
    
    // Set default parameters
    engine->default_confidence_threshold = 0.6f;
    engine->default_strength_threshold = 0.5f;
    engine->attention_boost_factor = 1.5f;
    engine->temporal_decay_rate = 0.95f;
    engine->revision_factor = 0.8f;
    engine->choice_factor = 0.7f;
    
    engine->initialized = true;
    
    printf("PLN reasoning engine initialized with %zu inference history capacity\n", 
           engine->history_capacity);
    
    return engine;
}

// Free PLN reasoning engine
void pln_reasoning_engine_free(pln_reasoning_engine_t* engine) {
    if (!engine) return;
    
    if (engine->inference_history) {
        free(engine->inference_history);
    }
    
    free(engine);
}

// PLN truth value combination for deduction A→B, B→C ⊢ A→C
static opencog_truth_value_t pln_deduction_formula(
    opencog_truth_value_t ab_tv,
    opencog_truth_value_t bc_tv) {
    
    opencog_truth_value_t result;
    
    // PLN deduction strength formula
    result.strength = ab_tv.strength * bc_tv.strength;
    
    // PLN deduction confidence formula
    float count_factor = (ab_tv.count * bc_tv.count) / (ab_tv.count + bc_tv.count + 1.0f);
    result.confidence = (ab_tv.confidence * bc_tv.confidence) * count_factor;
    
    result.count = fminf(ab_tv.count, bc_tv.count);
    
    return result;
}

// PLN induction formula A→B, B ⊢ A
static opencog_truth_value_t pln_induction_formula(
    opencog_truth_value_t ab_tv,
    opencog_truth_value_t b_tv) {
    
    opencog_truth_value_t result;
    
    // PLN induction strength formula (weaker than deduction)
    result.strength = (ab_tv.strength * b_tv.strength) / 
                     (ab_tv.strength + (1.0f - ab_tv.strength) * b_tv.strength);
    
    // Induction confidence is generally lower
    result.confidence = ab_tv.confidence * b_tv.confidence * 0.8f;
    
    result.count = fminf(ab_tv.count, b_tv.count) * 0.9f;
    
    return result;
}

// PLN abduction formula A→B, B ⊢ A (explanation)
static opencog_truth_value_t pln_abduction_formula(
    opencog_truth_value_t ab_tv,
    opencog_truth_value_t b_tv) {
    
    opencog_truth_value_t result;
    
    // PLN abduction strength formula (weakest inference)
    result.strength = (ab_tv.strength * b_tv.strength) / 
                     (ab_tv.strength + (1.0f - ab_tv.strength) * (1.0f - b_tv.strength));
    
    // Abduction confidence is lowest
    result.confidence = ab_tv.confidence * b_tv.confidence * 0.6f;
    
    result.count = fminf(ab_tv.count, b_tv.count) * 0.7f;
    
    return result;
}

// Apply modus ponens: P → Q, P ⊢ Q
bool pln_modus_ponens(
    pln_reasoning_engine_t* engine,
    uint64_t implication_atom_id,
    uint64_t premise_atom_id,
    uint64_t* conclusion_atom_id,
    opencog_truth_value_t* conclusion_tv) {
    
    if (!engine || !conclusion_atom_id || !conclusion_tv) return false;
    
    // Get implication atom (should be A→B)
    opencog_atom_t* implication_atom = opencog_get_atom(engine->atomspace, implication_atom_id);
    if (!implication_atom || implication_atom->type != OPENCOG_IMPLICATION_LINK) {
        engine->failed_inferences++;
        return false;
    }
    
    // Get premise atom
    opencog_atom_t* premise_atom = opencog_get_atom(engine->atomspace, premise_atom_id);
    if (!premise_atom) {
        engine->failed_inferences++;
        return false;
    }
    
    // Extract conclusion from implication (second outgoing atom)
    if (implication_atom->outgoing_count < 2) {
        engine->failed_inferences++;
        return false;
    }
    
    *conclusion_atom_id = implication_atom->outgoing[1];
    
    // Combine truth values using PLN modus ponens formula
    opencog_truth_value_t impl_tv = implication_atom->truth_value;
    opencog_truth_value_t prem_tv = premise_atom->truth_value;
    
    conclusion_tv->strength = impl_tv.strength * prem_tv.strength;
    conclusion_tv->confidence = impl_tv.confidence * prem_tv.confidence;
    conclusion_tv->count = fminf(impl_tv.count, prem_tv.count);
    
    // Check if conclusion meets confidence threshold
    if (conclusion_tv->confidence < engine->default_confidence_threshold) {
        engine->failed_inferences++;
        return false;
    }
    
    engine->successful_inferences++;
    engine->total_inferences++;
    
    printf("PLN Modus Ponens: Concluded atom %lu with strength %.2f, confidence %.2f\n",
           *conclusion_atom_id, conclusion_tv->strength, conclusion_tv->confidence);
    
    return true;
}

// Apply deduction: A → B, B → C ⊢ A → C
bool pln_deduction(
    pln_reasoning_engine_t* engine,
    uint64_t premise1_atom_id,
    uint64_t premise2_atom_id,
    uint64_t* conclusion_atom_id,
    opencog_truth_value_t* conclusion_tv) {
    
    if (!engine || !conclusion_atom_id || !conclusion_tv) return false;
    
    // Get premise atoms (should be implications)
    opencog_atom_t* premise1 = opencog_get_atom(engine->atomspace, premise1_atom_id);
    opencog_atom_t* premise2 = opencog_get_atom(engine->atomspace, premise2_atom_id);
    
    if (!premise1 || !premise2 || 
        premise1->type != OPENCOG_IMPLICATION_LINK ||
        premise2->type != OPENCOG_IMPLICATION_LINK) {
        engine->failed_inferences++;
        return false;
    }
    
    // Check if chains properly: A→B, B→C
    if (premise1->outgoing_count < 2 || premise2->outgoing_count < 2) {
        engine->failed_inferences++;
        return false;
    }
    
    uint64_t b_from_first = premise1->outgoing[1];
    uint64_t b_from_second = premise2->outgoing[0];
    
    if (b_from_first != b_from_second) {
        engine->failed_inferences++;
        return false;
    }
    
    // Create new implication A→C
    uint64_t antecedent = premise1->outgoing[0];
    uint64_t consequent = premise2->outgoing[1];
    
    uint64_t outgoing[2] = {antecedent, consequent};
    *conclusion_atom_id = opencog_add_link(engine->atomspace, 
                                          OPENCOG_IMPLICATION_LINK, 
                                          outgoing, 2);
    
    // Apply PLN deduction formula
    *conclusion_tv = pln_deduction_formula(premise1->truth_value, premise2->truth_value);
    
    // Set truth value for new atom
    opencog_set_truth_value(engine->atomspace, *conclusion_atom_id, 
                           conclusion_tv->strength, conclusion_tv->confidence);
    
    if (conclusion_tv->confidence < engine->default_confidence_threshold) {
        engine->failed_inferences++;
        return false;
    }
    
    engine->successful_inferences++;
    engine->total_inferences++;
    
    printf("PLN Deduction: Created atom %lu with strength %.2f, confidence %.2f\n",
           *conclusion_atom_id, conclusion_tv->strength, conclusion_tv->confidence);
    
    return true;
}

// Apply induction: A → B, B ⊢ A
bool pln_induction(
    pln_reasoning_engine_t* engine,
    uint64_t implication_atom_id,
    uint64_t conclusion_atom_id,
    uint64_t* premise_atom_id,
    opencog_truth_value_t* premise_tv) {
    
    if (!engine || !premise_atom_id || !premise_tv) return false;
    
    // Get implication and conclusion atoms
    opencog_atom_t* implication = opencog_get_atom(engine->atomspace, implication_atom_id);
    opencog_atom_t* conclusion = opencog_get_atom(engine->atomspace, conclusion_atom_id);
    
    if (!implication || !conclusion || 
        implication->type != OPENCOG_IMPLICATION_LINK) {
        engine->failed_inferences++;
        return false;
    }
    
    // Extract premise from implication
    if (implication->outgoing_count < 2) {
        engine->failed_inferences++;
        return false;
    }
    
    *premise_atom_id = implication->outgoing[0];
    
    // Apply PLN induction formula
    *premise_tv = pln_induction_formula(implication->truth_value, conclusion->truth_value);
    
    // Set truth value for premise atom
    opencog_set_truth_value(engine->atomspace, *premise_atom_id, 
                           premise_tv->strength, premise_tv->confidence);
    
    if (premise_tv->confidence < engine->default_confidence_threshold) {
        engine->failed_inferences++;
        return false;
    }
    
    engine->successful_inferences++;
    engine->total_inferences++;
    
    printf("PLN Induction: Inferred atom %lu with strength %.2f, confidence %.2f\n",
           *premise_atom_id, premise_tv->strength, premise_tv->confidence);
    
    return true;
}

// Apply abduction: A → B, B ⊢ A (likely explanation)
bool pln_abduction(
    pln_reasoning_engine_t* engine,
    uint64_t implication_atom_id,
    uint64_t observation_atom_id,
    uint64_t* explanation_atom_id,
    opencog_truth_value_t* explanation_tv) {
    
    if (!engine || !explanation_atom_id || !explanation_tv) return false;
    
    // Get implication and observation atoms
    opencog_atom_t* implication = opencog_get_atom(engine->atomspace, implication_atom_id);
    opencog_atom_t* observation = opencog_get_atom(engine->atomspace, observation_atom_id);
    
    if (!implication || !observation || 
        implication->type != OPENCOG_IMPLICATION_LINK) {
        engine->failed_inferences++;
        return false;
    }
    
    // Extract explanation from implication
    if (implication->outgoing_count < 2) {
        engine->failed_inferences++;
        return false;
    }
    
    *explanation_atom_id = implication->outgoing[0];
    
    // Apply PLN abduction formula
    *explanation_tv = pln_abduction_formula(implication->truth_value, observation->truth_value);
    
    // Set truth value for explanation atom
    opencog_set_truth_value(engine->atomspace, *explanation_atom_id, 
                           explanation_tv->strength, explanation_tv->confidence);
    
    if (explanation_tv->confidence < engine->default_confidence_threshold * 0.8f) {
        engine->failed_inferences++;
        return false;
    }
    
    engine->successful_inferences++;
    engine->total_inferences++;
    
    printf("PLN Abduction: Explained with atom %lu, strength %.2f, confidence %.2f\n",
           *explanation_atom_id, explanation_tv->strength, explanation_tv->confidence);
    
    return true;
}

// Belief revision with new evidence
bool pln_belief_revision(
    pln_reasoning_engine_t* engine,
    uint64_t belief_atom_id,
    uint64_t evidence_atom_id,
    opencog_truth_value_t* revised_tv) {
    
    if (!engine || !revised_tv) return false;
    
    opencog_atom_t* belief = opencog_get_atom(engine->atomspace, belief_atom_id);
    opencog_atom_t* evidence = opencog_get_atom(engine->atomspace, evidence_atom_id);
    
    if (!belief || !evidence) {
        engine->failed_inferences++;
        return false;
    }
    
    opencog_truth_value_t belief_tv = belief->truth_value;
    opencog_truth_value_t evidence_tv = evidence->truth_value;
    
    // PLN belief revision formula
    float weight_old = belief_tv.confidence;
    float weight_new = evidence_tv.confidence * engine->revision_factor;
    float total_weight = weight_old + weight_new;
    
    if (total_weight < 0.01f) {
        engine->failed_inferences++;
        return false;
    }
    
    revised_tv->strength = (belief_tv.strength * weight_old + evidence_tv.strength * weight_new) / total_weight;
    revised_tv->confidence = fminf(1.0f, total_weight);
    revised_tv->count = belief_tv.count + evidence_tv.count;
    
    // Update belief atom
    opencog_set_truth_value(engine->atomspace, belief_atom_id, 
                           revised_tv->strength, revised_tv->confidence);
    
    engine->successful_inferences++;
    engine->total_inferences++;
    
    printf("PLN Belief Revision: Updated atom %lu to strength %.2f, confidence %.2f\n",
           belief_atom_id, revised_tv->strength, revised_tv->confidence);
    
    return true;
}

// Multi-step inference chain
bool pln_inference_chain(
    pln_reasoning_engine_t* engine,
    pln_inference_context_t* context) {
    
    if (!engine || !context || context->premise_count == 0) return false;
    
    bool success = true;
    uint64_t current_conclusion = 0;
    opencog_truth_value_t current_tv = {0.0f, 0.0f, 0.0f};
    
    context->chain_length = 0;
    
    // Apply the inference rule based on context
    switch (context->rule_type) {
        case PLN_RULE_MODUS_PONENS:
            if (context->premise_count >= 2) {
                success = pln_modus_ponens(engine,
                    context->premises[0].atom_id,
                    context->premises[1].atom_id,
                    &current_conclusion,
                    &current_tv);
            }
            break;
            
        case PLN_RULE_DEDUCTION:
            if (context->premise_count >= 2) {
                success = pln_deduction(engine,
                    context->premises[0].atom_id,
                    context->premises[1].atom_id,
                    &current_conclusion,
                    &current_tv);
            }
            break;
            
        case PLN_RULE_INDUCTION:
            if (context->premise_count >= 2) {
                success = pln_induction(engine,
                    context->premises[0].atom_id,
                    context->premises[1].atom_id,
                    &current_conclusion,
                    &current_tv);
            }
            break;
            
        case PLN_RULE_ABDUCTION:
            if (context->premise_count >= 2) {
                success = pln_abduction(engine,
                    context->premises[0].atom_id,
                    context->premises[1].atom_id,
                    &current_conclusion,
                    &current_tv);
            }
            break;
            
        case PLN_RULE_REVISION:
            if (context->premise_count >= 2) {
                success = pln_belief_revision(engine,
                    context->premises[0].atom_id,
                    context->premises[1].atom_id,
                    &current_tv);
                current_conclusion = context->premises[0].atom_id;
            }
            break;
            
        default:
            success = false;
            break;
    }
    
    if (success) {
        context->conclusion_atom_id = current_conclusion;
        context->conclusion_truth_value = current_tv;
        context->inference_confidence = current_tv.confidence;
        
        // Add to inference chain
        if (context->chain_length < PLN_MAX_INFERENCE_CHAIN) {
            context->inference_chain[context->chain_length++] = current_conclusion;
        }
        
        // Store in history
        if (engine->history_count < engine->history_capacity) {
            engine->inference_history[engine->history_count++] = *context;
        }
    }
    
    return success;
}

// Get reasoning statistics
void pln_get_statistics(
    pln_reasoning_engine_t* engine,
    float* success_rate,
    float* average_confidence,
    uint64_t* total_inferences) {
    
    if (!engine) return;
    
    if (success_rate) {
        *success_rate = (engine->total_inferences > 0) ? 
            (float)engine->successful_inferences / (float)engine->total_inferences : 0.0f;
    }
    
    if (average_confidence) {
        *average_confidence = engine->average_confidence;
    }
    
    if (total_inferences) {
        *total_inferences = engine->total_inferences;
    }
}

// Create inference context
pln_inference_context_t* pln_create_inference_context(
    pln_inference_rule_t rule_type,
    uint64_t* premise_atoms,
    size_t premise_count) {
    
    if (!premise_atoms || premise_count == 0 || premise_count > PLN_MAX_PREMISES) {
        return NULL;
    }
    
    pln_inference_context_t* context = malloc(sizeof(pln_inference_context_t));
    if (!context) return NULL;
    
    memset(context, 0, sizeof(pln_inference_context_t));
    
    context->rule_type = rule_type;
    context->premise_count = premise_count;
    
    for (size_t i = 0; i < premise_count; i++) {
        context->premises[i].atom_id = premise_atoms[i];
        context->premises[i].attention_weight = 1.0f;
        context->premises[i].temporal_factor = 1.0f;
    }
    
    context->confidence_threshold = 0.6f;
    context->strength_threshold = 0.5f;
    context->attention_threshold = 0.1f;
    context->inference_time = (uint64_t)time(NULL);
    context->temporal_decay_factor = 0.95f;
    context->use_attention_weights = false;
    context->attention_amplification = 1.0f;
    
    return context;
}

// Free inference context
void pln_free_inference_context(pln_inference_context_t* context) {
    if (context) {
        free(context);
    }
}

// Print reasoning engine state
void pln_print_engine_state(pln_reasoning_engine_t* engine) {
    if (!engine) return;
    
    printf("\n=== PLN Reasoning Engine State ===\n");
    printf("Initialized: %s\n", engine->initialized ? "Yes" : "No");
    printf("Total inferences: %lu\n", engine->total_inferences);
    printf("Successful inferences: %lu\n", engine->successful_inferences);
    printf("Failed inferences: %lu\n", engine->failed_inferences);
    
    if (engine->total_inferences > 0) {
        float success_rate = (float)engine->successful_inferences / (float)engine->total_inferences;
        printf("Success rate: %.2f%%\n", success_rate * 100.0f);
    }
    
    printf("History entries: %zu/%zu\n", engine->history_count, engine->history_capacity);
    printf("Confidence threshold: %.2f\n", engine->default_confidence_threshold);
    printf("Strength threshold: %.2f\n", engine->default_strength_threshold);
    printf("===================================\n\n");
}

// Convert PLN inference to cognitive tensor
struct ggml_tensor* pln_inference_to_tensor(
    struct ggml_context* ctx,
    pln_reasoning_engine_t* engine,
    pln_inference_context_t* inference) {
    
    if (!ctx || !engine || !inference) return NULL;
    
    // Create tensor encoding of inference context
    int64_t dims[2] = {(int64_t)inference->premise_count + 1, 4}; // premises + conclusion, 4 features each
    struct ggml_tensor* tensor = ggml_new_tensor_2d(ctx, GGML_TYPE_F32, dims[0], dims[1]);
    
    if (!tensor) return NULL;
    
    float* data = (float*)tensor->data;
    
    // Encode premises
    for (size_t i = 0; i < inference->premise_count; i++) {
        size_t offset = i * 4;
        data[offset + 0] = (float)inference->premises[i].atom_id;
        data[offset + 1] = inference->premises[i].truth_value.strength;
        data[offset + 2] = inference->premises[i].truth_value.confidence;
        data[offset + 3] = inference->premises[i].attention_weight;
    }
    
    // Encode conclusion
    size_t conclusion_offset = inference->premise_count * 4;
    data[conclusion_offset + 0] = (float)inference->conclusion_atom_id;
    data[conclusion_offset + 1] = inference->conclusion_truth_value.strength;
    data[conclusion_offset + 2] = inference->conclusion_truth_value.confidence;
    data[conclusion_offset + 3] = inference->inference_confidence;
    
    return tensor;
}