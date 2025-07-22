#include "ggml-cogutil.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("Testing CogUtil implementation...\n");
    
    // Test 1: Type Registry
    printf("\n=== Testing Type Registry ===\n");
    cogutil_type_registry_t registry;
    cogutil_error_t result = cogutil_type_registry_init(&registry);
    assert(result == COGUTIL_SUCCESS);
    printf("✓ Type registry initialized\n");
    
    uint16_t concept_type_id;
    result = cogutil_type_register(&registry, "ConceptNode", true, false, 1, &concept_type_id);
    assert(result == COGUTIL_SUCCESS);
    printf("✓ ConceptNode type registered with ID: %d\n", concept_type_id);
    
    cogutil_type_t* concept_type = cogutil_type_get_by_name(&registry, "ConceptNode");
    assert(concept_type != NULL);
    assert(concept_type->is_node == true);
    assert(concept_type->is_link == false);
    printf("✓ ConceptNode type retrieved by name\n");
    
    cogutil_type_registry_free(&registry);
    printf("✓ Type registry freed\n");
    
    // Test 2: Configuration Management
    printf("\n=== Testing Configuration Management ===\n");
    cogutil_config_t config;
    result = cogutil_config_init(&config);
    assert(result == COGUTIL_SUCCESS);
    printf("✓ Configuration initialized\n");
    
    result = cogutil_config_set_string(&config, "atomspace_size", "2048");
    assert(result == COGUTIL_SUCCESS);
    
    result = cogutil_config_set_number(&config, "attention_decay", 0.95);
    assert(result == COGUTIL_SUCCESS);
    
    result = cogutil_config_set_bool(&config, "enable_logging", true);
    assert(result == COGUTIL_SUCCESS);
    printf("✓ Configuration values set\n");
    
    const char* size_str;
    result = cogutil_config_get_string(&config, "atomspace_size", &size_str);
    assert(result == COGUTIL_SUCCESS);
    assert(strcmp(size_str, "2048") == 0);
    
    double decay_rate;
    result = cogutil_config_get_number(&config, "attention_decay", &decay_rate);
    assert(result == COGUTIL_SUCCESS);
    assert(decay_rate == 0.95);
    
    bool logging_enabled;
    result = cogutil_config_get_bool(&config, "enable_logging", &logging_enabled);
    assert(result == COGUTIL_SUCCESS);
    assert(logging_enabled == true);
    printf("✓ Configuration values retrieved correctly\n");
    
    cogutil_config_free(&config);
    printf("✓ Configuration freed\n");
    
    // Test 3: Logging System
    printf("\n=== Testing Logging System ===\n");
    result = cogutil_log_init(COGUTIL_LOG_DEBUG, NULL);
    assert(result == COGUTIL_SUCCESS);
    printf("✓ Logging system initialized\n");
    
    COGUTIL_LOG_INFO("Test info message");
    COGUTIL_LOG_DEBUG("Test debug message");
    COGUTIL_LOG_WARN("Test warning message");
    printf("✓ Logging messages sent\n");
    
    // Test 4: String Utilities
    printf("\n=== Testing String Utilities ===\n");
    cogutil_string_t str;
    result = cogutil_string_init(&str, 64);
    assert(result == COGUTIL_SUCCESS);
    printf("✓ String initialized\n");
    
    result = cogutil_string_set(&str, "Hello");
    assert(result == COGUTIL_SUCCESS);
    assert(cogutil_string_length(&str) == 5);
    
    result = cogutil_string_append(&str, " World");
    assert(result == COGUTIL_SUCCESS);
    assert(cogutil_string_length(&str) == 11);
    assert(strcmp(cogutil_string_data(&str), "Hello World") == 0);
    printf("✓ String operations work correctly\n");
    
    result = cogutil_string_format(&str, "Number: %d, Float: %.2f", 42, 3.14);
    assert(result == COGUTIL_SUCCESS);
    printf("✓ String formatted: %s\n", cogutil_string_data(&str));
    
    cogutil_string_free(&str);
    printf("✓ String freed\n");
    
    // Test 5: Memory Pool
    printf("\n=== Testing Memory Pool ===\n");
    cogutil_memory_pool_t pool;
    result = cogutil_memory_pool_init(&pool, 1024, 8);
    assert(result == COGUTIL_SUCCESS);
    printf("✓ Memory pool initialized\n");
    
    void* ptr1 = cogutil_memory_pool_alloc(&pool, 64);
    assert(ptr1 != NULL);
    
    void* ptr2 = cogutil_memory_pool_alloc(&pool, 128);
    assert(ptr2 != NULL);
    
    size_t total, used, free_size;
    cogutil_memory_pool_stats(&pool, &total, &used, &free_size);
    printf("✓ Memory pool stats: %zu total, %zu used, %zu free\n", total, used, free_size);
    
    cogutil_memory_pool_reset(&pool);
    cogutil_memory_pool_stats(&pool, &total, &used, &free_size);
    assert(used == 0);
    printf("✓ Memory pool reset successfully\n");
    
    cogutil_memory_pool_free(&pool);
    printf("✓ Memory pool freed\n");
    
    // Test 6: Validation Utilities
    printf("\n=== Testing Validation Utilities ===\n");
    result = cogutil_validate_pointer(&registry);
    assert(result == COGUTIL_SUCCESS);
    
    result = cogutil_validate_pointer(NULL);
    assert(result == COGUTIL_ERROR_NULL_POINTER);
    
    result = cogutil_validate_string("valid_string");
    assert(result == COGUTIL_SUCCESS);
    
    result = cogutil_validate_string("");
    assert(result == COGUTIL_ERROR_INVALID_PARAMETER);
    
    result = cogutil_validate_bounds(5, 10);
    assert(result == COGUTIL_SUCCESS);
    
    result = cogutil_validate_bounds(10, 10);
    assert(result == COGUTIL_ERROR_INVALID_PARAMETER);
    printf("✓ Validation utilities work correctly\n");
    
    // Test 7: Utility Functions
    printf("\n=== Testing Utility Functions ===\n");
    uint64_t timestamp = cogutil_get_timestamp_ms();
    assert(timestamp > 0);
    printf("✓ Timestamp: %lu ms\n", timestamp);
    
    uint32_t hash1 = cogutil_hash_string("test");
    uint32_t hash2 = cogutil_hash_string("test");
    uint32_t hash3 = cogutil_hash_string("different");
    assert(hash1 == hash2);
    assert(hash1 != hash3);
    printf("✓ String hashing works: %u, %u, %u\n", hash1, hash2, hash3);
    
    char buffer[64];
    result = cogutil_safe_strcpy(buffer, sizeof(buffer), "Safe copy test");
    assert(result == COGUTIL_SUCCESS);
    assert(strcmp(buffer, "Safe copy test") == 0);
    printf("✓ Safe string copy works\n");
    
    // Test error messages
    printf("\n=== Testing Error Handling ===\n");
    const char* error_msg = cogutil_error_message(COGUTIL_ERROR_NULL_POINTER);
    printf("✓ Error message for NULL_POINTER: %s\n", error_msg);
    
    assert(cogutil_is_success(COGUTIL_SUCCESS) == true);
    assert(cogutil_is_error(COGUTIL_ERROR_NULL_POINTER) == true);
    printf("✓ Error status checking works\n");
    
    printf("\n🎉 All CogUtil tests passed! 🎉\n");
    return 0;
}