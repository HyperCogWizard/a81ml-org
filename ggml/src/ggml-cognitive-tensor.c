#include "ggml-cognitive-tensor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// Prime number generation using Sieve of Eratosthenes
static void generate_primes(uint32_t* primes, size_t* count, uint32_t limit) {
    bool* is_prime = calloc(limit + 1, sizeof(bool));
    for (uint32_t i = 2; i <= limit; i++) {
        is_prime[i] = true;
    }
    
    *count = 0;
    for (uint32_t i = 2; i <= limit; i++) {
        if (is_prime[i]) {
            primes[(*count)++] = i;
            if (*count >= GGML_COGNITIVE_MAX_PRIMES) break;
            
            for (uint32_t j = i * i; j <= limit; j += i) {
                is_prime[j] = false;
            }
        }
    }
    
    free(is_prime);
}

// Initialize prime lookup table
void ggml_init_prime_lookup(ggml_prime_lookup_t* lookup) {
    if (lookup->initialized) return;
    
    // Generate primes up to a reasonable limit
    generate_primes(lookup->primes, &lookup->prime_count, 10000);
    lookup->initialized = true;
    
    printf("Initialized prime lookup with %zu primes\n", lookup->prime_count);
}

// Get nth prime number
uint32_t ggml_nth_prime(uint32_t n) {
    static ggml_prime_lookup_t static_lookup = {0};
    
    if (!static_lookup.initialized) {
        ggml_init_prime_lookup(&static_lookup);
    }
    
    if (n == 0 || n > static_lookup.prime_count) {
        return 0; // Invalid index
    }
    
    return static_lookup.primes[n - 1];
}

// Prime offset function: p(n) = (n+1)-th prime
uint32_t ggml_prime_offset(uint32_t n) {
    return ggml_nth_prime(n + 1);
}

// Check if number is prime
bool ggml_is_prime(uint32_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    
    for (uint32_t i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

// Simple tree expression parser for Matula-Goebel encoding
static uint32_t parse_tree_expression(const char* expr, size_t* pos) {
    size_t len = strlen(expr);
    
    if (*pos >= len) return 1; // Empty tree maps to 1
    
    if (expr[*pos] == '(') {
        (*pos)++; // Skip '('
        
        uint32_t result = 1;
        
        // Parse subtrees
        while (*pos < len && expr[*pos] != ')') {
            uint32_t subtree = parse_tree_expression(expr, pos);
            if (subtree > 0) {
                uint32_t prime = ggml_prime_offset(subtree);
                result *= prime;
            }
        }
        
        if (*pos < len && expr[*pos] == ')') {
            (*pos)++; // Skip ')'
        }
        
        return result;
    } else {
        // Leaf node or atomic expression
        return 1;
    }
}

// Encode tree expression using Matula-Goebel prime offset
ggml_matula_encoding_t ggml_encode_tree(const char* tree_expression, ggml_prime_lookup_t* prime_cache) {
    ggml_matula_encoding_t encoding = {0};
    
    if (!tree_expression || strlen(tree_expression) == 0) {
        encoding.matula_value = 1;
        encoding.system_level = 1;
        encoding.phase = 1.0f + 0.0f * I;
        return encoding;
    }
    
    size_t pos = 0;
    encoding.matula_value = parse_tree_expression(tree_expression, &pos);
    
    // Determine system level based on Matula value
    encoding.system_level = ggml_matula_decode_system_level(encoding.matula_value);
    
    // Encode quantum phase
    encoding.phase = ggml_quantum_phase_encode(encoding.matula_value, 0.0f);
    
    // Simple breadth/depth assignment (could be more sophisticated)
    encoding.breadth_index = encoding.matula_value % GGML_COGNITIVE_MAX_BREADTH;
    encoding.depth_index = (encoding.matula_value / GGML_COGNITIVE_MAX_BREADTH) % GGML_COGNITIVE_MAX_DEPTH;
    
    return encoding;
}

// Decode system level from Matula value
uint32_t ggml_matula_decode_system_level(uint32_t matula_value) {
    if (matula_value == 1) return 1;
    if (matula_value <= 4) return 2;
    if (matula_value <= 9) return 3;
    if (matula_value <= 16) return 4;
    
    // For larger values, use log-based approximation
    return (uint32_t)(log2(matula_value)) + 1;
}

// Factorize Matula value into prime factors
void ggml_matula_factorize(uint32_t matula_value, uint32_t* factors, size_t* factor_count) {
    *factor_count = 0;
    uint32_t n = matula_value;
    
    // Handle factor 2
    while (n % 2 == 0) {
        factors[(*factor_count)++] = 2;
        n /= 2;
    }
    
    // Handle odd factors
    for (uint32_t i = 3; i * i <= n; i += 2) {
        while (n % i == 0) {
            factors[(*factor_count)++] = i;
            n /= i;
        }
    }
    
    if (n > 2) {
        factors[(*factor_count)++] = n;
    }
}

// Quantum phase encoding
ggml_complex_t ggml_quantum_phase_encode(uint32_t matula_value, float phase_parameter) {
    float phase = fmodf(phase_parameter + (float)matula_value * 0.1f, 2.0f * M_PI);
    return cosf(phase) + sinf(phase) * I;
}

// Initialize cognitive kernel tensor
ggml_cognitive_kernel_t* ggml_cognitive_kernel_init(
    struct ggml_context* ctx,
    uint32_t max_systems,
    uint32_t max_breadth,
    uint32_t max_depth) {
    
    ggml_cognitive_kernel_t* kernel = calloc(1, sizeof(ggml_cognitive_kernel_t));
    if (!kernel) return NULL;
    
    kernel->max_systems = max_systems;
    kernel->max_breadth = max_breadth;
    kernel->max_depth = max_depth;
    
    // Initialize prime lookup
    ggml_init_prime_lookup(&kernel->prime_cache);
    
    // Create primary 4-mode tensor [System × Breadth × Depth × Phase]
    kernel->cognitive_kernel = ggml_new_tensor_4d(ctx, GGML_TYPE_F32, 
                                                  max_systems, max_breadth, max_depth, 2);
    
    // Create auxiliary structures
    kernel->prime_lookup = ggml_new_tensor_1d(ctx, GGML_TYPE_I32, GGML_COGNITIVE_MAX_PRIMES);
    kernel->matula_embedding = ggml_new_tensor_2d(ctx, GGML_TYPE_F32, 4, 1024); // [coordinates, max_matula]
    kernel->factorization_graph = ggml_new_tensor_2d(ctx, GGML_TYPE_F32, 1024, 1024); // Sparse representation
    
    // Create quantum phase encoding matrices
    kernel->phase_interference = ggml_new_tensor_2d(ctx, GGML_TYPE_F32, max_breadth, max_depth);
    kernel->superposition_states = ggml_new_tensor_3d(ctx, GGML_TYPE_F32, max_systems, max_breadth, max_depth);
    
    // Initialize tensors with default values
    ggml_set_zero(kernel->cognitive_kernel);
    ggml_set_zero(kernel->prime_lookup);
    ggml_set_zero(kernel->matula_embedding);
    ggml_set_zero(kernel->factorization_graph);
    ggml_set_zero(kernel->phase_interference);
    ggml_set_zero(kernel->superposition_states);
    
    // Fill prime lookup tensor
    int32_t* prime_data = (int32_t*)kernel->prime_lookup->data;
    for (size_t i = 0; i < kernel->prime_cache.prime_count && i < GGML_COGNITIVE_MAX_PRIMES; i++) {
        prime_data[i] = (int32_t)kernel->prime_cache.primes[i];
    }
    
    printf("Initialized cognitive kernel tensor: [%u × %u × %u × 2]\n", 
           max_systems, max_breadth, max_depth);
    
    return kernel;
}

// Cleanup cognitive kernel tensor
void ggml_cognitive_kernel_free(ggml_cognitive_kernel_t* kernel) {
    if (kernel) {
        // Note: tensors are owned by the ggml context and will be freed with it
        free(kernel);
    }
}

// Encode tree expression into cognitive kernel tensor
struct ggml_tensor* ggml_cognitive_kernel_encode(
    struct ggml_context* ctx,
    ggml_cognitive_kernel_t* kernel,
    const char* tree_expression) {
    
    ggml_matula_encoding_t encoding = ggml_encode_tree(tree_expression, &kernel->prime_cache);
    
    // Create result tensor
    struct ggml_tensor* result = ggml_new_tensor_1d(ctx, GGML_TYPE_F32, 4);
    float* data = (float*)result->data;
    
    // Store encoding components
    data[0] = (float)encoding.system_level;
    data[1] = (float)encoding.breadth_index;
    data[2] = (float)encoding.depth_index;
    data[3] = (float)encoding.matula_value;
    
    // Update cognitive kernel tensor at the appropriate coordinates
    if (encoding.system_level < kernel->max_systems &&
        encoding.breadth_index < kernel->max_breadth &&
        encoding.depth_index < kernel->max_depth) {
        
        // Access the 4D tensor data
        float* kernel_data = (float*)kernel->cognitive_kernel->data;
        size_t idx = (encoding.system_level * kernel->max_breadth * kernel->max_depth * 2) +
                     (encoding.breadth_index * kernel->max_depth * 2) +
                     (encoding.depth_index * 2);
        
        // Store real and imaginary parts of the phase
        kernel_data[idx] = crealf(encoding.phase);
        kernel_data[idx + 1] = cimagf(encoding.phase);
    }
    
    return result;
}

// Create superposition of tree states
struct ggml_tensor* ggml_cognitive_kernel_superposition(
    struct ggml_context* ctx,
    ggml_cognitive_kernel_t* kernel,
    ggml_tree_tensor_t* tree_states,
    size_t state_count) {
    
    struct ggml_tensor* result = ggml_new_tensor_2d(ctx, GGML_TYPE_F32, state_count, 4);
    float* data = (float*)result->data;
    
    float normalization = 0.0f;
    for (size_t i = 0; i < state_count; i++) {
        normalization += tree_states[i].probability_amplitude * tree_states[i].probability_amplitude;
    }
    normalization = sqrtf(normalization);
    
    for (size_t i = 0; i < state_count; i++) {
        size_t idx = i * 4;
        data[idx] = (float)tree_states[i].matula_value;
        data[idx + 1] = crealf(tree_states[i].phase);
        data[idx + 2] = cimagf(tree_states[i].phase);
        data[idx + 3] = tree_states[i].probability_amplitude / normalization;
    }
    
    return result;
}

// Prime-structured attention implementation
struct ggml_tensor* ggml_prime_attention(
    struct ggml_context* ctx,
    struct ggml_tensor* query,
    struct ggml_tensor* key,
    struct ggml_tensor* value,
    ggml_prime_attention_t* prime_config) {
    
    // For now, implement a simplified version
    // Just return the input query as a placeholder since matrix operations are complex
    (void)key;
    (void)value;
    (void)prime_config;
    
    return query;
}

// Generate phase interference pattern
struct ggml_tensor* ggml_phase_interference_pattern(
    struct ggml_context* ctx,
    ggml_cognitive_kernel_t* kernel,
    uint32_t breadth,
    uint32_t depth) {
    
    struct ggml_tensor* pattern = ggml_new_tensor_2d(ctx, GGML_TYPE_F32, breadth, depth);
    float* data = (float*)pattern->data;
    
    for (uint32_t b = 0; b < breadth; b++) {
        for (uint32_t d = 0; d < depth; d++) {
            // Generate interference pattern based on prime-structured coordinates
            float phase = (float)(b * ggml_nth_prime(d + 1)) * 0.1f;
            data[b * depth + d] = cosf(phase);
        }
    }
    
    return pattern;
}

// Tree tensor composition
ggml_tree_tensor_t ggml_tree_tensor_compose(ggml_tree_tensor_t t1, ggml_tree_tensor_t t2) {
    ggml_tree_tensor_t result = {0};
    
    if (t1.is_prime && t2.is_prime) {
        // Prime × Prime → Composite with phase entanglement
        result.matula_value = t1.matula_value * t2.matula_value;
        result.phase = t1.phase * t2.phase;
        result.is_prime = false;
        result.probability_amplitude = t1.probability_amplitude * t2.probability_amplitude;
    } else if (t1.has_single_skin) {
        // Single skin preservation → Prime encoding
        result.matula_value = ggml_prime_offset(t1.matula_value);
        result.phase = t1.phase;
        result.is_prime = true;
        result.probability_amplitude = t1.probability_amplitude;
    } else {
        // Default composition
        result.matula_value = t1.matula_value + t2.matula_value;
        result.phase = (t1.phase + t2.phase) / 2.0f;
        result.is_prime = false;
        result.probability_amplitude = (t1.probability_amplitude + t2.probability_amplitude) / 2.0f;
    }
    
    return result;
}

// Hypergraph composition of multiple tensors
struct ggml_tensor* ggml_hypergraph_compose(
    struct ggml_context* ctx,
    struct ggml_tensor** tensors,
    size_t tensor_count) {
    
    if (tensor_count == 0) return NULL;
    if (tensor_count == 1) return tensors[0];
    
    // Start with first tensor
    struct ggml_tensor* result = tensors[0];
    
    // Compose with remaining tensors
    for (size_t i = 1; i < tensor_count; i++) {
        result = ggml_add(ctx, result, tensors[i]);
    }
    
    return result;
}

// Cognitive tensor norm
float ggml_cognitive_tensor_norm(struct ggml_tensor* tensor, const char* norm_type) {
    if (!tensor || !tensor->data) return 0.0f;
    
    float* data = (float*)tensor->data;
    size_t n_elements = ggml_nelements(tensor);
    
    if (strcmp(norm_type, "l1") == 0) {
        float sum = 0.0f;
        for (size_t i = 0; i < n_elements; i++) {
            sum += fabsf(data[i]);
        }
        return sum;
    } else if (strcmp(norm_type, "l2") == 0) {
        float sum = 0.0f;
        for (size_t i = 0; i < n_elements; i++) {
            sum += data[i] * data[i];
        }
        return sqrtf(sum);
    } else if (strcmp(norm_type, "inf") == 0) {
        float max_val = 0.0f;
        for (size_t i = 0; i < n_elements; i++) {
            float abs_val = fabsf(data[i]);
            if (abs_val > max_val) max_val = abs_val;
        }
        return max_val;
    }
    
    return 0.0f;
}

// Cognitive tensor similarity
float ggml_cognitive_tensor_similarity(struct ggml_tensor* a, struct ggml_tensor* b) {
    if (!a || !b || !a->data || !b->data) return 0.0f;
    if (ggml_nelements(a) != ggml_nelements(b)) return 0.0f;
    
    float* data_a = (float*)a->data;
    float* data_b = (float*)b->data;
    size_t n_elements = ggml_nelements(a);
    
    float dot_product = 0.0f;
    float norm_a = 0.0f;
    float norm_b = 0.0f;
    
    for (size_t i = 0; i < n_elements; i++) {
        dot_product += data_a[i] * data_b[i];
        norm_a += data_a[i] * data_a[i];
        norm_b += data_b[i] * data_b[i];
    }
    
    if (norm_a == 0.0f || norm_b == 0.0f) return 0.0f;
    
    return dot_product / (sqrtf(norm_a) * sqrtf(norm_b));
}

// Print cognitive tensor statistics
void ggml_cognitive_tensor_print_stats(ggml_cognitive_kernel_t* kernel) {
    if (!kernel) return;
    
    printf("Cognitive Kernel Statistics:\n");
    printf("  Max Systems: %u\n", kernel->max_systems);
    printf("  Max Breadth: %u\n", kernel->max_breadth);
    printf("  Max Depth: %u\n", kernel->max_depth);
    printf("  Prime Cache: %zu primes\n", kernel->prime_cache.prime_count);
    
    if (kernel->cognitive_kernel) {
        printf("  Cognitive Kernel Tensor: ");
        for (int i = 0; i < GGML_MAX_DIMS; i++) {
            if (kernel->cognitive_kernel->ne[i] > 1) {
                printf("%ld ", kernel->cognitive_kernel->ne[i]);
            }
        }
        printf("\n");
    }
}

// Advanced Pattern Matching Implementation

// Default pattern matching configuration
ggml_pattern_match_config_t ggml_pattern_match_config_default(void) {
    ggml_pattern_match_config_t config = {
        .structure_weight = 0.4f,
        .semantic_weight = 0.4f,
        .phase_weight = 0.2f,
        .fuzzy_threshold = 0.7f,
        .enable_recursive = true,
        .enable_hierarchical = true,
        .max_recursion_depth = 5
    };
    return config;
}

// Combine multiple similarity scores using weighted average
float ggml_pattern_match_combine_scores(
    float structural, float semantic, float phase,
    ggml_pattern_match_config_t* config) {
    
    if (!config) {
        return (structural + semantic + phase) / 3.0f;
    }
    
    float total_weight = config->structure_weight + config->semantic_weight + config->phase_weight;
    if (total_weight == 0.0f) return 0.0f;
    
    return (structural * config->structure_weight + 
            semantic * config->semantic_weight + 
            phase * config->phase_weight) / total_weight;
}

// Extract Matula-Goebel structure from tensor
static uint32_t extract_matula_structure(struct ggml_tensor* tensor) {
    if (!tensor || !tensor->data) return 1;
    
    float* data = (float*)tensor->data;
    size_t n_elements = ggml_nelements(tensor);
    
    if (n_elements >= 4) {
        // Assume tensor contains [system_level, breadth, depth, matula_value]
        return (uint32_t)data[3];
    }
    
    // Fallback: compute simple hash-based Matula value
    uint32_t hash = 1;
    for (size_t i = 0; i < n_elements && i < 8; i++) {
        uint32_t val = (uint32_t)(fabsf(data[i]) * 1000) % 100;
        if (val > 0) {
            hash *= ggml_nth_prime(val % 20 + 1);
        }
    }
    return hash;
}

// Structure-aware pattern matching using Matula-Goebel encoding
ggml_pattern_match_result_t ggml_pattern_match_structural(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel,
    ggml_pattern_match_config_t* config) {
    
    ggml_pattern_match_result_t result = {0};
    
    if (!pattern || !target || !kernel) {
        result.match_type = PATTERN_MATCH_NONE;
        return result;
    }
    
    // Extract Matula values for structural comparison
    uint32_t pattern_matula = extract_matula_structure(pattern);
    uint32_t target_matula = extract_matula_structure(target);
    
    // Check for exact structural match
    if (pattern_matula == target_matula) {
        result.structural_similarity = 1.0f;
        result.is_exact_match = true;
        result.match_type = PATTERN_MATCH_EXACT;
        result.confidence_score = 1.0f;
        return result;
    }
    
    // Factorize both Matula values for structural comparison
    uint32_t pattern_factors[64], target_factors[64];
    size_t pattern_factor_count, target_factor_count;
    
    ggml_matula_factorize(pattern_matula, pattern_factors, &pattern_factor_count);
    ggml_matula_factorize(target_matula, target_factors, &target_factor_count);
    
    // Compute structural similarity based on common prime factors
    size_t common_factors = 0;
    size_t total_unique_factors = 0;
    
    for (size_t i = 0; i < pattern_factor_count; i++) {
        for (size_t j = 0; j < target_factor_count; j++) {
            if (pattern_factors[i] == target_factors[j]) {
                common_factors++;
                break;
            }
        }
    }
    
    total_unique_factors = pattern_factor_count + target_factor_count - common_factors;
    if (total_unique_factors == 0) {
        result.structural_similarity = 1.0f;
    } else {
        result.structural_similarity = (float)(2 * common_factors) / (float)total_unique_factors;
    }
    
    result.match_type = PATTERN_MATCH_STRUCTURAL;
    result.confidence_score = result.structural_similarity;
    
    return result;
}

// Phase-coherence pattern matching using quantum phases
ggml_pattern_match_result_t ggml_pattern_match_phase_coherence(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel) {
    
    ggml_pattern_match_result_t result = {0};
    
    if (!pattern || !target || !kernel) {
        result.match_type = PATTERN_MATCH_NONE;
        return result;
    }
    
    // Extract phase information from cognitive kernel tensor
    float* pattern_data = (float*)pattern->data;
    float* target_data = (float*)target->data;
    size_t n_elements = ggml_nelements(pattern);
    
    if (ggml_nelements(target) != n_elements) {
        result.match_type = PATTERN_MATCH_NONE;
        return result;
    }
    
    // Compute phase coherence using complex inner product
    float real_sum = 0.0f;
    float imag_sum = 0.0f;
    float pattern_norm = 0.0f;
    float target_norm = 0.0f;
    
    for (size_t i = 0; i < n_elements; i += 2) {
        if (i + 1 < n_elements) {
            // Treat pairs as complex numbers [real, imag]
            float p_real = pattern_data[i];
            float p_imag = pattern_data[i + 1];
            float t_real = target_data[i];
            float t_imag = target_data[i + 1];
            
            // Complex conjugate inner product: p* · t
            real_sum += p_real * t_real + p_imag * t_imag;
            imag_sum += p_real * t_imag - p_imag * t_real;
            
            pattern_norm += p_real * p_real + p_imag * p_imag;
            target_norm += t_real * t_real + t_imag * t_imag;
        }
    }
    
    if (pattern_norm == 0.0f || target_norm == 0.0f) {
        result.match_type = PATTERN_MATCH_NONE;
        return result;
    }
    
    // Phase coherence is the magnitude of normalized inner product
    float coherence_magnitude = sqrtf(real_sum * real_sum + imag_sum * imag_sum);
    result.phase_coherence = coherence_magnitude / sqrtf(pattern_norm * target_norm);
    
    result.match_type = PATTERN_MATCH_PHASE;
    result.confidence_score = result.phase_coherence;
    
    return result;
}

// Probabilistic pattern matching with confidence scores
ggml_pattern_match_result_t ggml_pattern_match_probabilistic(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel,
    float* confidence_map,
    size_t map_size) {
    
    ggml_pattern_match_result_t result = {0};
    
    if (!pattern || !target || !kernel) {
        result.match_type = PATTERN_MATCH_NONE;
        return result;
    }
    
    // First get basic semantic similarity
    result.semantic_similarity = ggml_cognitive_tensor_similarity(pattern, target);
    
    // Apply confidence weighting if provided
    if (confidence_map && map_size > 0) {
        float confidence_boost = 0.0f;
        size_t effective_size = map_size < ggml_nelements(pattern) ? map_size : ggml_nelements(pattern);
        
        for (size_t i = 0; i < effective_size; i++) {
            confidence_boost += confidence_map[i];
        }
        confidence_boost /= (float)effective_size;
        
        // Boost semantic similarity based on confidence
        result.semantic_similarity = result.semantic_similarity * (0.5f + 0.5f * confidence_boost);
    }
    
    // Compute overall confidence based on tensor statistics
    float* pattern_data = (float*)pattern->data;
    float* target_data = (float*)target->data;
    size_t n_elements = ggml_nelements(pattern);
    
    float variance_pattern = 0.0f;
    float variance_target = 0.0f;
    float mean_pattern = 0.0f;
    float mean_target = 0.0f;
    
    // Compute means
    for (size_t i = 0; i < n_elements; i++) {
        mean_pattern += pattern_data[i];
        mean_target += target_data[i];
    }
    mean_pattern /= (float)n_elements;
    mean_target /= (float)n_elements;
    
    // Compute variances
    for (size_t i = 0; i < n_elements; i++) {
        float diff_p = pattern_data[i] - mean_pattern;
        float diff_t = target_data[i] - mean_target;
        variance_pattern += diff_p * diff_p;
        variance_target += diff_t * diff_t;
    }
    variance_pattern /= (float)n_elements;
    variance_target /= (float)n_elements;
    
    // Confidence based on similar variance (similar information content)
    float variance_similarity = 1.0f / (1.0f + fabsf(variance_pattern - variance_target));
    result.confidence_score = (result.semantic_similarity + variance_similarity) / 2.0f;
    
    result.match_type = PATTERN_MATCH_SEMANTIC;
    
    return result;
}

// Fuzzy pattern matching with threshold controls
ggml_pattern_match_result_t ggml_pattern_match_fuzzy(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel,
    float threshold,
    float tolerance) {
    
    ggml_pattern_match_result_t result = {0};
    
    if (!pattern || !target || !kernel) {
        result.match_type = PATTERN_MATCH_NONE;
        return result;
    }
    
    // Get basic semantic similarity
    result.semantic_similarity = ggml_cognitive_tensor_similarity(pattern, target);
    
    // Apply fuzzy matching logic
    if (result.semantic_similarity >= threshold) {
        result.is_exact_match = (result.semantic_similarity >= (1.0f - tolerance));
        result.is_fuzzy_match = true;
        result.match_type = result.is_exact_match ? PATTERN_MATCH_EXACT : PATTERN_MATCH_FUZZY;
        result.confidence_score = result.semantic_similarity;
    } else {
        // Check if within tolerance of threshold
        if (result.semantic_similarity >= (threshold - tolerance)) {
            result.is_fuzzy_match = true;
            result.match_type = PATTERN_MATCH_FUZZY;
            // Reduced confidence for near-threshold matches
            result.confidence_score = result.semantic_similarity * 0.8f;
        } else {
            result.match_type = PATTERN_MATCH_NONE;
            result.confidence_score = 0.0f;
        }
    }
    
    return result;
}

// Recursive pattern matching for hierarchical structures
ggml_pattern_match_result_t ggml_pattern_match_recursive(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel,
    uint32_t current_depth,
    uint32_t max_depth) {
    
    ggml_pattern_match_result_t result = {0};
    
    if (!pattern || !target || !kernel || current_depth >= max_depth) {
        result.match_type = PATTERN_MATCH_NONE;
        return result;
    }
    
    // Base case: direct pattern matching
    ggml_pattern_match_config_t config = ggml_pattern_match_config_default();
    result = ggml_pattern_match_structural(pattern, target, kernel, &config);
    
    // If no good match at this level, try decomposing using Matula factorization
    if (result.structural_similarity < 0.5f && current_depth < max_depth - 1) {
        uint32_t pattern_matula = extract_matula_structure(pattern);
        uint32_t target_matula = extract_matula_structure(target);
        
        uint32_t pattern_factors[64], target_factors[64];
        size_t pattern_factor_count, target_factor_count;
        
        ggml_matula_factorize(pattern_matula, pattern_factors, &pattern_factor_count);
        ggml_matula_factorize(target_matula, target_factors, &target_factor_count);
        
        // Try matching individual factors recursively
        float recursive_score = 0.0f;
        size_t successful_matches = 0;
        
        for (size_t i = 0; i < pattern_factor_count && i < 4; i++) {
            for (size_t j = 0; j < target_factor_count && j < 4; j++) {
                if (pattern_factors[i] == target_factors[j]) {
                    recursive_score += 1.0f;
                    successful_matches++;
                }
            }
        }
        
        if (successful_matches > 0) {
            recursive_score /= (float)fmax(pattern_factor_count, target_factor_count);
            if (recursive_score > result.structural_similarity) {
                result.structural_similarity = recursive_score;
                result.match_type = PATTERN_MATCH_RECURSIVE;
                result.confidence_score = recursive_score * 0.9f; // Slightly reduced for recursive
            }
        }
    }
    
    return result;
}

// Multi-level pattern matching (syntactic, semantic, pragmatic)
ggml_pattern_match_result_t ggml_pattern_match_multilevel(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel,
    uint32_t match_levels) {
    
    ggml_pattern_match_result_t result = {0};
    
    if (!pattern || !target || !kernel) {
        result.match_type = PATTERN_MATCH_NONE;
        return result;
    }
    
    float level_scores[3] = {0.0f, 0.0f, 0.0f};
    int level_count = 0;
    
    // Level 1: Syntactic (structural) matching
    if (match_levels & 0x1) {
        ggml_pattern_match_config_t config = ggml_pattern_match_config_default();
        ggml_pattern_match_result_t structural_result = 
            ggml_pattern_match_structural(pattern, target, kernel, &config);
        level_scores[0] = structural_result.structural_similarity;
        level_count++;
    }
    
    // Level 2: Semantic matching
    if (match_levels & 0x2) {
        level_scores[1] = ggml_cognitive_tensor_similarity(pattern, target);
        level_count++;
    }
    
    // Level 3: Pragmatic (phase coherence) matching
    if (match_levels & 0x4) {
        ggml_pattern_match_result_t phase_result = 
            ggml_pattern_match_phase_coherence(pattern, target, kernel);
        level_scores[2] = phase_result.phase_coherence;
        level_count++;
    }
    
    if (level_count == 0) {
        result.match_type = PATTERN_MATCH_NONE;
        return result;
    }
    
    // Combine scores with appropriate weights
    result.structural_similarity = level_scores[0];
    result.semantic_similarity = level_scores[1];
    result.phase_coherence = level_scores[2];
    
    ggml_pattern_match_config_t config = ggml_pattern_match_config_default();
    result.confidence_score = ggml_pattern_match_combine_scores(
        level_scores[0], level_scores[1], level_scores[2], &config);
    
    // Determine primary match type based on strongest component
    if (level_scores[0] >= level_scores[1] && level_scores[0] >= level_scores[2]) {
        result.match_type = PATTERN_MATCH_STRUCTURAL;
    } else if (level_scores[1] >= level_scores[2]) {
        result.match_type = PATTERN_MATCH_SEMANTIC;
    } else {
        result.match_type = PATTERN_MATCH_PHASE;
    }
    
    return result;
}

// Unified advanced pattern matching interface
ggml_pattern_match_result_t ggml_pattern_match_advanced(
    struct ggml_tensor* pattern,
    struct ggml_tensor* target,
    ggml_cognitive_kernel_t* kernel,
    ggml_pattern_match_config_t* config) {
    
    ggml_pattern_match_result_t result = {0};
    
    if (!pattern || !target || !kernel) {
        result.match_type = PATTERN_MATCH_NONE;
        return result;
    }
    
    ggml_pattern_match_config_t default_config = ggml_pattern_match_config_default();
    if (!config) config = &default_config;
    
    // Perform multi-level matching (all levels)
    ggml_pattern_match_result_t multilevel_result = 
        ggml_pattern_match_multilevel(pattern, target, kernel, 0x7);
    
    result.structural_similarity = multilevel_result.structural_similarity;
    result.semantic_similarity = multilevel_result.semantic_similarity;
    result.phase_coherence = multilevel_result.phase_coherence;
    
    // Apply fuzzy matching if enabled
    if (config->fuzzy_threshold > 0.0f) {
        ggml_pattern_match_result_t fuzzy_result = 
            ggml_pattern_match_fuzzy(pattern, target, kernel, 
                                   config->fuzzy_threshold, 0.1f);
        result.is_fuzzy_match = fuzzy_result.is_fuzzy_match;
        result.is_exact_match = fuzzy_result.is_exact_match;
    }
    
    // Apply recursive matching if enabled
    if (config->enable_recursive && config->max_recursion_depth > 0) {
        ggml_pattern_match_result_t recursive_result = 
            ggml_pattern_match_recursive(pattern, target, kernel, 
                                       0, config->max_recursion_depth);
        // Use recursive result if it's better
        if (recursive_result.confidence_score > result.confidence_score) {
            result.structural_similarity = recursive_result.structural_similarity;
            result.match_type = recursive_result.match_type;
        }
    }
    
    // Compute final confidence score
    result.confidence_score = ggml_pattern_match_combine_scores(
        result.structural_similarity, 
        result.semantic_similarity, 
        result.phase_coherence, 
        config);
    
    // Determine final match type based on strongest component and configuration
    if (result.is_exact_match) {
        result.match_type = PATTERN_MATCH_EXACT;
    } else if (result.is_fuzzy_match) {
        result.match_type = PATTERN_MATCH_FUZZY;
    } else {
        result.match_type = multilevel_result.match_type;
    }
    
    return result;
}

// Print pattern match result
void ggml_pattern_match_result_print(ggml_pattern_match_result_t* result) {
    if (!result) return;
    
    printf("Pattern Match Result:\n");
    printf("  Match Type: %s\n", 
           result->match_type == PATTERN_MATCH_EXACT ? "Exact" :
           result->match_type == PATTERN_MATCH_STRUCTURAL ? "Structural" :
           result->match_type == PATTERN_MATCH_SEMANTIC ? "Semantic" :
           result->match_type == PATTERN_MATCH_PHASE ? "Phase" :
           result->match_type == PATTERN_MATCH_FUZZY ? "Fuzzy" :
           result->match_type == PATTERN_MATCH_RECURSIVE ? "Recursive" :
           "None");
    printf("  Structural Similarity: %.3f\n", result->structural_similarity);
    printf("  Semantic Similarity: %.3f\n", result->semantic_similarity);
    printf("  Phase Coherence: %.3f\n", result->phase_coherence);
    printf("  Confidence Score: %.3f\n", result->confidence_score);
    printf("  Exact Match: %s\n", result->is_exact_match ? "Yes" : "No");
    printf("  Fuzzy Match: %s\n", result->is_fuzzy_match ? "Yes" : "No");
}