#include "ggml-pattern-matching.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

// Initialize pattern matching engine
pattern_engine_t* pattern_engine_init(
    struct ggml_context* ctx,
    opencog_atomspace_t* atomspace,
    pln_reasoning_engine_t* pln_engine,
    ggml_cognitive_kernel_t* cognitive_kernel) {
    
    if (!ctx) return NULL;
    
    pattern_engine_t* engine = malloc(sizeof(pattern_engine_t));
    if (!engine) return NULL;
    
    memset(engine, 0, sizeof(pattern_engine_t));
    
    engine->ctx = ctx;
    engine->atomspace = atomspace;
    engine->pln_engine = pln_engine;
    engine->cognitive_kernel = cognitive_kernel;
    
    // Initialize template storage
    engine->template_capacity = PATTERN_MAX_TEMPLATES;
    engine->templates = calloc(engine->template_capacity, sizeof(pattern_template_t));
    if (!engine->templates) {
        free(engine);
        return NULL;
    }
    
    // Initialize match storage
    engine->match_capacity = PATTERN_MAX_MATCHES;
    engine->matches = calloc(engine->match_capacity, sizeof(pattern_match_t));
    if (!engine->matches) {
        free(engine->templates);
        free(engine);
        return NULL;
    }
    
    // Initialize cache
    engine->cache_capacity = 256;
    engine->cached_encodings = calloc(engine->cache_capacity, sizeof(struct ggml_tensor*));
    
    // Set default configuration
    engine->global_similarity_threshold = 0.7f;
    engine->global_confidence_threshold = 0.6f;
    engine->use_parallel_matching = false;
    engine->max_concurrent_matches = 4;
    
    engine->initialized = true;
    engine->last_update_time = (uint64_t)time(NULL);
    
    printf("Pattern matching engine initialized with %zu template capacity\n", 
           engine->template_capacity);
    
    return engine;
}

// Free pattern matching engine
void pattern_engine_free(pattern_engine_t* engine) {
    if (!engine) return;
    
    // Free templates
    if (engine->templates) {
        for (size_t i = 0; i < engine->template_count; i++) {
            pattern_free_template(&engine->templates[i]);
        }
        free(engine->templates);
    }
    
    // Free matches
    if (engine->matches) {
        for (size_t i = 0; i < engine->match_count; i++) {
            if (engine->matches[i].matched_atom_ids) {
                free(engine->matches[i].matched_atom_ids);
            }
        }
        free(engine->matches);
    }
    
    // Free cache
    if (engine->cached_encodings) {
        free(engine->cached_encodings);
    }
    
    free(engine);
}

// Generate unique ID
static uint64_t generate_unique_id(void) {
    static uint64_t counter = 1;
    return counter++;
}

// Create pattern template
pattern_template_t* pattern_create_template(
    pattern_engine_t* engine,
    const char* name,
    pattern_type_t type,
    pattern_algorithm_t algorithm) {
    
    if (!engine || !name || engine->template_count >= engine->template_capacity) {
        return NULL;
    }
    
    pattern_template_t* template = &engine->templates[engine->template_count];
    memset(template, 0, sizeof(pattern_template_t));
    
    template->template_id = generate_unique_id();
    strncpy(template->name, name, sizeof(template->name) - 1);
    template->type = type;
    template->algorithm = algorithm;
    
    // Initialize node storage
    template->node_capacity = PATTERN_MAX_NODES;
    template->nodes = calloc(template->node_capacity, sizeof(pattern_node_t));
    if (!template->nodes) {
        return NULL;
    }
    
    // Initialize edge storage
    template->edge_capacity = PATTERN_MAX_EDGES;
    template->edges = calloc(template->edge_capacity, sizeof(pattern_edge_t));
    if (!template->edges) {
        free(template->nodes);
        return NULL;
    }
    
    // Set default parameters
    template->similarity_threshold = engine->global_similarity_threshold;
    template->confidence_threshold = engine->global_confidence_threshold;
    template->use_attention_weighting = true;
    template->use_temporal_constraints = false;
    
    engine->template_count++;
    
    printf("Created pattern template '%s' (ID: %lu, type: %d, algorithm: %d)\n",
           name, template->template_id, type, algorithm);
    
    return template;
}

// Add node to pattern template
bool pattern_add_node(
    pattern_template_t* template,
    const char* label,
    pattern_type_t type,
    bool is_variable,
    const char* variable_name) {
    
    if (!template || !label || template->node_count >= template->node_capacity) {
        return false;
    }
    
    pattern_node_t* node = &template->nodes[template->node_count];
    memset(node, 0, sizeof(pattern_node_t));
    
    node->node_id = generate_unique_id();
    strncpy(node->label, label, sizeof(node->label) - 1);
    node->type = type;
    node->is_variable = is_variable;
    
    if (is_variable && variable_name) {
        strncpy(node->variable_name, variable_name, sizeof(node->variable_name) - 1);
        
        // Add to template's variable list
        if (template->variable_count < PATTERN_MAX_VARIABLES) {
            strncpy(template->variables[template->variable_count], variable_name, 31);
            template->variable_count++;
        }
    }
    
    // Set default values
    node->similarity_threshold = 0.8f;
    node->truth_value.strength = 0.8f;
    node->truth_value.confidence = 0.9f;
    node->truth_value.count = 1.0f;
    
    template->node_count++;
    
    return true;
}

// Add edge to pattern template
bool pattern_add_edge(
    pattern_template_t* template,
    uint64_t source_node_id,
    uint64_t target_node_id,
    const char* relation_type,
    float weight) {
    
    if (!template || !relation_type || template->edge_count >= template->edge_capacity) {
        return false;
    }
    
    // Verify nodes exist
    bool source_found = false, target_found = false;
    for (size_t i = 0; i < template->node_count; i++) {
        if (template->nodes[i].node_id == source_node_id) source_found = true;
        if (template->nodes[i].node_id == target_node_id) target_found = true;
    }
    
    if (!source_found || !target_found) {
        return false;
    }
    
    pattern_edge_t* edge = &template->edges[template->edge_count];
    memset(edge, 0, sizeof(pattern_edge_t));
    
    edge->edge_id = generate_unique_id();
    edge->source_node_id = source_node_id;
    edge->target_node_id = target_node_id;
    strncpy(edge->relation_type, relation_type, sizeof(edge->relation_type) - 1);
    edge->weight = weight;
    edge->is_directed = true;
    
    // Set default constraints
    edge->min_weight = 0.0f;
    edge->max_weight = 1.0f;
    edge->weight_constraint_active = false;
    
    template->edge_count++;
    
    return true;
}

// Set pattern constraints
bool pattern_set_constraints(
    pattern_template_t* template,
    float similarity_threshold,
    float confidence_threshold,
    bool use_attention_weighting) {
    
    if (!template) return false;
    
    template->similarity_threshold = fmaxf(0.0f, fminf(1.0f, similarity_threshold));
    template->confidence_threshold = fmaxf(0.0f, fminf(1.0f, confidence_threshold));
    template->use_attention_weighting = use_attention_weighting;
    
    return true;
}

// Calculate cosine similarity between tensors
static float calculate_tensor_similarity(struct ggml_tensor* a, struct ggml_tensor* b) {
    if (!a || !b || !ggml_are_same_shape(a, b)) {
        return 0.0f;
    }
    
    float* data_a = (float*)a->data;
    float* data_b = (float*)b->data;
    size_t size = ggml_nelements(a);
    
    float dot_product = 0.0f;
    float norm_a = 0.0f;
    float norm_b = 0.0f;
    
    for (size_t i = 0; i < size; i++) {
        dot_product += data_a[i] * data_b[i];
        norm_a += data_a[i] * data_a[i];
        norm_b += data_b[i] * data_b[i];
    }
    
    norm_a = sqrtf(norm_a);
    norm_b = sqrtf(norm_b);
    
    if (norm_a < 1e-6f || norm_b < 1e-6f) {
        return 0.0f;
    }
    
    return dot_product / (norm_a * norm_b);
}

// Compile pattern template to tensor representation
bool pattern_compile_template(
    pattern_engine_t* engine,
    pattern_template_t* template) {
    
    if (!engine || !template) return false;
    
    // Create tensor encoding of the pattern structure
    // Dimension: [nodes, edges, features_per_element]
    int64_t dims[3] = {(int64_t)template->node_count, (int64_t)template->edge_count, 4};
    
    template->template_tensor = ggml_new_tensor_3d(engine->ctx, GGML_TYPE_F32, 
                                                   dims[0], dims[1], dims[2]);
    
    if (!template->template_tensor) return false;
    
    float* data = (float*)template->template_tensor->data;
    size_t offset = 0;
    
    // Encode nodes
    for (size_t i = 0; i < template->node_count; i++) {
        pattern_node_t* node = &template->nodes[i];
        
        data[offset++] = (float)node->node_id;
        data[offset++] = (float)node->type;
        data[offset++] = node->truth_value.strength;
        data[offset++] = node->is_variable ? 1.0f : 0.0f;
    }
    
    // Encode edges
    for (size_t i = 0; i < template->edge_count; i++) {
        pattern_edge_t* edge = &template->edges[i];
        
        data[offset++] = (float)edge->source_node_id;
        data[offset++] = (float)edge->target_node_id;
        data[offset++] = edge->weight;
        data[offset++] = edge->is_directed ? 1.0f : 0.0f;
    }
    
    printf("Compiled pattern template '%s' to tensor [%ld, %ld, %ld]\n",
           template->name, dims[0], dims[1], dims[2]);
    
    return true;
}

// Find exact structural matches
pattern_match_t* pattern_find_exact_matches(
    pattern_engine_t* engine,
    pattern_template_t* template,
    size_t* match_count) {
    
    if (!engine || !template || !match_count) {
        if (match_count) *match_count = 0;
        return NULL;
    }
    
    *match_count = 0;
    
    if (!engine->atomspace || engine->match_count >= engine->match_capacity) {
        return NULL;
    }
    
    // Simple exact matching implementation
    // In a full implementation, this would use graph isomorphism algorithms
    
    pattern_match_t* matches = malloc(sizeof(pattern_match_t) * 10); // Max 10 matches for demo
    if (!matches) return NULL;
    
    size_t found_matches = 0;
    
    // Simulate finding matches by checking AtomSpace
    for (size_t i = 0; i < engine->atomspace->atom_count && found_matches < 10; i++) {
        opencog_atom_t* atom = &engine->atomspace->atoms[i];
        
        if (atom->is_deleted) continue;
        
        // Simple heuristic: match based on atom type and truth value similarity
        bool potential_match = false;
        
        for (size_t j = 0; j < template->node_count; j++) {
            pattern_node_t* pattern_node = &template->nodes[j];
            
            // Check if atom could match this pattern node
            float tv_similarity = 1.0f - fabsf(atom->truth_value.strength - 
                                               pattern_node->truth_value.strength);
            
            if (tv_similarity >= template->similarity_threshold) {
                potential_match = true;
                break;
            }
        }
        
        if (potential_match) {
            pattern_match_t* match = &matches[found_matches];
            memset(match, 0, sizeof(pattern_match_t));
            
            match->match_id = generate_unique_id();
            match->template_id = template->template_id;
            match->match_confidence = 0.8f + (float)rand() / RAND_MAX * 0.2f; // Simulate confidence
            match->structural_similarity = 0.9f;
            match->semantic_similarity = 0.8f;
            match->match_timestamp = (uint64_t)time(NULL);
            
            // Simple binding - bind first variable to this atom
            if (template->variable_count > 0) {
                strncpy(match->variable_bindings[0].variable_name, 
                       template->variables[0], 31);
                match->variable_bindings[0].bound_atom_id = atom->atom_id;
                match->variable_bindings[0].binding_confidence = match->match_confidence;
                match->binding_count = 1;
            }
            
            // Store matched atom
            match->matched_atom_ids = malloc(sizeof(uint64_t));
            if (match->matched_atom_ids) {
                match->matched_atom_ids[0] = atom->atom_id;
                match->matched_count = 1;
            }
            
            found_matches++;
        }
    }
    
    *match_count = found_matches;
    
    // Update template statistics
    template->total_searches++;
    template->match_count += found_matches;
    if (found_matches > 0) {
        float total_confidence = 0.0f;
        for (size_t i = 0; i < found_matches; i++) {
            total_confidence += matches[i].match_confidence;
        }
        template->average_match_confidence = total_confidence / (float)found_matches;
    }
    
    // Update engine statistics
    engine->total_pattern_searches++;
    if (found_matches > 0) {
        engine->successful_matches++;
    }
    
    printf("Pattern '%s': Found %zu exact matches\n", template->name, found_matches);
    
    return matches;
}

// Find fuzzy similarity matches
pattern_match_t* pattern_find_fuzzy_matches(
    pattern_engine_t* engine,
    pattern_template_t* template,
    float similarity_threshold,
    size_t* match_count) {
    
    if (!engine || !template || !match_count) {
        if (match_count) *match_count = 0;
        return NULL;
    }
    
    *match_count = 0;
    
    if (!engine->atomspace) return NULL;
    
    pattern_match_t* matches = malloc(sizeof(pattern_match_t) * 20); // Max 20 fuzzy matches
    if (!matches) return NULL;
    
    size_t found_matches = 0;
    
    // Fuzzy matching with relaxed constraints
    for (size_t i = 0; i < engine->atomspace->atom_count && found_matches < 20; i++) {
        opencog_atom_t* atom = &engine->atomspace->atoms[i];
        
        if (atom->is_deleted) continue;
        
        // Calculate fuzzy similarity
        float max_similarity = 0.0f;
        
        for (size_t j = 0; j < template->node_count; j++) {
            pattern_node_t* pattern_node = &template->nodes[j];
            
            // Truth value similarity
            float tv_sim = 1.0f - fabsf(atom->truth_value.strength - 
                                       pattern_node->truth_value.strength);
            
            // Attention similarity (if available)
            float att_sim = 1.0f - fabsf(atom->attention_value.sti - 0.5f); // Assume 0.5 as default
            
            // Combined similarity
            float combined_sim = (tv_sim + att_sim) / 2.0f;
            
            if (combined_sim > max_similarity) {
                max_similarity = combined_sim;
            }
        }
        
        if (max_similarity >= similarity_threshold) {
            pattern_match_t* match = &matches[found_matches];
            memset(match, 0, sizeof(pattern_match_t));
            
            match->match_id = generate_unique_id();
            match->template_id = template->template_id;
            match->match_confidence = max_similarity;
            match->structural_similarity = max_similarity * 0.9f;
            match->semantic_similarity = max_similarity * 0.8f;
            match->match_timestamp = (uint64_t)time(NULL);
            
            // Store matched atom
            match->matched_atom_ids = malloc(sizeof(uint64_t));
            if (match->matched_atom_ids) {
                match->matched_atom_ids[0] = atom->atom_id;
                match->matched_count = 1;
            }
            
            found_matches++;
        }
    }
    
    *match_count = found_matches;
    
    printf("Pattern '%s': Found %zu fuzzy matches (threshold: %.2f)\n", 
           template->name, found_matches, similarity_threshold);
    
    return matches;
}

// Find tensor-based matches
pattern_match_t* pattern_find_tensor_matches(
    pattern_engine_t* engine,
    pattern_template_t* template,
    struct ggml_tensor* query_tensor,
    size_t* match_count) {
    
    if (!engine || !template || !query_tensor || !match_count) {
        if (match_count) *match_count = 0;
        return NULL;
    }
    
    *match_count = 0;
    
    if (!template->template_tensor) {
        // Compile template if not already done
        pattern_compile_template(engine, template);
    }
    
    if (!template->template_tensor) return NULL;
    
    // Calculate tensor similarity
    float similarity = calculate_tensor_similarity(template->template_tensor, query_tensor);
    
    if (similarity >= template->similarity_threshold) {
        pattern_match_t* match = malloc(sizeof(pattern_match_t));
        if (!match) return NULL;
        
        memset(match, 0, sizeof(pattern_match_t));
        
        match->match_id = generate_unique_id();
        match->template_id = template->template_id;
        match->match_confidence = similarity;
        match->structural_similarity = similarity;
        match->semantic_similarity = similarity * 0.9f;
        match->match_timestamp = (uint64_t)time(NULL);
        
        *match_count = 1;
        
        printf("Pattern '%s': Tensor match found with similarity %.3f\n", 
               template->name, similarity);
        
        return match;
    }
    
    return NULL;
}

// Calculate pattern similarity
float pattern_calculate_similarity(
    pattern_engine_t* engine,
    pattern_template_t* template1,
    pattern_template_t* template2) {
    
    if (!engine || !template1 || !template2) return 0.0f;
    
    // Structural similarity
    float node_ratio = (float)fmin(template1->node_count, template2->node_count) /
                       (float)fmax(template1->node_count, template2->node_count);
    
    float edge_ratio = (float)fmin(template1->edge_count, template2->edge_count) /
                       (float)fmax(template1->edge_count, template2->edge_count);
    
    float structural_sim = (node_ratio + edge_ratio) / 2.0f;
    
    // Type similarity
    float type_sim = (template1->type == template2->type) ? 1.0f : 0.5f;
    
    // Algorithm similarity
    float algo_sim = (template1->algorithm == template2->algorithm) ? 1.0f : 0.7f;
    
    // Tensor similarity (if both compiled)
    float tensor_sim = 0.5f; // Default
    if (template1->template_tensor && template2->template_tensor) {
        tensor_sim = calculate_tensor_similarity(template1->template_tensor, 
                                               template2->template_tensor);
    }
    
    // Combined similarity
    float overall_sim = (structural_sim * 0.3f + type_sim * 0.2f + 
                        algo_sim * 0.2f + tensor_sim * 0.3f);
    
    return overall_sim;
}

// Get pattern statistics
void pattern_get_statistics(
    pattern_engine_t* engine,
    uint64_t* total_searches,
    uint64_t* successful_matches,
    float* average_match_time,
    float* success_rate) {
    
    if (!engine) return;
    
    if (total_searches) {
        *total_searches = engine->total_pattern_searches;
    }
    
    if (successful_matches) {
        *successful_matches = engine->successful_matches;
    }
    
    if (average_match_time) {
        *average_match_time = engine->average_match_time;
    }
    
    if (success_rate) {
        *success_rate = (engine->total_pattern_searches > 0) ?
            (float)engine->successful_matches / (float)engine->total_pattern_searches : 0.0f;
    }
}

// Print pattern template
void pattern_print_template(pattern_template_t* template) {
    if (!template) return;
    
    printf("\n=== Pattern Template: %s ===\n", template->name);
    printf("ID: %lu\n", template->template_id);
    printf("Type: %d, Algorithm: %d\n", template->type, template->algorithm);
    printf("Nodes: %zu, Edges: %zu\n", template->node_count, template->edge_count);
    printf("Variables: %zu\n", template->variable_count);
    printf("Similarity threshold: %.2f\n", template->similarity_threshold);
    printf("Confidence threshold: %.2f\n", template->confidence_threshold);
    printf("Match count: %lu\n", template->match_count);
    printf("Total searches: %lu\n", template->total_searches);
    printf("Average match confidence: %.2f\n", template->average_match_confidence);
    
    // Print variables
    if (template->variable_count > 0) {
        printf("Variables: ");
        for (size_t i = 0; i < template->variable_count; i++) {
            printf("%s ", template->variables[i]);
        }
        printf("\n");
    }
    
    printf("===============================\n\n");
}

// Print pattern matches
void pattern_print_matches(
    pattern_match_t* matches,
    size_t match_count) {
    
    if (!matches || match_count == 0) {
        printf("No pattern matches to display.\n");
        return;
    }
    
    printf("\n=== Pattern Matches (%zu) ===\n", match_count);
    
    for (size_t i = 0; i < match_count; i++) {
        pattern_match_t* match = &matches[i];
        
        printf("Match %zu:\n", i + 1);
        printf("  ID: %lu, Template ID: %lu\n", match->match_id, match->template_id);
        printf("  Confidence: %.3f\n", match->match_confidence);
        printf("  Structural similarity: %.3f\n", match->structural_similarity);
        printf("  Semantic similarity: %.3f\n", match->semantic_similarity);
        printf("  Matched atoms: %zu\n", match->matched_count);
        printf("  Variable bindings: %zu\n", match->binding_count);
        
        if (match->binding_count > 0) {
            printf("  Bindings:\n");
            for (size_t j = 0; j < match->binding_count; j++) {
                printf("    %s -> %lu (conf: %.2f)\n",
                       match->variable_bindings[j].variable_name,
                       match->variable_bindings[j].bound_atom_id,
                       match->variable_bindings[j].binding_confidence);
            }
        }
        printf("\n");
    }
    
    printf("===============================\n\n");
}

// Print engine statistics
void pattern_print_engine_stats(pattern_engine_t* engine) {
    if (!engine) return;
    
    printf("\n=== Pattern Engine Statistics ===\n");
    printf("Initialized: %s\n", engine->initialized ? "Yes" : "No");
    printf("Templates: %zu/%zu\n", engine->template_count, engine->template_capacity);
    printf("Matches: %zu/%zu\n", engine->match_count, engine->match_capacity);
    printf("Total searches: %lu\n", engine->total_pattern_searches);
    printf("Successful matches: %lu\n", engine->successful_matches);
    
    if (engine->total_pattern_searches > 0) {
        float success_rate = (float)engine->successful_matches / (float)engine->total_pattern_searches;
        printf("Success rate: %.2f%%\n", success_rate * 100.0f);
    }
    
    printf("Average match time: %.2f ms\n", engine->average_match_time);
    printf("Global similarity threshold: %.2f\n", engine->global_similarity_threshold);
    printf("Global confidence threshold: %.2f\n", engine->global_confidence_threshold);
    printf("Cache size: %zu/%zu\n", engine->cache_size, engine->cache_capacity);
    printf("==================================\n\n");
}

// Free pattern template
void pattern_free_template(pattern_template_t* template) {
    if (!template) return;
    
    if (template->nodes) {
        free(template->nodes);
    }
    
    if (template->edges) {
        free(template->edges);
    }
    
    // Note: template_tensor is managed by ggml context
}

// Free pattern matches
void pattern_free_matches(
    pattern_match_t* matches,
    size_t match_count) {
    
    if (!matches) return;
    
    for (size_t i = 0; i < match_count; i++) {
        if (matches[i].matched_atom_ids) {
            free(matches[i].matched_atom_ids);
        }
    }
    
    free(matches);
}