#include "ggml-opencog.h"
#include "ggml-cogutil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

// Global type registry for OpenCog types
static cogutil_type_registry_t g_opencog_type_registry;
static bool g_opencog_types_initialized = false;

// Initialize OpenCog type system using cogutil
static cogutil_error_t initialize_opencog_types(void) {
    if (g_opencog_types_initialized) return COGUTIL_SUCCESS;
    
    cogutil_error_t result = cogutil_type_registry_init(&g_opencog_type_registry);
    if (result != COGUTIL_SUCCESS) {
        COGUTIL_LOG_ERROR("Failed to initialize OpenCog type registry: %s", 
                         cogutil_error_message(result));
        return result;
    }
    
    // Register OpenCog atom types
    uint16_t type_id;
    cogutil_type_register(&g_opencog_type_registry, "ConceptNode", true, false, 1, &type_id);
    cogutil_type_register(&g_opencog_type_registry, "PredicateNode", true, false, 1, &type_id);
    cogutil_type_register(&g_opencog_type_registry, "VariableNode", true, false, 1, &type_id);
    cogutil_type_register(&g_opencog_type_registry, "InheritanceLink", false, true, 2, &type_id);
    cogutil_type_register(&g_opencog_type_registry, "EvaluationLink", false, true, 2, &type_id);
    cogutil_type_register(&g_opencog_type_registry, "ImplicationLink", false, true, 2, &type_id);
    cogutil_type_register(&g_opencog_type_registry, "SimilarityLink", false, true, 2, &type_id);
    cogutil_type_register(&g_opencog_type_registry, "MemberLink", false, true, 2, &type_id);
    
    g_opencog_types_initialized = true;
    COGUTIL_LOG_INFO("OpenCog type system initialized with %zu types", 
                     g_opencog_type_registry.type_count);
    
    return COGUTIL_SUCCESS;
}

// Generate unique atom ID
static uint64_t generate_atom_id(opencog_atomspace_t* atomspace) {
    return atomspace->next_atom_id++;
}

// Initialize OpenCog AtomSpace
opencog_atomspace_t* opencog_atomspace_init(struct ggml_context* ctx) {
    // Validate input parameters
    cogutil_error_t validation_result = cogutil_validate_pointer(ctx);
    if (validation_result != COGUTIL_SUCCESS) {
        COGUTIL_LOG_ERROR("Invalid context pointer: %s", cogutil_error_message(validation_result));
        return NULL;
    }
    
    // Initialize logging if not already done
    static bool log_initialized = false;
    if (!log_initialized) {
        cogutil_log_init(COGUTIL_LOG_INFO, NULL);
        log_initialized = true;
    }
    
    // Initialize OpenCog type system
    cogutil_error_t type_init_result = initialize_opencog_types();
    if (type_init_result != COGUTIL_SUCCESS) {
        COGUTIL_LOG_ERROR("Failed to initialize OpenCog types: %s", 
                         cogutil_error_message(type_init_result));
        return NULL;
    }
    
    opencog_atomspace_t* atomspace = malloc(sizeof(opencog_atomspace_t));
    if (!atomspace) {
        COGUTIL_LOG_ERROR("Memory allocation failed for AtomSpace");
        return NULL;
    }
    
    atomspace->ctx = ctx;
    
    // Initialize atom storage
    atomspace->atom_capacity = OPENCOG_MAX_ATOMS;
    atomspace->atoms = calloc(atomspace->atom_capacity, sizeof(opencog_atom_t));
    if (!atomspace->atoms) {
        COGUTIL_LOG_ERROR("Memory allocation failed for atom storage");
        free(atomspace);
        return NULL;
    }
    
    atomspace->atom_count = 0;
    atomspace->next_atom_id = 1;
    
    // Initialize ECAN parameters
    atomspace->attention_decay_rate = 0.95f;
    atomspace->attention_threshold = 0.1f;
    atomspace->importance_diffusion_rate = 0.1f;
    
    // Initialize PLN parameters
    atomspace->default_strength = 0.8f;
    atomspace->default_confidence = 0.9f;
    
    // Initialize performance metrics
    atomspace->total_inferences = 0;
    atomspace->successful_inferences = 0;
    atomspace->reasoning_accuracy = 0.0f;
    
    atomspace->initialized = true;
    atomspace->cogfluence_system = NULL;
    
    COGUTIL_LOG_INFO("OpenCog AtomSpace initialized with capacity for %zu atoms", 
                     atomspace->atom_capacity);
    
    return atomspace;
}

// Free OpenCog AtomSpace
void opencog_atomspace_free(opencog_atomspace_t* atomspace) {
    if (!atomspace) return;
    
    // Free atom storage
    for (size_t i = 0; i < atomspace->atom_count; i++) {
        if (atomspace->atoms[i].outgoing) {
            free(atomspace->atoms[i].outgoing);
        }
        if (atomspace->atoms[i].incoming) {
            free(atomspace->atoms[i].incoming);
        }
    }
    
    free(atomspace->atoms);
    free(atomspace);
}

// Add node to AtomSpace
uint64_t opencog_add_node(
    opencog_atomspace_t* atomspace,
    opencog_atom_type_t type,
    const char* name) {
    
    // Validate input parameters
    cogutil_error_t validation_result = cogutil_validate_pointer(atomspace);
    if (validation_result != COGUTIL_SUCCESS) {
        COGUTIL_LOG_ERROR("Invalid atomspace pointer: %s", cogutil_error_message(validation_result));
        return 0;
    }
    
    validation_result = cogutil_validate_string(name);
    if (validation_result != COGUTIL_SUCCESS) {
        COGUTIL_LOG_ERROR("Invalid atom name: %s", cogutil_error_message(validation_result));
        return 0;
    }
    
    if (atomspace->atom_count >= atomspace->atom_capacity) {
        COGUTIL_LOG_ERROR("AtomSpace capacity exceeded: %zu/%zu", 
                         atomspace->atom_count, atomspace->atom_capacity);
        return 0;
    }
    
    opencog_atom_t* atom = &atomspace->atoms[atomspace->atom_count];
    uint64_t atom_id = generate_atom_id(atomspace);
    
    // Initialize atom
    atom->atom_id = atom_id;
    cogutil_error_t copy_result = cogutil_safe_strcpy(atom->name, OPENCOG_MAX_ATOM_NAME, name);
    if (copy_result != COGUTIL_SUCCESS) {
        COGUTIL_LOG_WARN("Atom name truncated: %s", name);
        strncpy(atom->name, name, OPENCOG_MAX_ATOM_NAME - 1);
        atom->name[OPENCOG_MAX_ATOM_NAME - 1] = '\0';
    }
    atom->type = type;
    
    // Initialize truth value
    atom->truth_value.strength = atomspace->default_strength;
    atom->truth_value.confidence = atomspace->default_confidence;
    atom->truth_value.count = 1.0f;
    
    // Initialize attention value
    atom->attention_value.sti = 0.0f;
    atom->attention_value.lti = 0.0f;
    atom->attention_value.vlti = 0.0f;
    
    // Create tensor encoding
    atom->tensor_encoding = ggml_new_tensor_1d(atomspace->ctx, GGML_TYPE_F32, 128);
    ggml_set_zero(atom->tensor_encoding);
    
    // Initialize name-based encoding using cogutil hash
    if (atom->tensor_encoding->type == GGML_TYPE_F32) {
        float* data = (float*)atom->tensor_encoding->data;
        uint32_t name_hash = cogutil_hash_string(name);
        
        // Simple hash-based encoding
        for (int i = 0; i < 128; i++) {
            data[i] = ((float)((name_hash >> (i % 32)) & 1)) * 0.5f - 0.25f;
        }
        
        // Mix in character-based encoding for first few dimensions
        size_t name_len = strlen(name);
        for (int i = 0; i < 32 && i < name_len; i++) {
            data[i] = (float)name[i] / 255.0f;
        }
    }
    
    // Initialize links
    atom->outgoing = NULL;
    atom->outgoing_count = 0;
    atom->outgoing_capacity = 0;
    atom->incoming = NULL;
    atom->incoming_count = 0;
    atom->incoming_capacity = 0;
    
    // Initialize metadata
    atom->creation_time = cogutil_get_timestamp_ms();
    atom->last_access = atom->creation_time;
    atom->is_deleted = false;
    atom->cogfluence_unit_id = 0;
    
    atomspace->atom_count++;
    
    COGUTIL_LOG_DEBUG("Added OpenCog node '%s' (type %d, ID %lu)", name, type, atom_id);
    
    return atom_id;
}

// Add link to AtomSpace
uint64_t opencog_add_link(
    opencog_atomspace_t* atomspace,
    opencog_atom_type_t type,
    uint64_t* outgoing,
    size_t outgoing_count) {
    
    if (!atomspace || !outgoing || outgoing_count == 0 || 
        atomspace->atom_count >= atomspace->atom_capacity) {
        return 0;
    }
    
    // Verify all outgoing atoms exist
    for (size_t i = 0; i < outgoing_count; i++) {
        if (!opencog_get_atom(atomspace, outgoing[i])) {
            return 0;
        }
    }
    
    opencog_atom_t* atom = &atomspace->atoms[atomspace->atom_count];
    uint64_t atom_id = generate_atom_id(atomspace);
    
    // Initialize atom
    atom->atom_id = atom_id;
    snprintf(atom->name, OPENCOG_MAX_ATOM_NAME, "Link_%lu", atom_id);
    atom->type = type;
    
    // Initialize truth value
    atom->truth_value.strength = atomspace->default_strength;
    atom->truth_value.confidence = atomspace->default_confidence;
    atom->truth_value.count = 1.0f;
    
    // Initialize attention value
    atom->attention_value.sti = 0.0f;
    atom->attention_value.lti = 0.0f;
    atom->attention_value.vlti = 0.0f;
    
    // Create tensor encoding (aggregate from outgoing)
    atom->tensor_encoding = ggml_new_tensor_1d(atomspace->ctx, GGML_TYPE_F32, 128);
    ggml_set_zero(atom->tensor_encoding);
    
    // Initialize outgoing links
    atom->outgoing_capacity = outgoing_count;
    atom->outgoing = malloc(atom->outgoing_capacity * sizeof(uint64_t));
    memcpy(atom->outgoing, outgoing, outgoing_count * sizeof(uint64_t));
    atom->outgoing_count = outgoing_count;
    
    // Initialize incoming links
    atom->incoming = NULL;
    atom->incoming_count = 0;
    atom->incoming_capacity = 0;
    
    // Add incoming links to outgoing atoms
    for (size_t i = 0; i < outgoing_count; i++) {
        opencog_atom_t* outgoing_atom = opencog_get_atom(atomspace, outgoing[i]);
        if (outgoing_atom) {
            if (outgoing_atom->incoming_count >= outgoing_atom->incoming_capacity) {
                outgoing_atom->incoming_capacity = outgoing_atom->incoming_capacity == 0 ? 
                    4 : outgoing_atom->incoming_capacity * 2;
                outgoing_atom->incoming = realloc(outgoing_atom->incoming,
                    outgoing_atom->incoming_capacity * sizeof(uint64_t));
            }
            outgoing_atom->incoming[outgoing_atom->incoming_count++] = atom_id;
        }
    }
    
    // Initialize metadata
    atom->creation_time = (uint64_t)time(NULL);
    atom->last_access = atom->creation_time;
    atom->is_deleted = false;
    atom->cogfluence_unit_id = 0;
    
    atomspace->atom_count++;
    
    printf("Added OpenCog link (type %d, ID %lu) with %zu outgoing atoms\n", 
           type, atom_id, outgoing_count);
    
    return atom_id;
}

// Get atom by ID
opencog_atom_t* opencog_get_atom(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id) {
    
    if (!atomspace || atom_id == 0) return NULL;
    
    // Linear search for now (could be optimized with hash table)
    for (size_t i = 0; i < atomspace->atom_count; i++) {
        if (atomspace->atoms[i].atom_id == atom_id && !atomspace->atoms[i].is_deleted) {
            atomspace->atoms[i].last_access = (uint64_t)time(NULL);
            return &atomspace->atoms[i];
        }
    }
    
    return NULL;
}

// PLN AND operation
opencog_truth_value_t opencog_pln_and(
    opencog_truth_value_t tv1,
    opencog_truth_value_t tv2) {
    
    opencog_truth_value_t result;
    
    // PLN AND formula: Min(s1, s2) with confidence combination
    result.strength = fminf(tv1.strength, tv2.strength);
    result.confidence = (tv1.confidence * tv2.confidence) / 
                       (tv1.confidence + tv2.confidence - tv1.confidence * tv2.confidence);
    result.count = fminf(tv1.count, tv2.count);
    
    return result;
}

// PLN OR operation
opencog_truth_value_t opencog_pln_or(
    opencog_truth_value_t tv1,
    opencog_truth_value_t tv2) {
    
    opencog_truth_value_t result;
    
    // PLN OR formula: Max(s1, s2) with confidence combination
    result.strength = fmaxf(tv1.strength, tv2.strength);
    result.confidence = (tv1.confidence * tv2.confidence) / 
                       (tv1.confidence + tv2.confidence - tv1.confidence * tv2.confidence);
    result.count = fmaxf(tv1.count, tv2.count);
    
    return result;
}

// PLN NOT operation
opencog_truth_value_t opencog_pln_not(opencog_truth_value_t tv) {
    opencog_truth_value_t result;
    
    result.strength = 1.0f - tv.strength;
    result.confidence = tv.confidence;
    result.count = tv.count;
    
    return result;
}

// PLN Implication operation (A → B)
opencog_truth_value_t opencog_pln_implication(
    opencog_truth_value_t premise,
    opencog_truth_value_t conclusion) {
    
    opencog_truth_value_t result;
    
    // PLN implication formula: if s_A is high, then s_B should be high
    // Strength of implication based on conditional probability
    float conditional_strength = 0.0f;
    if (premise.strength > 0.0f) {
        conditional_strength = conclusion.strength / premise.strength;
        conditional_strength = fminf(1.0f, conditional_strength);
    }
    
    result.strength = conditional_strength;
    result.confidence = fminf(premise.confidence, conclusion.confidence);
    result.count = fminf(premise.count, conclusion.count);
    
    return result;
}

// PLN Modus Ponens: (A, A→B) ⊢ B
opencog_truth_value_t opencog_pln_modus_ponens(
    opencog_truth_value_t premise_a,
    opencog_truth_value_t implication_ab) {
    
    opencog_truth_value_t result;
    
    // Modus ponens strength: s_A * s_(A→B)
    result.strength = premise_a.strength * implication_ab.strength;
    
    // Confidence combination with geometric mean
    result.confidence = sqrtf(premise_a.confidence * implication_ab.confidence);
    
    // Count as minimum of premises
    result.count = fminf(premise_a.count, implication_ab.count);
    
    return result;
}

// PLN Deduction: (A→B, B→C) ⊢ (A→C)
opencog_truth_value_t opencog_pln_deduction(
    opencog_truth_value_t ab_implication,
    opencog_truth_value_t bc_implication) {
    
    opencog_truth_value_t result;
    
    // Deduction strength: transitivity of implications
    result.strength = ab_implication.strength * bc_implication.strength;
    
    // Confidence weakens with chaining
    result.confidence = ab_implication.confidence * bc_implication.confidence * 0.9f;
    
    // Count combination
    result.count = fminf(ab_implication.count, bc_implication.count);
    
    return result;
}

// PLN Induction: (A→B, A) ⊢ B (with uncertainty)
opencog_truth_value_t opencog_pln_induction(
    opencog_truth_value_t specific_case,
    opencog_truth_value_t general_pattern) {
    
    opencog_truth_value_t result;
    
    // Induction strength based on pattern matching
    result.strength = specific_case.strength * general_pattern.strength * 0.8f; // Uncertainty factor
    
    // Lower confidence due to inductive uncertainty
    result.confidence = sqrtf(specific_case.confidence * general_pattern.confidence) * 0.7f;
    
    // Evidence count affects reliability
    result.count = (specific_case.count + general_pattern.count) / 2.0f;
    
    return result;
}

// PLN Abduction: (A→B, B) ⊢ A (hypothetical)
opencog_truth_value_t opencog_pln_abduction(
    opencog_truth_value_t implication_ab,
    opencog_truth_value_t conclusion_b) {
    
    opencog_truth_value_t result;
    
    // Abduction strength: reverse inference with uncertainty
    result.strength = conclusion_b.strength * implication_ab.strength * 0.6f; // High uncertainty
    
    // Low confidence due to hypothetical nature
    result.confidence = sqrtf(implication_ab.confidence * conclusion_b.confidence) * 0.5f;
    
    // Count reflects hypothesis quality
    result.count = fminf(implication_ab.count, conclusion_b.count) * 0.8f;
    
    return result;
}

// PLN Similarity inference: if A~B and B→C, then A→C (with strength reduction)
opencog_truth_value_t opencog_pln_similarity_inference(
    opencog_truth_value_t similarity_ab,
    opencog_truth_value_t implication_bc) {
    
    opencog_truth_value_t result;
    
    // Similarity-based inference with strength reduction
    result.strength = similarity_ab.strength * implication_bc.strength * 0.85f;
    
    // Confidence combination
    result.confidence = sqrtf(similarity_ab.confidence * implication_bc.confidence);
    
    // Count preservation
    result.count = fminf(similarity_ab.count, implication_bc.count);
    
    return result;
}

// Set truth value
void opencog_set_truth_value(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id,
    float strength,
    float confidence) {
    
    opencog_atom_t* atom = opencog_get_atom(atomspace, atom_id);
    if (!atom) return;
    
    atom->truth_value.strength = fmaxf(0.0f, fminf(1.0f, strength));
    atom->truth_value.confidence = fmaxf(0.0f, fminf(1.0f, confidence));
    atom->truth_value.count = 1.0f;
}

// Get truth value
opencog_truth_value_t opencog_get_truth_value(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id) {
    
    opencog_truth_value_t default_tv = {0.0f, 0.0f, 0.0f};
    
    opencog_atom_t* atom = opencog_get_atom(atomspace, atom_id);
    if (!atom) return default_tv;
    
    return atom->truth_value;
}

// Set attention value
void opencog_set_attention_value(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id,
    float sti,
    float lti,
    float vlti) {
    
    opencog_atom_t* atom = opencog_get_atom(atomspace, atom_id);
    if (!atom) return;
    
    atom->attention_value.sti = fmaxf(-1.0f, fminf(1.0f, sti));
    atom->attention_value.lti = fmaxf(0.0f, fminf(1.0f, lti));
    atom->attention_value.vlti = fmaxf(0.0f, fminf(1.0f, vlti));
}

// Get attention value
opencog_attention_value_t opencog_get_attention_value(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id) {
    
    opencog_attention_value_t default_av = {0.0f, 0.0f, 0.0f};
    
    opencog_atom_t* atom = opencog_get_atom(atomspace, atom_id);
    if (!atom) return default_av;
    
    return atom->attention_value;
}

// Update attention values (ECAN)
void opencog_update_attention_values(opencog_atomspace_t* atomspace) {
    if (!atomspace) return;
    
    // Attention decay
    for (size_t i = 0; i < atomspace->atom_count; i++) {
        if (atomspace->atoms[i].is_deleted) continue;
        
        opencog_atom_t* atom = &atomspace->atoms[i];
        
        // Apply decay
        atom->attention_value.sti *= atomspace->attention_decay_rate;
        atom->attention_value.lti *= atomspace->attention_decay_rate;
        
        // Convert STI to LTI over time
        if (atom->attention_value.sti > atomspace->attention_threshold) {
            float transfer = atom->attention_value.sti * 0.1f;
            atom->attention_value.lti += transfer;
            atom->attention_value.sti -= transfer;
        }
        
        // Clamp values
        atom->attention_value.sti = fmaxf(-1.0f, fminf(1.0f, atom->attention_value.sti));
        atom->attention_value.lti = fmaxf(0.0f, fminf(1.0f, atom->attention_value.lti));
        atom->attention_value.vlti = fmaxf(0.0f, fminf(1.0f, atom->attention_value.vlti));
    }
}

// Spread attention
void opencog_spread_attention(
    opencog_atomspace_t* atomspace,
    uint64_t source_atom_id,
    float amount) {
    
    opencog_atom_t* source_atom = opencog_get_atom(atomspace, source_atom_id);
    if (!source_atom) return;
    
    // Spread to outgoing atoms
    if (source_atom->outgoing_count > 0) {
        float spread_amount = amount / source_atom->outgoing_count;
        
        for (size_t i = 0; i < source_atom->outgoing_count; i++) {
            opencog_atom_t* target_atom = opencog_get_atom(atomspace, source_atom->outgoing[i]);
            if (target_atom) {
                target_atom->attention_value.sti += spread_amount;
                target_atom->attention_value.sti = fmaxf(-1.0f, fminf(1.0f, target_atom->attention_value.sti));
            }
        }
    }
    
    // Spread to incoming atoms
    if (source_atom->incoming_count > 0) {
        float spread_amount = amount / source_atom->incoming_count;
        
        for (size_t i = 0; i < source_atom->incoming_count; i++) {
            opencog_atom_t* target_atom = opencog_get_atom(atomspace, source_atom->incoming[i]);
            if (target_atom) {
                target_atom->attention_value.sti += spread_amount;
                target_atom->attention_value.sti = fmaxf(-1.0f, fminf(1.0f, target_atom->attention_value.sti));
            }
        }
    }
}

// Link with Cogfluence system
bool opencog_link_cogfluence(
    opencog_atomspace_t* atomspace,
    cogfluence_system_t* cogfluence_system) {
    
    if (!atomspace || !cogfluence_system) return false;
    
    atomspace->cogfluence_system = cogfluence_system;
    
    printf("Linked OpenCog AtomSpace with Cogfluence system\n");
    
    return true;
}

// Create atom from Cogfluence unit
uint64_t opencog_from_cogfluence_unit(
    opencog_atomspace_t* atomspace,
    cogfluence_knowledge_unit_t* unit) {
    
    if (!atomspace || !unit) return 0;
    
    // Map Cogfluence unit type to OpenCog atom type
    opencog_atom_type_t atom_type = OPENCOG_CONCEPT_NODE;
    switch (unit->type) {
        case COGFLUENCE_CONCEPT: atom_type = OPENCOG_CONCEPT_NODE; break;
        case COGFLUENCE_RELATION: atom_type = OPENCOG_INHERITANCE_LINK; break;
        case COGFLUENCE_RULE: atom_type = OPENCOG_IMPLICATION_LINK; break;
        default: atom_type = OPENCOG_CONCEPT_NODE; break;
    }
    
    uint64_t atom_id = opencog_add_node(atomspace, atom_type, unit->name);
    
    if (atom_id > 0) {
        opencog_atom_t* atom = opencog_get_atom(atomspace, atom_id);
        if (atom) {
            // Copy truth value
            atom->truth_value.strength = unit->truth_value;
            atom->truth_value.confidence = unit->confidence;
            
            // Copy attention value
            atom->attention_value.sti = unit->attention_value;
            atom->attention_value.lti = unit->activation_level;
            
            // Link back to Cogfluence unit
            atom->cogfluence_unit_id = unit->atomspace_id;
            
            // Copy tensor encoding
            if (unit->tensor_encoding) {
                atom->tensor_encoding = ggml_dup(atomspace->ctx, unit->tensor_encoding);
            }
            
            printf("Created OpenCog atom from Cogfluence unit '%s' (ID %lu)\n", 
                   unit->name, atom_id);
        }
    }
    
    return atom_id;
}

// Convert atom to tensor
struct ggml_tensor* opencog_atom_to_tensor(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id) {
    
    opencog_atom_t* atom = opencog_get_atom(atomspace, atom_id);
    if (!atom) return NULL;
    
    if (atom->tensor_encoding) {
        return ggml_dup(atomspace->ctx, atom->tensor_encoding);
    }
    
    // Create default tensor
    struct ggml_tensor* tensor = ggml_new_tensor_1d(atomspace->ctx, GGML_TYPE_F32, 128);
    ggml_set_zero(tensor);
    
    return tensor;
}

// Print atom
void opencog_print_atom(
    opencog_atomspace_t* atomspace,
    uint64_t atom_id) {
    
    opencog_atom_t* atom = opencog_get_atom(atomspace, atom_id);
    if (!atom) return;
    
    printf("Atom %lu: %s (type %d)\n", atom->atom_id, atom->name, atom->type);
    printf("  Truth: strength=%.2f, confidence=%.2f\n", 
           atom->truth_value.strength, atom->truth_value.confidence);
    printf("  Attention: sti=%.2f, lti=%.2f, vlti=%.2f\n",
           atom->attention_value.sti, atom->attention_value.lti, atom->attention_value.vlti);
    printf("  Outgoing: %zu, Incoming: %zu\n", 
           atom->outgoing_count, atom->incoming_count);
    
    if (atom->cogfluence_unit_id > 0) {
        printf("  Cogfluence unit: %lu\n", atom->cogfluence_unit_id);
    }
}

// Print AtomSpace statistics
void opencog_print_atomspace_statistics(opencog_atomspace_t* atomspace) {
    if (!atomspace) return;
    
    printf("\n=== OpenCog AtomSpace Statistics ===\n");
    printf("Atoms: %zu/%zu\n", atomspace->atom_count, atomspace->atom_capacity);
    printf("Total inferences: %lu\n", atomspace->total_inferences);
    printf("Successful inferences: %lu\n", atomspace->successful_inferences);
    printf("Reasoning accuracy: %.2f\n", atomspace->reasoning_accuracy);
    
    // Count atoms by type
    int type_counts[9] = {0};
    for (size_t i = 0; i < atomspace->atom_count; i++) {
        if (!atomspace->atoms[i].is_deleted && 
            atomspace->atoms[i].type >= 1 && atomspace->atoms[i].type <= 8) {
            type_counts[atomspace->atoms[i].type]++;
        }
    }
    
    printf("Atom types:\n");
    printf("  Concept nodes: %d\n", type_counts[OPENCOG_CONCEPT_NODE]);
    printf("  Predicate nodes: %d\n", type_counts[OPENCOG_PREDICATE_NODE]);
    printf("  Variable nodes: %d\n", type_counts[OPENCOG_VARIABLE_NODE]);
    printf("  Inheritance links: %d\n", type_counts[OPENCOG_INHERITANCE_LINK]);
    printf("  Evaluation links: %d\n", type_counts[OPENCOG_EVALUATION_LINK]);
    printf("  Implication links: %d\n", type_counts[OPENCOG_IMPLICATION_LINK]);
    printf("  Similarity links: %d\n", type_counts[OPENCOG_SIMILARITY_LINK]);
    printf("  Member links: %d\n", type_counts[OPENCOG_MEMBER_LINK]);
    
    // Average attention values
    float avg_sti = 0.0f, avg_lti = 0.0f;
    int active_atoms = 0;
    
    for (size_t i = 0; i < atomspace->atom_count; i++) {
        if (!atomspace->atoms[i].is_deleted) {
            avg_sti += atomspace->atoms[i].attention_value.sti;
            avg_lti += atomspace->atoms[i].attention_value.lti;
            active_atoms++;
        }
    }
    
    if (active_atoms > 0) {
        avg_sti /= active_atoms;
        avg_lti /= active_atoms;
        printf("Average attention: STI=%.2f, LTI=%.2f\n", avg_sti, avg_lti);
    }
    
    printf("=====================================\n");
}

// PLN Reasoning Engine Functions

// PLN forward chaining: apply rules to derive new knowledge
bool opencog_pln_forward_chain(
    opencog_atomspace_t* atomspace,
    uint64_t premise_atoms[],
    size_t premise_count,
    void* rule) {
    
    if (!atomspace || !premise_atoms || premise_count == 0) {
        return false;
    }
    
    // Simple forward chaining: for now just apply basic modus ponens pattern
    if (premise_count >= 2) {
        opencog_truth_value_t premise1_tv = opencog_get_truth_value(atomspace, premise_atoms[0]);
        opencog_truth_value_t premise2_tv = opencog_get_truth_value(atomspace, premise_atoms[1]);
        
        // Apply modus ponens if appropriate
        opencog_truth_value_t conclusion_tv = opencog_pln_modus_ponens(premise1_tv, premise2_tv);
        
        // Create conclusion atom if rule application was successful
        if (conclusion_tv.confidence > 0.1f) { // Minimum confidence threshold
            char conclusion_name[OPENCOG_MAX_ATOM_NAME];
            snprintf(conclusion_name, OPENCOG_MAX_ATOM_NAME, "Inferred_%lu", 
                     atomspace->total_inferences);
            
            uint64_t conclusion_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, conclusion_name);
            if (conclusion_id > 0) {
                opencog_set_truth_value(atomspace, conclusion_id, 
                                        conclusion_tv.strength, conclusion_tv.confidence);
                
                atomspace->total_inferences++;
                if (conclusion_tv.strength > 0.7f && conclusion_tv.confidence > 0.7f) {
                    atomspace->successful_inferences++;
                }
                
                atomspace->reasoning_accuracy = (float)atomspace->successful_inferences / 
                                               (float)atomspace->total_inferences;
                
                return true;
            }
        }
    }
    
    return false;
}

// PLN backward chaining: find premises that could lead to a goal
bool opencog_pln_backward_chain(
    opencog_atomspace_t* atomspace,
    uint64_t goal_atom_id,
    opencog_truth_value_t desired_tv) {
    
    if (!atomspace || goal_atom_id == 0) return false;
    
    opencog_atom_t* goal_atom = opencog_get_atom(atomspace, goal_atom_id);
    if (!goal_atom) return false;
    
    // Simple backward chaining: look for atoms that could imply the goal
    for (size_t i = 0; i < atomspace->atom_count; i++) {
        if (atomspace->atoms[i].is_deleted) continue;
        
        opencog_atom_t* potential_premise = &atomspace->atoms[i];
        
        // Check if this atom could be a premise for the goal
        if (potential_premise->type == OPENCOG_IMPLICATION_LINK ||
            potential_premise->type == OPENCOG_INHERITANCE_LINK) {
            
            // Check if this implication could lead to our goal
            for (size_t j = 0; j < potential_premise->outgoing_count; j++) {
                if (potential_premise->outgoing[j] == goal_atom_id) {
                    // Found a potential reasoning path
                    opencog_truth_value_t premise_tv = potential_premise->truth_value;
                    
                    // Apply modus ponens if we can establish the antecedent
                    if (premise_tv.strength >= desired_tv.strength * 0.8f &&
                        premise_tv.confidence >= desired_tv.confidence * 0.8f) {
                        
                        // Update goal atom's truth value
                        opencog_truth_value_t new_tv = opencog_pln_modus_ponens(
                            premise_tv, potential_premise->truth_value);
                        
                        opencog_set_truth_value(atomspace, goal_atom_id, 
                                                new_tv.strength, new_tv.confidence);
                        
                        atomspace->total_inferences++;
                        if (new_tv.strength >= desired_tv.strength) {
                            atomspace->successful_inferences++;
                        }
                        
                        atomspace->reasoning_accuracy = (float)atomspace->successful_inferences / 
                                                       (float)atomspace->total_inferences;
                        
                        return true;
                    }
                }
            }
        }
    }
    
    return false;
}

// PLN pattern matching for rule discovery
size_t opencog_pln_find_patterns(
    opencog_atomspace_t* atomspace,
    opencog_atom_type_t pattern_type,
    uint64_t* matching_atoms,
    size_t max_matches) {
    
    if (!atomspace || !matching_atoms) return 0;
    
    size_t match_count = 0;
    
    for (size_t i = 0; i < atomspace->atom_count && match_count < max_matches; i++) {
        if (atomspace->atoms[i].is_deleted) continue;
        
        if (atomspace->atoms[i].type == pattern_type) {
            matching_atoms[match_count++] = atomspace->atoms[i].atom_id;
        }
    }
    
    return match_count;
}

// PLN inference session management
// (Structure definition is in the header file)

// Initialize PLN reasoning session
opencog_pln_session_t* opencog_pln_session_init(
    opencog_atomspace_t* atomspace,
    float accuracy_threshold) {
    
    if (!atomspace) return NULL;
    
    opencog_pln_session_t* session = malloc(sizeof(opencog_pln_session_t));
    if (!session) return NULL;
    
    session->atomspace = atomspace;
    session->inference_count = 0;
    session->successful_count = 0;
    session->accuracy_threshold = accuracy_threshold;
    session->forward_chaining_enabled = true;
    session->backward_chaining_enabled = true;
    
    printf("PLN reasoning session initialized with accuracy threshold: %.2f\n", 
           accuracy_threshold);
    
    return session;
}

// Run PLN inference cycle
bool opencog_pln_inference_cycle(opencog_pln_session_t* session) {
    if (!session || !session->atomspace) return false;
    
    bool made_inference = false;
    
    // Forward chaining: look for applicable rules
    if (session->forward_chaining_enabled) {
        // Find implication links to apply modus ponens
        uint64_t implications[64];
        size_t impl_count = opencog_pln_find_patterns(
            session->atomspace, OPENCOG_IMPLICATION_LINK, implications, 64);
        
        for (size_t i = 0; i < impl_count; i++) {
            opencog_atom_t* impl_atom = opencog_get_atom(session->atomspace, implications[i]);
            if (!impl_atom || impl_atom->outgoing_count < 2) continue;
            
            // Look for antecedent in atomspace
            uint64_t antecedent_id = impl_atom->outgoing[0];
            uint64_t consequent_id = impl_atom->outgoing[1];
            
            opencog_atom_t* antecedent = opencog_get_atom(session->atomspace, antecedent_id);
            if (antecedent && antecedent->truth_value.strength > 0.6f) {
                // Apply modus ponens
                opencog_truth_value_t conclusion = opencog_pln_modus_ponens(
                    antecedent->truth_value, impl_atom->truth_value);
                
                opencog_set_truth_value(session->atomspace, consequent_id,
                                        conclusion.strength, conclusion.confidence);
                
                session->inference_count++;
                if (conclusion.strength > session->accuracy_threshold) {
                    session->successful_count++;
                }
                
                made_inference = true;
            }
        }
    }
    
    return made_inference;
}

// Free PLN reasoning session
void opencog_pln_session_free(opencog_pln_session_t* session) {
    if (session) {
        printf("PLN session completed: %lu inferences, %lu successful (%.2f%% accuracy)\n",
               session->inference_count, session->successful_count,
               session->inference_count > 0 ? 
               (float)session->successful_count / (float)session->inference_count * 100.0f : 0.0f);
        free(session);
    }
}