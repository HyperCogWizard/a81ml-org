#pragma once

//
// OpenCog Utility Library (CogUtil) for GGML
//
// This header provides core utility functions that form the foundation
// of OpenCog systems, including type management, configuration,
// logging, error handling, and memory management utilities.
//

#include "ggml.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Type system definitions
#define COGUTIL_MAX_TYPE_NAME 64
#define COGUTIL_MAX_LOG_MESSAGE 512
#define COGUTIL_MAX_CONFIG_KEY 64
#define COGUTIL_MAX_CONFIG_VALUE 256

// Error codes
typedef enum {
    COGUTIL_SUCCESS = 0,
    COGUTIL_ERROR_NULL_POINTER = -1,
    COGUTIL_ERROR_INVALID_TYPE = -2,
    COGUTIL_ERROR_MEMORY_ALLOCATION = -3,
    COGUTIL_ERROR_INVALID_PARAMETER = -4,
    COGUTIL_ERROR_CONFIG_NOT_FOUND = -5,
    COGUTIL_ERROR_BUFFER_OVERFLOW = -6,
    COGUTIL_ERROR_TYPE_NOT_FOUND = -7
} cogutil_error_t;

// Log levels
typedef enum {
    COGUTIL_LOG_TRACE = 0,
    COGUTIL_LOG_DEBUG = 1,
    COGUTIL_LOG_INFO = 2,
    COGUTIL_LOG_WARN = 3,
    COGUTIL_LOG_ERROR = 4,
    COGUTIL_LOG_FATAL = 5
} cogutil_log_level_t;

// Type handle for OpenCog types
typedef struct {
    uint16_t type_id;
    char name[COGUTIL_MAX_TYPE_NAME];
    bool is_node;
    bool is_link;
    uint16_t parent_type_id;
} cogutil_type_t;

// Type registry
typedef struct {
    cogutil_type_t* types;
    size_t type_count;
    size_t type_capacity;
    uint16_t next_type_id;
    bool initialized;
} cogutil_type_registry_t;

// Configuration entry
typedef struct {
    char key[COGUTIL_MAX_CONFIG_KEY];
    char value[COGUTIL_MAX_CONFIG_VALUE];
    bool is_numeric;
    union {
        double numeric_value;
        bool bool_value;
    };
} cogutil_config_entry_t;

// Configuration manager
typedef struct {
    cogutil_config_entry_t* entries;
    size_t entry_count;
    size_t entry_capacity;
    bool initialized;
} cogutil_config_t;

// Memory pool for efficient allocation
typedef struct {
    void* memory_block;
    size_t block_size;
    size_t used_size;
    size_t alignment;
    bool initialized;
} cogutil_memory_pool_t;

// String utilities
typedef struct {
    char* data;
    size_t length;
    size_t capacity;
} cogutil_string_t;

// ==== Type System Functions ====

// Initialize the type registry
GGML_API cogutil_error_t cogutil_type_registry_init(cogutil_type_registry_t* registry);

// Free the type registry
GGML_API void cogutil_type_registry_free(cogutil_type_registry_t* registry);

// Register a new type
GGML_API cogutil_error_t cogutil_type_register(
    cogutil_type_registry_t* registry,
    const char* name,
    bool is_node,
    bool is_link,
    uint16_t parent_type_id,
    uint16_t* type_id);

// Get type by ID
GGML_API cogutil_type_t* cogutil_type_get_by_id(
    cogutil_type_registry_t* registry,
    uint16_t type_id);

// Get type by name
GGML_API cogutil_type_t* cogutil_type_get_by_name(
    cogutil_type_registry_t* registry,
    const char* name);

// Check if type is a subtype of another
GGML_API bool cogutil_type_is_subtype(
    cogutil_type_registry_t* registry,
    uint16_t child_type_id,
    uint16_t parent_type_id);

// ==== Configuration Management ====

// Initialize configuration manager
GGML_API cogutil_error_t cogutil_config_init(cogutil_config_t* config);

// Free configuration manager
GGML_API void cogutil_config_free(cogutil_config_t* config);

// Set string configuration value
GGML_API cogutil_error_t cogutil_config_set_string(
    cogutil_config_t* config,
    const char* key,
    const char* value);

// Set numeric configuration value
GGML_API cogutil_error_t cogutil_config_set_number(
    cogutil_config_t* config,
    const char* key,
    double value);

// Set boolean configuration value
GGML_API cogutil_error_t cogutil_config_set_bool(
    cogutil_config_t* config,
    const char* key,
    bool value);

// Get string configuration value
GGML_API cogutil_error_t cogutil_config_get_string(
    cogutil_config_t* config,
    const char* key,
    const char** value);

// Get numeric configuration value
GGML_API cogutil_error_t cogutil_config_get_number(
    cogutil_config_t* config,
    const char* key,
    double* value);

// Get boolean configuration value
GGML_API cogutil_error_t cogutil_config_get_bool(
    cogutil_config_t* config,
    const char* key,
    bool* value);

// Load configuration from file
GGML_API cogutil_error_t cogutil_config_load_file(
    cogutil_config_t* config,
    const char* filename);

// ==== Logging System ====

// Initialize logging system
GGML_API cogutil_error_t cogutil_log_init(
    cogutil_log_level_t min_level,
    const char* log_file);

// Log a message
GGML_API void cogutil_log(
    cogutil_log_level_t level,
    const char* format,
    ...);

// Log with location information
GGML_API void cogutil_log_location(
    cogutil_log_level_t level,
    const char* file,
    int line,
    const char* function,
    const char* format,
    ...);

// Set log level
GGML_API void cogutil_log_set_level(cogutil_log_level_t level);

// Get current log level
GGML_API cogutil_log_level_t cogutil_log_get_level(void);

// Convenience macros for logging
#define COGUTIL_LOG_TRACE(...) cogutil_log_location(COGUTIL_LOG_TRACE, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define COGUTIL_LOG_DEBUG(...) cogutil_log_location(COGUTIL_LOG_DEBUG, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define COGUTIL_LOG_INFO(...) cogutil_log_location(COGUTIL_LOG_INFO, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define COGUTIL_LOG_WARN(...) cogutil_log_location(COGUTIL_LOG_WARN, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define COGUTIL_LOG_ERROR(...) cogutil_log_location(COGUTIL_LOG_ERROR, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define COGUTIL_LOG_FATAL(...) cogutil_log_location(COGUTIL_LOG_FATAL, __FILE__, __LINE__, __func__, __VA_ARGS__)

// ==== Error Handling ====

// Get error message for error code
GGML_API const char* cogutil_error_message(cogutil_error_t error);

// Check if error code represents success
GGML_API bool cogutil_is_success(cogutil_error_t error);

// Check if error code represents failure
GGML_API bool cogutil_is_error(cogutil_error_t error);

// ==== Memory Management ====

// Initialize memory pool
GGML_API cogutil_error_t cogutil_memory_pool_init(
    cogutil_memory_pool_t* pool,
    size_t block_size,
    size_t alignment);

// Free memory pool
GGML_API void cogutil_memory_pool_free(cogutil_memory_pool_t* pool);

// Allocate from memory pool
GGML_API void* cogutil_memory_pool_alloc(
    cogutil_memory_pool_t* pool,
    size_t size);

// Reset memory pool (mark all memory as available)
GGML_API void cogutil_memory_pool_reset(cogutil_memory_pool_t* pool);

// Get memory pool usage statistics
GGML_API void cogutil_memory_pool_stats(
    cogutil_memory_pool_t* pool,
    size_t* total_size,
    size_t* used_size,
    size_t* free_size);

// ==== String Utilities ====

// Initialize string
GGML_API cogutil_error_t cogutil_string_init(
    cogutil_string_t* str,
    size_t initial_capacity);

// Free string
GGML_API void cogutil_string_free(cogutil_string_t* str);

// Set string content
GGML_API cogutil_error_t cogutil_string_set(
    cogutil_string_t* str,
    const char* content);

// Append to string
GGML_API cogutil_error_t cogutil_string_append(
    cogutil_string_t* str,
    const char* content);

// Format string
GGML_API cogutil_error_t cogutil_string_format(
    cogutil_string_t* str,
    const char* format,
    ...);

// Get string length
GGML_API size_t cogutil_string_length(const cogutil_string_t* str);

// Get string data
GGML_API const char* cogutil_string_data(const cogutil_string_t* str);

// String comparison
GGML_API int cogutil_string_compare(
    const cogutil_string_t* str1,
    const cogutil_string_t* str2);

// ==== Validation Utilities ====

// Validate pointer is not null
GGML_API cogutil_error_t cogutil_validate_pointer(const void* ptr);

// Validate string is not empty
GGML_API cogutil_error_t cogutil_validate_string(const char* str);

// Validate array bounds
GGML_API cogutil_error_t cogutil_validate_bounds(
    size_t index,
    size_t array_size);

// Validate type ID
GGML_API cogutil_error_t cogutil_validate_type_id(
    cogutil_type_registry_t* registry,
    uint16_t type_id);

// ==== Utility Functions ====

// Get current timestamp in milliseconds
GGML_API uint64_t cogutil_get_timestamp_ms(void);

// Get current timestamp in microseconds
GGML_API uint64_t cogutil_get_timestamp_us(void);

// Sleep for specified milliseconds
GGML_API void cogutil_sleep_ms(uint32_t milliseconds);

// Generate random number in range
GGML_API uint32_t cogutil_random_range(uint32_t min, uint32_t max);

// Compute hash of string
GGML_API uint32_t cogutil_hash_string(const char* str);

// Safe string copy
GGML_API cogutil_error_t cogutil_safe_strcpy(
    char* dest,
    size_t dest_size,
    const char* src);

// Safe string concatenation
GGML_API cogutil_error_t cogutil_safe_strcat(
    char* dest,
    size_t dest_size,
    const char* src);

#ifdef __cplusplus
}
#endif