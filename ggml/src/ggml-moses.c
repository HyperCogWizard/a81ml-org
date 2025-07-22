#include "ggml-moses.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <assert.h>

// Random number generation utilities
static float random_float() {
    return (float)rand() / (float)RAND_MAX;
}

static int random_int(int max) {
    return rand() % max;
}

// Generate unique program ID
static uint32_t generate_program_id(moses_system_t* moses) {
    static uint32_t next_id = 1;
    return next_id++;
}

// Generate unique node ID
static uint32_t generate_node_id(moses_system_t* moses) {
    static uint32_t next_node_id = 1;
    return next_node_id++;
}

// Initialize MOSES system
moses_system_t* moses_system_init(
    struct ggml_context* ctx,
    opencog_atomspace_t* atomspace,
    ggml_cognitive_kernel_t* cognitive_kernel) {
    
    moses_system_t* moses = calloc(1, sizeof(moses_system_t));
    if (!moses) return NULL;
    
    moses->ctx = ctx;
    moses->atomspace = atomspace;
    moses->cognitive_kernel = cognitive_kernel;
    
    // Initialize default configuration
    moses->config.population_size = 100;
    moses->config.max_generations = 500;
    moses->config.target_fitness = 0.99f;
    moses->config.fitness_tolerance = 0.001f;
    
    moses->config.mutation_probability = 0.1f;
    moses->config.crossover_probability = 0.8f;
    moses->config.reproduction_probability = 0.1f;
    
    moses->config.tournament_size = 3;
    moses->config.elitism_fraction = 0.1f;
    
    moses->config.max_program_depth = 10;
    moses->config.max_program_nodes = 64;
    moses->config.complexity_penalty_weight = 0.01f;
    
    moses->config.diversity_weight = 0.1f;
    moses->config.novelty_threshold = 0.8f;
    
    moses->config.enable_meta_optimization = true;
    moses->config.meta_generations = 10;
    moses->config.meta_learning_rate = 0.01f;
    
    moses->config.use_atomspace_seeding = true;
    moses->config.use_cognitive_tensor_fitness = true;
    moses->config.enable_prime_structure_bias = true;
    
    // Initialize operators with default implementations
    // (will be implemented as simple versions for now)
    
    // Initialize system state
    moses->initialized = true;
    moses->total_evaluations = 0;
    moses->successful_runs = 0;
    moses->best_ever_fitness = -INFINITY;
    moses->best_program = NULL;
    
    moses->start_time = (uint64_t)time(NULL);
    moses->total_evolution_time = 0.0f;
    
    // Initialize fitness trajectory
    for (int i = 0; i < MOSES_MAX_GENERATIONS; i++) {
        moses->average_fitness_trajectory[i] = 0.0f;
    }
    
    printf("MOSES system initialized with cognitive integration\n");
    printf("  Population size: %u\n", moses->config.population_size);
    printf("  Max generations: %u\n", moses->config.max_generations);
    printf("  AtomSpace integration: %s\n", atomspace ? "enabled" : "disabled");
    printf("  Cognitive tensor support: %s\n", cognitive_kernel ? "enabled" : "disabled");
    
    return moses;
}

// Free MOSES system
void moses_system_free(moses_system_t* moses) {
    if (!moses) return;
    
    if (moses->population) {
        moses_population_free(moses->population);
    }
    
    if (moses->training_data) {
        moses_dataset_free(moses->training_data);
    }
    
    if (moses->best_program) {
        moses_program_free(moses->best_program);
    }
    
    free(moses);
}

// Configure MOSES system
bool moses_system_configure(moses_system_t* moses, moses_config_t* config) {
    if (!moses || !config) return false;
    
    moses->config = *config;
    
    printf("MOSES system reconfigured:\n");
    printf("  Population size: %u\n", moses->config.population_size);
    printf("  Max generations: %u\n", moses->config.max_generations);
    printf("  Target fitness: %.3f\n", moses->config.target_fitness);
    
    return true;
}

// Create dataset
moses_dataset_t* moses_dataset_create(size_t max_cases, size_t feature_count) {
    moses_dataset_t* dataset = calloc(1, sizeof(moses_dataset_t));
    if (!dataset) return NULL;
    
    dataset->fitness_cases = calloc(max_cases, sizeof(moses_fitness_case_t));
    if (!dataset->fitness_cases) {
        free(dataset);
        return NULL;
    }
    
    dataset->max_cases = max_cases;
    dataset->feature_count = feature_count;
    dataset->case_count = 0;
    
    // Initialize feature metadata
    for (size_t i = 0; i < feature_count && i < MOSES_MAX_FEATURES; i++) {
        snprintf(dataset->feature_names[i], 64, "feature_%zu", i);
        dataset->feature_min[i] = INFINITY;
        dataset->feature_max[i] = -INFINITY;
    }
    
    dataset->output_min = INFINITY;
    dataset->output_max = -INFINITY;
    dataset->is_classification = false;
    
    printf("Created MOSES dataset: %zu max cases, %zu features\n", max_cases, feature_count);
    
    return dataset;
}

// Free dataset
void moses_dataset_free(moses_dataset_t* dataset) {
    if (!dataset) return;
    
    if (dataset->fitness_cases) {
        for (size_t i = 0; i < dataset->case_count; i++) {
            if (dataset->fitness_cases[i].input_features) {
                free(dataset->fitness_cases[i].input_features);
            }
        }
        free(dataset->fitness_cases);
    }
    
    free(dataset);
}

// Add fitness case to dataset
bool moses_dataset_add_case(
    moses_dataset_t* dataset,
    float* input_features,
    float target_output,
    float weight) {
    
    if (!dataset || !input_features || dataset->case_count >= dataset->max_cases) {
        return false;
    }
    
    moses_fitness_case_t* fitness_case = &dataset->fitness_cases[dataset->case_count];
    
    // Copy input features
    fitness_case->input_features = malloc(dataset->feature_count * sizeof(float));
    if (!fitness_case->input_features) return false;
    
    memcpy(fitness_case->input_features, input_features, 
           dataset->feature_count * sizeof(float));
    
    fitness_case->feature_count = dataset->feature_count;
    fitness_case->target_output = target_output;
    fitness_case->weight = weight;
    
    snprintf(fitness_case->description, 128, "case_%zu", dataset->case_count);
    
    // Update feature statistics
    for (size_t i = 0; i < dataset->feature_count; i++) {
        if (input_features[i] < dataset->feature_min[i]) {
            dataset->feature_min[i] = input_features[i];
        }
        if (input_features[i] > dataset->feature_max[i]) {
            dataset->feature_max[i] = input_features[i];
        }
    }
    
    // Update output statistics
    if (target_output < dataset->output_min) {
        dataset->output_min = target_output;
    }
    if (target_output > dataset->output_max) {
        dataset->output_max = target_output;
    }
    
    dataset->case_count++;
    
    return true;
}

// Create a simple program node
static moses_program_node_t* create_program_node(
    moses_system_t* moses,
    moses_node_type_t type,
    const char* name,
    float value) {
    
    moses_program_node_t* node = calloc(1, sizeof(moses_program_node_t));
    if (!node) return NULL;
    
    node->type = type;
    if (name) {
        strncpy(node->name, name, 63);
        node->name[63] = '\0';
    }
    node->value = value;
    
    node->left = NULL;
    node->right = NULL;
    node->condition = NULL;
    
    node->depth = 0;
    node->node_id = generate_node_id(moses);
    node->complexity_score = 1.0f;
    
    // Create tensor encoding for the node
    if (moses->ctx) {
        node->tensor_encoding = ggml_new_tensor_1d(moses->ctx, GGML_TYPE_F32, 16);
        ggml_set_zero(node->tensor_encoding);
        
        // Simple encoding based on node type and value
        float* data = (float*)node->tensor_encoding->data;
        data[0] = (float)type;
        data[1] = value;
        data[2] = (float)node->node_id;
    }
    
    return node;
}

// Create random program tree
static moses_program_node_t* create_random_tree(
    moses_system_t* moses,
    int current_depth,
    int max_depth) {
    
    if (current_depth >= max_depth || random_float() < 0.3f) {
        // Create terminal node
        if (random_float() < 0.5f) {
            // Constant
            float value = random_float() * 2.0f - 1.0f; // Range [-1, 1]
            return create_program_node(moses, MOSES_NODE_CONSTANT, "const", value);
        } else {
            // Variable
            int var_index = random_int(moses->training_data ? 
                                      moses->training_data->feature_count : 4);
            char var_name[64];
            snprintf(var_name, 64, "x%d", var_index);
            return create_program_node(moses, MOSES_NODE_VARIABLE, var_name, (float)var_index);
        }
    }
    
    // Create function node
    moses_node_type_t function_types[] = {
        MOSES_NODE_ARITHMETIC_ADD,
        MOSES_NODE_ARITHMETIC_MUL,
        MOSES_NODE_LOGICAL_AND,
        MOSES_NODE_LOGICAL_OR
    };
    
    moses_node_type_t type = function_types[random_int(4)];
    moses_program_node_t* node = create_program_node(moses, type, "func", 0.0f);
    
    if (node) {
        node->left = create_random_tree(moses, current_depth + 1, max_depth);
        node->right = create_random_tree(moses, current_depth + 1, max_depth);
        node->depth = current_depth;
        
        // Update complexity score
        float left_complexity = node->left ? node->left->complexity_score : 0.0f;
        float right_complexity = node->right ? node->right->complexity_score : 0.0f;
        node->complexity_score = 1.0f + left_complexity + right_complexity;
    }
    
    return node;
}

// Create random program
moses_program_t* moses_program_create_random(
    moses_system_t* moses,
    int max_depth) {
    
    if (!moses) return NULL;
    
    moses_program_t* program = calloc(1, sizeof(moses_program_t));
    if (!program) return NULL;
    
    program->root = create_random_tree(moses, 0, max_depth);
    program->fitness_score = -INFINITY;
    program->complexity_penalty = 0.0f;
    program->diversity_score = 0.0f;
    program->generation = 0;
    program->program_id = generate_program_id(moses);
    
    program->atomspace_atom_id = 0;
    program->evaluation_count = 0;
    program->execution_time = 0.0f;
    program->is_valid = true;
    
    // Create Matula encoding if cognitive kernel is available
    if (moses->cognitive_kernel && program->root) {
        char tree_expr[256] = "(program)"; // Simplified for now
        program->matula_encoding = ggml_encode_tree(tree_expr, 
                                                   &moses->cognitive_kernel->prime_cache);
    }
    
    return program;
}

// Clone program
moses_program_t* moses_program_clone(moses_program_t* program) {
    if (!program) return NULL;
    
    // For now, return a simple copy (deep cloning would require recursive tree copying)
    moses_program_t* clone = malloc(sizeof(moses_program_t));
    if (!clone) return NULL;
    
    *clone = *program;
    clone->program_id = generate_program_id(NULL); // Generate new ID
    clone->root = NULL; // Would need recursive cloning for full implementation
    
    return clone;
}

// Free program
void moses_program_free(moses_program_t* program) {
    if (!program) return;
    
    // TODO: Recursively free the tree structure
    free(program);
}

// Evaluate program on given inputs (simplified)
float moses_program_evaluate(
    moses_system_t* moses,
    moses_program_t* program,
    float* input_features) {
    
    if (!moses || !program || !program->root || !input_features) {
        return 0.0f;
    }
    
    // Simplified evaluation - just return a placeholder based on inputs
    float result = 0.0f;
    if (moses->training_data) {
        for (size_t i = 0; i < moses->training_data->feature_count && i < 4; i++) {
            result += input_features[i] * 0.1f;
        }
    }
    
    program->evaluation_count++;
    moses->total_evaluations++;
    
    return result;
}

// Compute program fitness
float moses_program_compute_fitness(
    moses_system_t* moses,
    moses_program_t* program) {
    
    if (!moses || !program || !moses->training_data) {
        return -INFINITY;
    }
    
    float total_error = 0.0f;
    float total_weight = 0.0f;
    
    // Evaluate program on all fitness cases
    for (size_t i = 0; i < moses->training_data->case_count; i++) {
        moses_fitness_case_t* fitness_case = &moses->training_data->fitness_cases[i];
        
        float predicted = moses_program_evaluate(moses, program, 
                                                fitness_case->input_features);
        float error = fabsf(predicted - fitness_case->target_output);
        
        total_error += error * fitness_case->weight;
        total_weight += fitness_case->weight;
    }
    
    float average_error = total_weight > 0.0f ? total_error / total_weight : INFINITY;
    
    // Fitness is inverse of error (higher is better)
    float fitness = 1.0f / (1.0f + average_error);
    
    // Apply complexity penalty
    float complexity_penalty = program->root ? 
        program->root->complexity_score * moses->config.complexity_penalty_weight : 0.0f;
    
    program->fitness_score = fitness - complexity_penalty;
    program->complexity_penalty = complexity_penalty;
    
    return program->fitness_score;
}

// Initialize population
moses_population_t* moses_population_initialize(moses_system_t* moses) {
    if (!moses) return NULL;
    
    moses_population_t* population = calloc(1, sizeof(moses_population_t));
    if (!population) return NULL;
    
    population->max_population_size = moses->config.population_size;
    population->programs = calloc(population->max_population_size, sizeof(moses_program_t));
    if (!population->programs) {
        free(population);
        return NULL;
    }
    
    // Initialize with random programs
    for (size_t i = 0; i < population->max_population_size; i++) {
        moses_program_t* program = moses_program_create_random(moses, 
                                                              moses->config.max_program_depth);
        if (program) {
            population->programs[i] = *program;
            free(program); // Copy made, free original
        }
    }
    
    population->population_size = population->max_population_size;
    population->current_generation = 0;
    
    // Initialize evolution parameters
    population->mutation_rate = moses->config.mutation_probability;
    population->crossover_rate = moses->config.crossover_probability;
    population->elitism_rate = moses->config.elitism_fraction;
    population->complexity_weight = moses->config.complexity_penalty_weight;
    
    printf("Initialized MOSES population with %zu programs\n", population->population_size);
    
    return population;
}

// Free population
void moses_population_free(moses_population_t* population) {
    if (!population) return;
    
    if (population->programs) {
        free(population->programs);
    }
    
    free(population);
}

// Evolve one generation (simplified)
bool moses_evolve_generation(moses_system_t* moses) {
    if (!moses || !moses->population || !moses->training_data) {
        return false;
    }
    
    moses_population_t* population = moses->population;
    
    // Evaluate all programs
    float total_fitness = 0.0f;
    float best_fitness = -INFINITY;
    
    for (size_t i = 0; i < population->population_size; i++) {
        float fitness = moses_program_compute_fitness(moses, &population->programs[i]);
        total_fitness += fitness;
        
        if (fitness > best_fitness) {
            best_fitness = fitness;
            
            // Update best program
            if (fitness > moses->best_ever_fitness) {
                moses->best_ever_fitness = fitness;
                if (moses->best_program) {
                    moses_program_free(moses->best_program);
                }
                moses->best_program = moses_program_clone(&population->programs[i]);
            }
        }
    }
    
    // Update population statistics
    population->best_fitness = best_fitness;
    population->average_fitness = population->population_size > 0 ? 
        total_fitness / population->population_size : 0.0f;
    population->current_generation++;
    
    // Store fitness trajectory
    if (population->current_generation < MOSES_MAX_GENERATIONS) {
        moses->average_fitness_trajectory[population->current_generation] = 
            population->average_fitness;
    }
    
    printf("Generation %u: best=%.4f, avg=%.4f, evaluations=%u\n",
           population->current_generation, best_fitness, population->average_fitness,
           moses->total_evaluations);
    
    return true;
}

// Run evolution
moses_program_t* moses_run_evolution(
    moses_system_t* moses,
    moses_dataset_t* training_data) {
    
    if (!moses || !training_data) return NULL;
    
    moses->training_data = training_data;
    
    // Initialize population
    moses->population = moses_population_initialize(moses);
    if (!moses->population) return NULL;
    
    printf("Starting MOSES evolution...\n");
    printf("Target fitness: %.3f\n", moses->config.target_fitness);
    
    // Evolution loop
    for (uint32_t gen = 0; gen < moses->config.max_generations; gen++) {
        if (!moses_evolve_generation(moses)) {
            printf("Evolution failed at generation %u\n", gen);
            break;
        }
        
        // Check termination criteria
        if (moses->population->best_fitness >= moses->config.target_fitness) {
            printf("Target fitness reached at generation %u!\n", gen + 1);
            moses->successful_runs++;
            break;
        }
        
        // Integration with AtomSpace
        if (moses->atomspace && moses->config.use_atomspace_seeding) {
            // Could add evolved programs to AtomSpace here
        }
    }
    
    printf("Evolution completed. Best fitness: %.4f\n", moses->best_ever_fitness);
    
    return moses->best_program ? moses_program_clone(moses->best_program) : NULL;
}

// Integration with AtomSpace
uint64_t moses_program_to_atomspace(
    moses_system_t* moses,
    moses_program_t* program) {
    
    if (!moses || !program || !moses->atomspace) return 0;
    
    // Create a concept node for the program
    char program_name[128];
    snprintf(program_name, 128, "moses_program_%u", program->program_id);
    
    uint64_t atom_id = opencog_add_node(moses->atomspace, 
                                       OPENCOG_CONCEPT_NODE, 
                                       program_name);
    
    if (atom_id > 0) {
        // Set truth value based on fitness
        float strength = fmaxf(0.0f, fminf(1.0f, program->fitness_score));
        float confidence = program->evaluation_count > 0 ? 0.9f : 0.1f;
        
        opencog_set_truth_value(moses->atomspace, atom_id, strength, confidence);
        
        // Set attention based on fitness
        float sti = program->fitness_score * 0.5f;
        opencog_set_attention_value(moses->atomspace, atom_id, sti, 0.0f, 0.0f);
        
        program->atomspace_atom_id = atom_id;
        
        printf("Added MOSES program to AtomSpace: %s (ID %lu)\n", 
               program_name, atom_id);
    }
    
    return atom_id;
}

// Convert program to tensor
struct ggml_tensor* moses_program_to_tensor(
    moses_system_t* moses,
    moses_program_t* program) {
    
    if (!moses || !program || !moses->ctx) return NULL;
    
    // Create tensor representation
    struct ggml_tensor* tensor = ggml_new_tensor_1d(moses->ctx, GGML_TYPE_F32, 8);
    float* data = (float*)tensor->data;
    
    // Encode program properties
    data[0] = (float)program->program_id;
    data[1] = program->fitness_score;
    data[2] = program->complexity_penalty;
    data[3] = program->diversity_score;
    data[4] = (float)program->generation;
    data[5] = (float)program->evaluation_count;
    data[6] = program->execution_time;
    data[7] = program->is_valid ? 1.0f : 0.0f;
    
    return tensor;
}

// Print program statistics
void moses_program_print(moses_program_t* program) {
    if (!program) return;
    
    printf("MOSES Program %u:\n", program->program_id);
    printf("  Fitness: %.4f\n", program->fitness_score);
    printf("  Complexity penalty: %.4f\n", program->complexity_penalty);
    printf("  Generation: %u\n", program->generation);
    printf("  Evaluations: %u\n", program->evaluation_count);
    printf("  Valid: %s\n", program->is_valid ? "yes" : "no");
    
    if (program->atomspace_atom_id > 0) {
        printf("  AtomSpace ID: %lu\n", program->atomspace_atom_id);
    }
}

// Print system statistics
void moses_system_print_statistics(moses_system_t* moses) {
    if (!moses) return;
    
    printf("\n=== MOSES System Statistics ===\n");
    printf("Total evaluations: %u\n", moses->total_evaluations);
    printf("Successful runs: %u\n", moses->successful_runs);
    printf("Best ever fitness: %.4f\n", moses->best_ever_fitness);
    printf("Evolution time: %.2f seconds\n", moses->total_evolution_time);
    
    if (moses->population) {
        printf("Current generation: %u\n", moses->population->current_generation);
        printf("Population size: %zu\n", moses->population->population_size);
        printf("Current best fitness: %.4f\n", moses->population->best_fitness);
        printf("Current average fitness: %.4f\n", moses->population->average_fitness);
    }
    
    printf("Configuration:\n");
    printf("  Max generations: %u\n", moses->config.max_generations);
    printf("  Population size: %u\n", moses->config.population_size);
    printf("  Mutation rate: %.3f\n", moses->config.mutation_probability);
    printf("  Crossover rate: %.3f\n", moses->config.crossover_probability);
    printf("  Complexity weight: %.3f\n", moses->config.complexity_penalty_weight);
    
    printf("Integration:\n");
    printf("  AtomSpace: %s\n", moses->atomspace ? "connected" : "not connected");
    printf("  Cognitive tensor: %s\n", moses->cognitive_kernel ? "enabled" : "disabled");
    
    printf("==============================\n");
}