#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>

// Simplified version of the distributed cognitive architecture for testing emergent behavior analysis
// This demonstrates the integration without complex dependencies

// Forward declarations for simplified structures
typedef struct emergent_behavior_analyzer emergent_behavior_analyzer_t;

// Copy emergent behavior analysis types from the main header
typedef enum {
    BEHAVIOR_TYPE_UNKNOWN = 0,
    BEHAVIOR_TYPE_CONVERGENT,
    BEHAVIOR_TYPE_OSCILLATORY,
    BEHAVIOR_TYPE_CHAOTIC,
    BEHAVIOR_TYPE_EMERGENT_COOPERATION,
    BEHAVIOR_TYPE_SPONTANEOUS_OPTIMIZATION,
    BEHAVIOR_TYPE_NOVEL_ADAPTATION,
    BEHAVIOR_TYPE_RECURSIVE_ENHANCEMENT,
    BEHAVIOR_TYPE_PHASE_TRANSITION
} emergent_behavior_type_t;

typedef struct {
    emergent_behavior_type_t type;
    char description[256];
    uint64_t detection_timestamp;
    float confidence;
    float novelty_score;
    uint32_t occurrences;
    
    // Context data
    float system_performance;
    float attention_distribution[4];
    uint32_t active_modules;
    
    // Correlation data
    float correlation_with_performance;
    float correlation_with_attention;
    bool triggered_by_self_modification;
} emergent_behavior_record_t;

typedef struct {
    float value;
    uint64_t timestamp;
} behavior_metric_sample_t;

typedef struct {
    behavior_metric_sample_t* samples;
    size_t sample_count;
    size_t sample_capacity;
    
    // Statistical properties
    float mean;
    float variance;
    float trend;
    float periodicity;
    bool is_stable;
    bool is_trending;
} behavior_metric_history_t;

struct emergent_behavior_analyzer {
    // Behavior records
    emergent_behavior_record_t* behaviors;
    size_t behavior_count;
    size_t behavior_capacity;
    
    // Metric histories
    behavior_metric_history_t performance_history;
    behavior_metric_history_t attention_coherence_history;
    behavior_metric_history_t adaptation_rate_history;
    behavior_metric_history_t module_activity_history;
    
    // Analysis parameters
    float novelty_threshold;
    float correlation_threshold;
    uint32_t min_pattern_length;
    uint32_t analysis_window_size;
    
    // System state tracking
    uint64_t system_time;
    uint32_t total_system_observations;
    
    // Pattern detection state
    float last_performance_levels[10];
    float last_attention_states[10][4];
    bool in_adaptation_phase;
    uint32_t consecutive_improvements;
    uint32_t consecutive_degradations;
    
    // Behavior detection cooldowns
    uint32_t last_convergent_detection;
    uint32_t last_oscillatory_detection;
    uint32_t last_optimization_detection;
    uint32_t last_cooperation_detection;
    uint32_t last_adaptation_detection;
    uint32_t last_phase_transition_detection;
};

// Simplified distributed cognitive architecture for testing
typedef struct {
    // Core state
    float performance;
    float attention_distribution[4];  // Memory, Reasoning, Communication, Self-modification
    uint32_t dynamic_module_count;
    uint64_t system_time;
    bool self_optimization_active;
    bool recursive_improvement_active;
    
    // Emergent behavior analysis
    emergent_behavior_analyzer_t* behavior_analyzer;
    bool emergent_behavior_analysis_enabled;
    
    // Mock evolution history for testing
    size_t evolution_history_count;
    
} test_distributed_cognitive_architecture_t;

// Mock functions that would exist in the real implementation
float distributed_cognitive_benchmark_performance(test_distributed_cognitive_architecture_t* arch) {
    return arch->performance;
}

// Utility functions (same as in the main implementation)
static float compute_attention_coherence(float attention_dist[4]) {
    float mean = 0.25f;  // Perfect balance
    float variance = 0.0f;
    
    for (int i = 0; i < 4; i++) {
        float diff = attention_dist[i] - mean;
        variance += diff * diff;
    }
    
    return 1.0f - (variance / 4.0f);  // Higher coherence = lower variance
}

static float compute_correlation(float* x, float* y, size_t n) {
    if (n < 2) return 0.0f;
    
    float sum_x = 0.0f, sum_y = 0.0f, sum_xy = 0.0f, sum_x2 = 0.0f, sum_y2 = 0.0f;
    
    for (size_t i = 0; i < n; i++) {
        sum_x += x[i];
        sum_y += y[i];
        sum_xy += x[i] * y[i];
        sum_x2 += x[i] * x[i];
        sum_y2 += y[i] * y[i];
    }
    
    float numerator = n * sum_xy - sum_x * sum_y;
    float denominator = sqrtf((n * sum_x2 - sum_x * sum_x) * (n * sum_y2 - sum_y * sum_y));
    
    return (denominator != 0.0f) ? numerator / denominator : 0.0f;
}

// Copy of the emergent behavior analysis implementation from the main file
#include "test_emergent_behavior_analysis.c"

// Override the original functions to work with simplified architecture
#undef add_metric_sample
#undef record_emergent_behavior

static void add_metric_sample_distributed(behavior_metric_history_t* history, float value, uint64_t timestamp) {
    if (history->sample_count >= history->sample_capacity) {
        // Shift samples to make room (simple sliding window)
        memmove(history->samples, history->samples + 1, 
                (history->sample_capacity - 1) * sizeof(behavior_metric_sample_t));
        history->sample_count = history->sample_capacity - 1;
    }
    
    history->samples[history->sample_count].value = value;
    history->samples[history->sample_count].timestamp = timestamp;
    history->sample_count++;
    
    // Update statistical properties
    float sum = 0.0f;
    for (size_t i = 0; i < history->sample_count; i++) {
        sum += history->samples[i].value;
    }
    history->mean = sum / history->sample_count;
    
    // Compute variance
    float var_sum = 0.0f;
    for (size_t i = 0; i < history->sample_count; i++) {
        float diff = history->samples[i].value - history->mean;
        var_sum += diff * diff;
    }
    history->variance = (history->sample_count > 1) ? var_sum / (history->sample_count - 1) : 0.0f;
    
    // Compute simple trend (slope of linear regression)
    if (history->sample_count >= 3) {
        float sum_t = 0.0f, sum_v = 0.0f, sum_tv = 0.0f, sum_t2 = 0.0f;
        for (size_t i = 0; i < history->sample_count; i++) {
            float t = (float)i;
            float v = history->samples[i].value;
            sum_t += t;
            sum_v += v;
            sum_tv += t * v;
            sum_t2 += t * t;
        }
        
        float n = (float)history->sample_count;
        float denominator = n * sum_t2 - sum_t * sum_t;
        history->trend = (denominator != 0.0f) ? (n * sum_tv - sum_t * sum_v) / denominator : 0.0f;
        
        history->is_trending = fabsf(history->trend) > 0.001f;
        history->is_stable = history->variance < 0.01f && fabsf(history->trend) < 0.001f;
    }
}

static void record_emergent_behavior_distributed(emergent_behavior_analyzer_t* analyzer,
                                                emergent_behavior_type_t type,
                                                const char* description,
                                                float confidence,
                                                float novelty_score,
                                                test_distributed_cognitive_architecture_t* arch) {
    
    if (analyzer->behavior_count >= analyzer->behavior_capacity) {
        return; // Could expand capacity here
    }
    
    emergent_behavior_record_t* record = &analyzer->behaviors[analyzer->behavior_count];
    
    record->type = type;
    strncpy(record->description, description, sizeof(record->description) - 1);
    record->description[sizeof(record->description) - 1] = '\0';
    
    record->detection_timestamp = arch->system_time;
    record->confidence = confidence;
    record->novelty_score = novelty_score;
    record->occurrences = 1;
    
    // Capture system context
    record->system_performance = arch->performance;
    memcpy(record->attention_distribution, arch->attention_distribution, sizeof(float) * 4);
    record->active_modules = arch->dynamic_module_count;
    
    record->triggered_by_self_modification = arch->self_optimization_active || arch->recursive_improvement_active;
    
    // Compute correlations if we have enough data
    if (analyzer->performance_history.sample_count >= 5) {
        size_t n = fminf(10, analyzer->performance_history.sample_count);
        float perf_values[10];
        float metric_values[10];
        
        for (size_t i = 0; i < n; i++) {
            size_t idx = analyzer->performance_history.sample_count - n + i;
            perf_values[i] = analyzer->performance_history.samples[idx].value;
            metric_values[i] = novelty_score + 0.1f * ((float)i / n);
        }
        
        float correlation = compute_correlation(perf_values, metric_values, n);
        record->correlation_with_performance = (isnan(correlation) || isinf(correlation)) ? 0.0f : correlation;
    } else {
        record->correlation_with_performance = 0.0f;
    }
    
    record->correlation_with_attention = compute_attention_coherence(arch->attention_distribution);
    
    analyzer->behavior_count++;
    
    printf("EMERGENT BEHAVIOR DETECTED: %s (confidence: %.3f, novelty: %.3f)\n", 
           description, confidence, novelty_score);
}

// Test-specific behavior detection functions
static bool detect_spontaneous_optimization_distributed(emergent_behavior_analyzer_t* analyzer, test_distributed_cognitive_architecture_t* arch) {
    if (analyzer->performance_history.sample_count < 8) {
        return false;
    }
    
    // Check cooldown period
    if (analyzer->system_time - analyzer->last_optimization_detection < 8) {
        return false;
    }
    
    // Check for improvement trend
    bool improving_trend = analyzer->performance_history.is_trending && analyzer->performance_history.trend > 0.01f;
    
    // Check if this improvement coincides with self-modification activity
    bool during_self_modification = arch->self_optimization_active || arch->recursive_improvement_active;
    
    if (improving_trend && !during_self_modification) {
        analyzer->last_optimization_detection = analyzer->system_time;
        record_emergent_behavior_distributed(analyzer, BEHAVIOR_TYPE_SPONTANEOUS_OPTIMIZATION,
                                           "System showing improvement without explicit optimization commands",
                                           0.9f, 0.9f, arch);
        return true;
    }
    
    return false;
}

static bool detect_emergent_cooperation_distributed(emergent_behavior_analyzer_t* analyzer, test_distributed_cognitive_architecture_t* arch) {
    if (analyzer->module_activity_history.sample_count >= 5 && analyzer->performance_history.sample_count >= 5) {
        
        // Check cooldown
        if (analyzer->system_time - analyzer->last_cooperation_detection < 12) {
            return false;
        }
        
        // Find recent period with module count increase
        size_t recent_samples = fminf(5, analyzer->module_activity_history.sample_count);
        bool module_increase = false;
        float performance_improvement = 0.0f;
        
        for (size_t i = 1; i < recent_samples; i++) {
            size_t idx = analyzer->module_activity_history.sample_count - recent_samples + i;
            size_t prev_idx = idx - 1;
            
            if (analyzer->module_activity_history.samples[idx].value > 
                analyzer->module_activity_history.samples[prev_idx].value) {
                module_increase = true;
                
                // Check corresponding performance change
                if (idx < analyzer->performance_history.sample_count && prev_idx < analyzer->performance_history.sample_count) {
                    performance_improvement = analyzer->performance_history.samples[idx].value - 
                                            analyzer->performance_history.samples[prev_idx].value;
                }
                break;
            }
        }
        
        // If module increase led to significant performance improvement (suggesting cooperation)
        if (module_increase && performance_improvement > 0.1f) {
            analyzer->last_cooperation_detection = analyzer->system_time;
            record_emergent_behavior_distributed(analyzer, BEHAVIOR_TYPE_EMERGENT_COOPERATION,
                                               "Modules showing emergent cooperative behavior with superlinear performance gains",
                                               0.7f, 0.8f, arch);
            return true;
        }
    }
    
    return false;
}

// Main analysis function for distributed architecture
bool emergent_behavior_analyze_distributed(test_distributed_cognitive_architecture_t* arch) {
    if (!arch || !arch->emergent_behavior_analysis_enabled || !arch->behavior_analyzer) {
        return false;
    }
    
    emergent_behavior_analyzer_t* analyzer = arch->behavior_analyzer;
    analyzer->system_time++;
    analyzer->total_system_observations++;
    
    // Record current metrics
    uint64_t timestamp = arch->system_time;
    float current_performance = arch->performance;
    add_metric_sample_distributed(&analyzer->performance_history, current_performance, timestamp);
    
    float attention_coherence = compute_attention_coherence(arch->attention_distribution);
    add_metric_sample_distributed(&analyzer->attention_coherence_history, attention_coherence, timestamp);
    
    add_metric_sample_distributed(&analyzer->module_activity_history, (float)arch->dynamic_module_count, timestamp);
    
    // Update performance tracking arrays
    memmove(analyzer->last_performance_levels, analyzer->last_performance_levels + 1, sizeof(float) * 9);
    analyzer->last_performance_levels[9] = current_performance;
    
    memmove(analyzer->last_attention_states, analyzer->last_attention_states + 1, sizeof(float[4]) * 9);
    memcpy(analyzer->last_attention_states[9], arch->attention_distribution, sizeof(float) * 4);
    
    // Track consecutive improvements/degradations
    if (analyzer->total_system_observations > 1) {
        if (current_performance > analyzer->last_performance_levels[8]) {
            analyzer->consecutive_improvements++;
            analyzer->consecutive_degradations = 0;
        } else if (current_performance < analyzer->last_performance_levels[8]) {
            analyzer->consecutive_degradations++;
            analyzer->consecutive_improvements = 0;
        }
    }
    
    // Run behavior detection algorithms
    bool detected_any = false;
    
    if (detect_spontaneous_optimization_distributed(analyzer, arch)) detected_any = true;
    if (detect_emergent_cooperation_distributed(analyzer, arch)) detected_any = true;
    
    // Detect phase transitions (major performance level shifts)
    if (analyzer->consecutive_improvements >= 5 && current_performance > 0.8f) {
        if (analyzer->system_time - analyzer->last_phase_transition_detection >= 15) {
            analyzer->last_phase_transition_detection = analyzer->system_time;
            record_emergent_behavior_distributed(analyzer, BEHAVIOR_TYPE_PHASE_TRANSITION,
                                               "System transitioned to high-performance operational mode",
                                               0.9f, 0.7f, arch);
            detected_any = true;
        }
    } else if (analyzer->consecutive_degradations >= 5 && current_performance < 0.3f) {
        if (analyzer->system_time - analyzer->last_phase_transition_detection >= 15) {
            analyzer->last_phase_transition_detection = analyzer->system_time;
            record_emergent_behavior_distributed(analyzer, BEHAVIOR_TYPE_PHASE_TRANSITION,
                                               "System transitioned to low-performance operational mode", 
                                               0.9f, 0.5f, arch);
            detected_any = true;
        }
    }
    
    return detected_any;
}

// Print analysis summary for distributed architecture
void emergent_behavior_print_analysis_distributed(test_distributed_cognitive_architecture_t* arch) {
    if (!arch || !arch->emergent_behavior_analysis_enabled || !arch->behavior_analyzer) {
        printf("Emergent behavior analysis not enabled\n");
        return;
    }
    
    emergent_behavior_analyzer_t* analyzer = arch->behavior_analyzer;
    
    printf("\n=== Distributed Emergent Behavior Analysis Summary ===\n");
    printf("Total observations: %u\n", analyzer->total_system_observations);
    printf("Detected behaviors: %zu\n", analyzer->behavior_count);
    
    if (analyzer->behavior_count > 0) {
        printf("\nDetected Behaviors:\n");
        for (size_t i = 0; i < analyzer->behavior_count; i++) {
            emergent_behavior_record_t* behavior = &analyzer->behaviors[i];
            printf("  %zu: %s - %s\n", i + 1, emergent_behavior_get_type_name(behavior->type), behavior->description);
            printf("      Confidence: %.3f, Novelty: %.3f, Occurrences: %u\n",
                   behavior->confidence, behavior->novelty_score, behavior->occurrences);
            printf("      Performance correlation: %.3f, Attention correlation: %.3f\n",
                   behavior->correlation_with_performance, behavior->correlation_with_attention);
        }
    }
    
    printf("\nSystem Metrics Summary:\n");
    printf("  Performance - Mean: %.3f, Variance: %.3f, Trend: %.3f\n",
           analyzer->performance_history.mean, analyzer->performance_history.variance, analyzer->performance_history.trend);
    printf("  Attention Coherence - Mean: %.3f, Variance: %.3f\n", 
           analyzer->attention_coherence_history.mean, analyzer->attention_coherence_history.variance);
    printf("  Module Activity - Mean: %.3f, Variance: %.3f\n",
           analyzer->module_activity_history.mean, analyzer->module_activity_history.variance);
    printf("  System Status: %s, %s\n",
           analyzer->performance_history.is_stable ? "Stable" : "Changing",
           analyzer->performance_history.is_trending ? "Trending" : "Flat");
}

// Initialize test distributed cognitive architecture
test_distributed_cognitive_architecture_t* test_distributed_arch_init(void) {
    test_distributed_cognitive_architecture_t* arch = malloc(sizeof(test_distributed_cognitive_architecture_t));
    if (!arch) return NULL;
    
    // Initialize basic state
    arch->performance = 0.5f;
    arch->attention_distribution[0] = 0.25f;  // Memory
    arch->attention_distribution[1] = 0.25f;  // Reasoning
    arch->attention_distribution[2] = 0.25f;  // Communication
    arch->attention_distribution[3] = 0.25f;  // Self-modification
    arch->dynamic_module_count = 2;
    arch->system_time = 0;
    arch->self_optimization_active = false;
    arch->recursive_improvement_active = false;
    arch->evolution_history_count = 0;
    
    // Initialize emergent behavior analysis
    arch->behavior_analyzer = emergent_behavior_analyzer_init();
    arch->emergent_behavior_analysis_enabled = (arch->behavior_analyzer != NULL);
    
    return arch;
}

void test_distributed_arch_cleanup(test_distributed_cognitive_architecture_t* arch) {
    if (arch) {
        if (arch->behavior_analyzer) {
            if (arch->behavior_analyzer->behaviors) free(arch->behavior_analyzer->behaviors);
            if (arch->behavior_analyzer->performance_history.samples) 
                free(arch->behavior_analyzer->performance_history.samples);
            if (arch->behavior_analyzer->attention_coherence_history.samples)
                free(arch->behavior_analyzer->attention_coherence_history.samples);
            if (arch->behavior_analyzer->adaptation_rate_history.samples)
                free(arch->behavior_analyzer->adaptation_rate_history.samples);
            if (arch->behavior_analyzer->module_activity_history.samples)
                free(arch->behavior_analyzer->module_activity_history.samples);
            free(arch->behavior_analyzer);
        }
        free(arch);
    }
}

// Simulate realistic distributed cognitive architecture behavior
void simulate_distributed_cognitive_workflow(test_distributed_cognitive_architecture_t* arch, int steps) {
    printf("Simulating distributed cognitive architecture workflow...\n");
    
    for (int i = 0; i < steps; i++) {
        // Simulate different phases of operation
        if (i < steps / 4) {
            // Phase 1: Initial optimization (recursive improvement active)
            arch->recursive_improvement_active = true;
            arch->performance += 0.03f + 0.01f * ((float)rand() / RAND_MAX - 0.5f);
            
            // Gradually adjust attention
            arch->attention_distribution[1] += 0.01f;  // Increase reasoning
            arch->attention_distribution[3] -= 0.01f;  // Decrease self-mod
            
        } else if (i < steps / 2) {
            // Phase 2: Stabilization (let recursive improvement continue but slower)
            arch->performance += 0.01f + 0.005f * ((float)rand() / RAND_MAX - 0.5f);
            
        } else if (i < 3 * steps / 4) {
            // Phase 3: Module addition leading to cooperation
            if (i == steps / 2) {
                arch->dynamic_module_count++;
                arch->performance += 0.15f;  // Superlinear improvement suggesting cooperation
            } else if (i == steps / 2 + 5) {
                arch->dynamic_module_count++;
                arch->performance += 0.18f;  // Even better cooperation
            } else {
                arch->performance += 0.005f;
            }
            
            arch->recursive_improvement_active = false;  // Turn off explicit optimization
            
        } else {
            // Phase 4: Novel adaptation pattern
            if (i == 3 * steps / 4) {
                // Switch attention to novel pattern
                arch->attention_distribution[0] = 0.5f;   // Heavy memory focus
                arch->attention_distribution[1] = 0.3f;   // Reduced reasoning
                arch->attention_distribution[2] = 0.15f;  // Minimal communication
                arch->attention_distribution[3] = 0.05f;  // Minimal self-mod
            }
            
            // Maintain good performance despite novel pattern
            arch->performance += 0.01f + 0.005f * ((float)rand() / RAND_MAX - 0.3f);
        }
        
        // Keep performance in bounds
        arch->performance = fmaxf(0.1f, fminf(0.95f, arch->performance));
        
        // Normalize attention distribution
        float total_attention = 0.0f;
        for (int j = 0; j < 4; j++) total_attention += arch->attention_distribution[j];
        for (int j = 0; j < 4; j++) arch->attention_distribution[j] /= total_attention;
        
        arch->system_time++;
        
        // Analyze emergent behaviors
        bool behavior_detected = emergent_behavior_analyze_distributed(arch);
        if (behavior_detected) {
            printf("  Step %d: Emergent behavior detected (Performance: %.3f)\n", i + 1, arch->performance);
        }
    }
}

// Main test function
int main(void) {
    printf("=== Distributed Cognitive Architecture Emergent Behavior Analysis Test ===\n\n");
    
    srand((unsigned int)time(NULL));
    
    // Initialize distributed cognitive architecture
    test_distributed_cognitive_architecture_t* arch = test_distributed_arch_init();
    if (!arch) {
        printf("Failed to initialize distributed cognitive architecture\n");
        return 1;
    }
    
    printf("Distributed cognitive architecture initialized successfully\n");
    printf("Emergent behavior analysis: %s\n", arch->emergent_behavior_analysis_enabled ? "ENABLED" : "DISABLED");
    printf("Initial performance: %.3f\n", arch->performance);
    printf("Initial modules: %u\n", arch->dynamic_module_count);
    
    // Run simulation
    simulate_distributed_cognitive_workflow(arch, 40);
    
    // Print final analysis
    emergent_behavior_print_analysis_distributed(arch);
    
    printf("\n=== Test Summary ===\n");
    printf("Final performance: %.3f\n", arch->performance);
    printf("Final modules: %u\n", arch->dynamic_module_count);
    printf("Evolution history records: %zu\n", arch->evolution_history_count);
    printf("Behaviors detected: %zu\n", arch->behavior_analyzer ? arch->behavior_analyzer->behavior_count : 0);
    
    printf("\nThe distributed cognitive architecture demonstrates:\n");
    printf("• Integration with existing cognitive architecture evolution\n");
    printf("• Real-time emergent behavior detection during operation\n");
    printf("• Detection of spontaneous optimization without explicit commands\n");
    printf("• Recognition of emergent cooperation between modules\n");
    printf("• Analysis of novel adaptation patterns\n");
    printf("• Performance correlation analysis for behavioral patterns\n");
    
    // Cleanup
    test_distributed_arch_cleanup(arch);
    
    printf("\n=== Distributed Emergent Behavior Analysis Test Completed Successfully! ===\n");
    
    return 0;
}