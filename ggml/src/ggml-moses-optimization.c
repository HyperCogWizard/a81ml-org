#include "ggml-moses-optimization.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

// Random number generation utilities
static float random_float(float min, float max) {
    return min + (float)rand() / RAND_MAX * (max - min);
}

static int random_int(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Initialize MOSES engine
moses_engine_t* moses_engine_init(
    struct ggml_context* ctx,
    opencog_atomspace_t* atomspace,
    pln_reasoning_engine_t* pln_engine,
    ggml_cognitive_kernel_t* cognitive_kernel) {
    
    if (!ctx) return NULL;
    
    moses_engine_t* engine = malloc(sizeof(moses_engine_t));
    if (!engine) return NULL;
    
    memset(engine, 0, sizeof(moses_engine_t));
    
    engine->ctx = ctx;
    engine->atomspace = atomspace;
    engine->pln_engine = pln_engine;
    engine->cognitive_kernel = cognitive_kernel;
    
    // Initialize fitness context
    engine->fitness_context = malloc(sizeof(moses_fitness_context_t));
    if (!engine->fitness_context) {
        free(engine);
        return NULL;
    }
    
    memset(engine->fitness_context, 0, sizeof(moses_fitness_context_t));
    engine->fitness_context->atomspace = atomspace;
    engine->fitness_context->pln_engine = pln_engine;
    engine->fitness_context->cognitive_kernel = cognitive_kernel;
    engine->fitness_context->accuracy_weight = 0.7f;
    engine->fitness_context->complexity_weight = 0.2f;
    engine->fitness_context->cognitive_weight = 0.1f;
    
    // Set default evolution parameters
    engine->target_fitness = 0.95f;
    engine->use_cognitive_fitness = true;
    engine->use_attention_weighting = true;
    
    engine->initialized = true;
    
    printf("MOSES optimization engine initialized\n");
    
    return engine;
}

// Free MOSES engine
void moses_engine_free(moses_engine_t* engine) {
    if (!engine) return;
    
    if (engine->population) {
        for (size_t i = 0; i < engine->population->population_size; i++) {
            moses_free_program(&engine->population->individuals[i]);
        }
        free(engine->population->individuals);
        if (engine->population->best_individual) {
            moses_free_program(engine->population->best_individual);
            free(engine->population->best_individual);
        }
        free(engine->population);
    }
    
    if (engine->fitness_context) {
        if (engine->fitness_context->input_data) {
            for (size_t i = 0; i < engine->fitness_context->data_size; i++) {
                free(engine->fitness_context->input_data[i]);
            }
            free(engine->fitness_context->input_data);
        }
        if (engine->fitness_context->target_output) {
            free(engine->fitness_context->target_output);
        }
        free(engine->fitness_context);
    }
    
    free(engine);
}

// Free program node recursively
static void moses_free_node(moses_node_t* node) {
    if (!node) return;
    
    moses_free_node(node->left);
    moses_free_node(node->right);
    moses_free_node(node->condition);
    free(node);
}

// Free program
void moses_free_program(moses_program_t* program) {
    if (!program) return;
    
    moses_free_node(program->root);
    // Note: don't free tensor_encoding here as it's managed by ggml context
}

// Create random node
static moses_node_t* moses_create_random_node(
    moses_engine_t* engine,
    size_t current_depth,
    size_t max_depth,
    size_t num_variables) {
    
    moses_node_t* node = malloc(sizeof(moses_node_t));
    if (!node) return NULL;
    
    memset(node, 0, sizeof(moses_node_t));
    
    // Choose operator based on depth to control tree growth
    if (current_depth >= max_depth) {
        // Force terminal nodes at max depth
        if (random_int(0, 1) == 0) {
            node->op = MOSES_OP_LITERAL;
            node->value = random_float(-10.0f, 10.0f);
        } else {
            node->op = MOSES_OP_VARIABLE;
            node->variable_index = random_int(0, (int)num_variables - 1);
        }
    } else {
        // Choose from all operators
        int op_choice = random_int(1, 22);
        node->op = (moses_operator_t)op_choice;
        
        switch (node->op) {
            case MOSES_OP_LITERAL:
                node->value = random_float(-10.0f, 10.0f);
                break;
                
            case MOSES_OP_VARIABLE:
                node->variable_index = random_int(0, (int)num_variables - 1);
                break;
                
            case MOSES_OP_NOT:
            case MOSES_OP_SIN:
            case MOSES_OP_COS:
            case MOSES_OP_EXP:
            case MOSES_OP_LOG:
            case MOSES_OP_PLN_NOT:
                // Unary operators
                node->left = moses_create_random_node(engine, current_depth + 1, max_depth, num_variables);
                break;
                
            case MOSES_OP_IF_THEN_ELSE:
                // Ternary operator
                node->condition = moses_create_random_node(engine, current_depth + 1, max_depth, num_variables);
                node->left = moses_create_random_node(engine, current_depth + 1, max_depth, num_variables);
                node->right = moses_create_random_node(engine, current_depth + 1, max_depth, num_variables);
                break;
                
            default:
                // Binary operators
                node->left = moses_create_random_node(engine, current_depth + 1, max_depth, num_variables);
                node->right = moses_create_random_node(engine, current_depth + 1, max_depth, num_variables);
                break;
        }
    }
    
    // Initialize cognitive components
    node->truth_value.strength = 0.8f;
    node->truth_value.confidence = 0.9f;
    node->truth_value.count = 1.0f;
    node->attention_value = random_float(0.1f, 1.0f);
    
    return node;
}

// Count nodes in tree
static size_t moses_count_nodes(moses_node_t* node) {
    if (!node) return 0;
    
    return 1 + moses_count_nodes(node->left) + 
           moses_count_nodes(node->right) + 
           moses_count_nodes(node->condition);
}

// Calculate tree depth
static size_t moses_calculate_depth(moses_node_t* node) {
    if (!node) return 0;
    
    size_t left_depth = moses_calculate_depth(node->left);
    size_t right_depth = moses_calculate_depth(node->right);
    size_t condition_depth = moses_calculate_depth(node->condition);
    
    size_t max_depth = left_depth;
    if (right_depth > max_depth) max_depth = right_depth;
    if (condition_depth > max_depth) max_depth = condition_depth;
    
    return 1 + max_depth;
}

// Create random program
moses_program_t* moses_create_random_program(
    moses_engine_t* engine,
    size_t max_depth,
    size_t num_variables) {
    
    if (!engine) return NULL;
    
    moses_program_t* program = malloc(sizeof(moses_program_t));
    if (!program) return NULL;
    
    memset(program, 0, sizeof(moses_program_t));
    
    program->root = moses_create_random_node(engine, 0, max_depth, num_variables);
    if (!program->root) {
        free(program);
        return NULL;
    }
    
    program->node_count = moses_count_nodes(program->root);
    program->max_depth = moses_calculate_depth(program->root);
    program->fitness = 0.0f;
    program->complexity = (float)program->node_count / 100.0f; // Normalize complexity
    program->generation = 0;
    program->individual_id = engine->total_evaluations++;
    
    return program;
}

// Clone node recursively
static moses_node_t* moses_clone_node(moses_node_t* node) {
    if (!node) return NULL;
    
    moses_node_t* clone = malloc(sizeof(moses_node_t));
    if (!clone) return NULL;
    
    *clone = *node; // Copy all fields
    
    // Recursively clone children
    clone->left = moses_clone_node(node->left);
    clone->right = moses_clone_node(node->right);
    clone->condition = moses_clone_node(node->condition);
    
    return clone;
}

// Clone program
moses_program_t* moses_clone_program(moses_program_t* program) {
    if (!program) return NULL;
    
    moses_program_t* clone = malloc(sizeof(moses_program_t));
    if (!clone) return NULL;
    
    *clone = *program; // Copy all fields
    clone->root = moses_clone_node(program->root);
    
    return clone;
}

// Evaluate node
static float moses_evaluate_node(
    moses_node_t* node,
    float* inputs,
    size_t input_size,
    opencog_atomspace_t* atomspace) {
    
    if (!node) return 0.0f;
    
    switch (node->op) {
        case MOSES_OP_LITERAL:
            return node->value;
            
        case MOSES_OP_VARIABLE:
            if (node->variable_index >= 0 && node->variable_index < (int)input_size) {
                return inputs[node->variable_index];
            }
            return 0.0f;
            
        case MOSES_OP_AND: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            float right = moses_evaluate_node(node->right, inputs, input_size, atomspace);
            return (left > 0.5f && right > 0.5f) ? 1.0f : 0.0f;
        }
        
        case MOSES_OP_OR: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            float right = moses_evaluate_node(node->right, inputs, input_size, atomspace);
            return (left > 0.5f || right > 0.5f) ? 1.0f : 0.0f;
        }
        
        case MOSES_OP_NOT: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            return (left > 0.5f) ? 0.0f : 1.0f;
        }
        
        case MOSES_OP_PLUS: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            float right = moses_evaluate_node(node->right, inputs, input_size, atomspace);
            return left + right;
        }
        
        case MOSES_OP_MINUS: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            float right = moses_evaluate_node(node->right, inputs, input_size, atomspace);
            return left - right;
        }
        
        case MOSES_OP_MULTIPLY: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            float right = moses_evaluate_node(node->right, inputs, input_size, atomspace);
            return left * right;
        }
        
        case MOSES_OP_DIVIDE: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            float right = moses_evaluate_node(node->right, inputs, input_size, atomspace);
            return (fabsf(right) > 0.001f) ? left / right : 0.0f;
        }
        
        case MOSES_OP_IF_THEN_ELSE: {
            float condition = moses_evaluate_node(node->condition, inputs, input_size, atomspace);
            if (condition > 0.5f) {
                return moses_evaluate_node(node->left, inputs, input_size, atomspace);
            } else {
                return moses_evaluate_node(node->right, inputs, input_size, atomspace);
            }
        }
        
        case MOSES_OP_GREATER: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            float right = moses_evaluate_node(node->right, inputs, input_size, atomspace);
            return (left > right) ? 1.0f : 0.0f;
        }
        
        case MOSES_OP_LESS: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            float right = moses_evaluate_node(node->right, inputs, input_size, atomspace);
            return (left < right) ? 1.0f : 0.0f;
        }
        
        case MOSES_OP_EQUAL: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            float right = moses_evaluate_node(node->right, inputs, input_size, atomspace);
            return (fabsf(left - right) < 0.1f) ? 1.0f : 0.0f;
        }
        
        case MOSES_OP_SIN: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            return sinf(left);
        }
        
        case MOSES_OP_COS: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            return cosf(left);
        }
        
        case MOSES_OP_EXP: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            return expf(fmaxf(-10.0f, fminf(10.0f, left))); // Clamp to prevent overflow
        }
        
        case MOSES_OP_LOG: {
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            return (left > 0.001f) ? logf(left) : -10.0f;
        }
        
        case MOSES_OP_PLN_AND: {
            // Use PLN AND operation
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            float right = moses_evaluate_node(node->right, inputs, input_size, atomspace);
            
            opencog_truth_value_t tv1 = {left, 0.9f, 1.0f};
            opencog_truth_value_t tv2 = {right, 0.9f, 1.0f};
            opencog_truth_value_t result = opencog_pln_and(tv1, tv2);
            
            return result.strength;
        }
        
        case MOSES_OP_PLN_OR: {
            // Use PLN OR operation
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            float right = moses_evaluate_node(node->right, inputs, input_size, atomspace);
            
            opencog_truth_value_t tv1 = {left, 0.9f, 1.0f};
            opencog_truth_value_t tv2 = {right, 0.9f, 1.0f};
            opencog_truth_value_t result = opencog_pln_or(tv1, tv2);
            
            return result.strength;
        }
        
        case MOSES_OP_PLN_NOT: {
            // Use PLN NOT operation
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            
            opencog_truth_value_t tv = {left, 0.9f, 1.0f};
            opencog_truth_value_t result = opencog_pln_not(tv);
            
            return result.strength;
        }
        
        case MOSES_OP_ATTENTION: {
            // Return attention-weighted value
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            return left * node->attention_value;
        }
        
        case MOSES_OP_MEMORY: {
            // Memory operation (simplified - could integrate with AtomSpace)
            float left = moses_evaluate_node(node->left, inputs, input_size, atomspace);
            return left * 0.9f; // Memory decay factor
        }
        
        default:
            return 0.0f;
    }
}

// Evaluate program
float moses_evaluate_program(
    moses_engine_t* engine,
    moses_program_t* program,
    float* inputs,
    size_t input_size) {
    
    if (!engine || !program || !inputs) return 0.0f;
    
    return moses_evaluate_node(program->root, inputs, input_size, engine->atomspace);
}

// Calculate fitness based on accuracy
static float moses_calculate_accuracy_fitness(
    moses_engine_t* engine,
    moses_program_t* program) {
    
    if (!engine->fitness_context->input_data || !engine->fitness_context->target_output) {
        return 0.0f;
    }
    
    float total_error = 0.0f;
    size_t valid_evaluations = 0;
    
    for (size_t i = 0; i < engine->fitness_context->data_size; i++) {
        float output = moses_evaluate_program(engine, program, 
            engine->fitness_context->input_data[i], 
            engine->fitness_context->input_dimension);
        
        float target = engine->fitness_context->target_output[i];
        float error = fabsf(output - target);
        
        total_error += error;
        valid_evaluations++;
    }
    
    if (valid_evaluations == 0) return 0.0f;
    
    float mean_error = total_error / (float)valid_evaluations;
    float accuracy = 1.0f / (1.0f + mean_error); // Convert error to accuracy
    
    return accuracy;
}

// Evaluate cognitive fitness
float moses_evaluate_cognitive_fitness(
    moses_engine_t* engine,
    moses_program_t* program) {
    
    if (!engine || !program) return 0.0f;
    
    float cognitive_fitness = 0.0f;
    
    // Factor 1: Program complexity (prefer simpler programs)
    float complexity_fitness = 1.0f / (1.0f + program->complexity);
    cognitive_fitness += complexity_fitness * 0.3f;
    
    // Factor 2: Attention coherence
    float attention_coherence = 0.0f;
    // (This would be computed based on attention values in the program tree)
    cognitive_fitness += attention_coherence * 0.2f;
    
    // Factor 3: PLN consistency
    float pln_consistency = 0.8f; // Placeholder - would compute actual PLN consistency
    cognitive_fitness += pln_consistency * 0.3f;
    
    // Factor 4: Novelty/diversity
    float novelty = random_float(0.5f, 1.0f); // Placeholder
    cognitive_fitness += novelty * 0.2f;
    
    return cognitive_fitness;
}

// Initialize population
bool moses_initialize_population(
    moses_engine_t* engine,
    size_t population_size,
    size_t max_depth,
    size_t num_variables) {
    
    if (!engine || population_size == 0) return false;
    
    // Allocate population structure
    engine->population = malloc(sizeof(moses_population_t));
    if (!engine->population) return false;
    
    memset(engine->population, 0, sizeof(moses_population_t));
    
    engine->population->max_population_size = population_size;
    engine->population->individuals = malloc(sizeof(moses_program_t) * population_size);
    if (!engine->population->individuals) {
        free(engine->population);
        engine->population = NULL;
        return false;
    }
    
    // Set evolution parameters
    engine->population->mutation_rate = 0.1f;
    engine->population->crossover_rate = 0.8f;
    engine->population->elite_ratio = 0.1f;
    engine->population->current_generation = 0;
    engine->population->max_generations = 1000;
    
    // Generate initial population
    for (size_t i = 0; i < population_size; i++) {
        moses_program_t* program = moses_create_random_program(engine, max_depth, num_variables);
        if (program) {
            engine->population->individuals[i] = *program;
            // Don't free the program wrapper since we copied its contents including pointers
            // The individual cleanup will handle the root node properly
            engine->population->population_size++;
            
            // Free only the wrapper struct, not the contents
            free(program);
        }
    }
    
    printf("MOSES population initialized with %zu individuals\n", 
           engine->population->population_size);
    
    return true;
}

// Evaluate population fitness
void moses_evaluate_population(moses_engine_t* engine) {
    if (!engine || !engine->population) return;
    
    float total_fitness = 0.0f;
    float best_fitness = -1.0f;
    size_t best_index = 0;
    
    for (size_t i = 0; i < engine->population->population_size; i++) {
        moses_program_t* program = &engine->population->individuals[i];
        
        // Calculate accuracy fitness
        float accuracy_fitness = moses_calculate_accuracy_fitness(engine, program);
        
        // Calculate cognitive fitness if enabled
        float cognitive_fitness = 0.0f;
        if (engine->use_cognitive_fitness) {
            cognitive_fitness = moses_evaluate_cognitive_fitness(engine, program);
        }
        
        // Combine fitness components
        program->fitness = 
            accuracy_fitness * engine->fitness_context->accuracy_weight +
            cognitive_fitness * engine->fitness_context->cognitive_weight -
            program->complexity * engine->fitness_context->complexity_weight;
        
        program->cognitive_fitness = cognitive_fitness;
        
        total_fitness += program->fitness;
        
        if (program->fitness > best_fitness) {
            best_fitness = program->fitness;
            best_index = i;
        }
        
        engine->fitness_context->evaluations_count++;
    }
    
    // Update population statistics
    engine->population->average_fitness = total_fitness / (float)engine->population->population_size;
    engine->population->best_fitness = best_fitness;
    
    // Update best individual
    if (!engine->population->best_individual || 
        best_fitness > engine->population->best_individual->fitness) {
        
        if (engine->population->best_individual) {
            moses_free_program(engine->population->best_individual);
            free(engine->population->best_individual);
        }
        
        engine->population->best_individual = malloc(sizeof(moses_program_t));
        if (engine->population->best_individual) {
            *engine->population->best_individual = 
                *moses_clone_program(&engine->population->individuals[best_index]);
        }
    }
    
    if (engine->fitness_context->best_seen_fitness < best_fitness) {
        engine->fitness_context->best_seen_fitness = best_fitness;
    }
}

// Tournament selection
moses_program_t* moses_tournament_selection(
    moses_engine_t* engine,
    size_t tournament_size) {
    
    if (!engine || !engine->population || tournament_size == 0) return NULL;
    
    float best_fitness = -1.0f;
    moses_program_t* best_individual = NULL;
    
    for (size_t i = 0; i < tournament_size; i++) {
        size_t index = random_int(0, (int)engine->population->population_size - 1);
        moses_program_t* candidate = &engine->population->individuals[index];
        
        if (candidate->fitness > best_fitness) {
            best_fitness = candidate->fitness;
            best_individual = candidate;
        }
    }
    
    return best_individual;
}

// Get evolution statistics
void moses_get_statistics(
    moses_engine_t* engine,
    float* best_fitness,
    float* average_fitness,
    float* diversity,
    uint32_t* generation) {
    
    if (!engine || !engine->population) return;
    
    if (best_fitness) {
        *best_fitness = engine->population->best_fitness;
    }
    
    if (average_fitness) {
        *average_fitness = engine->population->average_fitness;
    }
    
    if (diversity) {
        *diversity = engine->population->population_diversity;
    }
    
    if (generation) {
        *generation = engine->population->current_generation;
    }
}

// Print program structure
void moses_print_program(moses_program_t* program, int indent) {
    if (!program) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    printf("Program (fitness: %.3f, complexity: %.3f, nodes: %zu, depth: %zu)\n",
           program->fitness, program->complexity, program->node_count, program->max_depth);
}

// Print population statistics
void moses_print_population_stats(moses_engine_t* engine) {
    if (!engine || !engine->population) return;
    
    printf("\n=== MOSES Population Statistics ===\n");
    printf("Generation: %u\n", engine->population->current_generation);
    printf("Population size: %zu\n", engine->population->population_size);
    printf("Best fitness: %.4f\n", engine->population->best_fitness);
    printf("Average fitness: %.4f\n", engine->population->average_fitness);
    printf("Diversity: %.4f\n", engine->population->population_diversity);
    printf("Total evaluations: %lu\n", engine->total_evaluations);
    printf("===================================\n\n");
}