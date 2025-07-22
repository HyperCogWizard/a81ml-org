#include "ggml-cogutil.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <assert.h>

// Global variables for logging and configuration
static cogutil_log_level_t g_log_level = COGUTIL_LOG_INFO;
static FILE* g_log_file = NULL;
static bool g_log_initialized = false;

// ==== Type System Implementation ====

cogutil_error_t cogutil_type_registry_init(cogutil_type_registry_t* registry) {
    if (!registry) return COGUTIL_ERROR_NULL_POINTER;
    
    registry->type_capacity = 256;  // Start with reasonable capacity
    registry->types = calloc(registry->type_capacity, sizeof(cogutil_type_t));
    if (!registry->types) return COGUTIL_ERROR_MEMORY_ALLOCATION;
    
    registry->type_count = 0;
    registry->next_type_id = 1;  // Start from 1, 0 is reserved for invalid type
    registry->initialized = true;
    
    // Register fundamental types
    uint16_t node_type_id, link_type_id;
    cogutil_type_register(registry, "Node", true, false, 0, &node_type_id);
    cogutil_type_register(registry, "Link", false, true, 0, &link_type_id);
    
    return COGUTIL_SUCCESS;
}

void cogutil_type_registry_free(cogutil_type_registry_t* registry) {
    if (!registry || !registry->initialized) return;
    
    free(registry->types);
    registry->types = NULL;
    registry->type_count = 0;
    registry->type_capacity = 0;
    registry->initialized = false;
}

cogutil_error_t cogutil_type_register(
    cogutil_type_registry_t* registry,
    const char* name,
    bool is_node,
    bool is_link,
    uint16_t parent_type_id,
    uint16_t* type_id) {
    
    if (!registry || !name || !type_id) return COGUTIL_ERROR_NULL_POINTER;
    if (!registry->initialized) return COGUTIL_ERROR_INVALID_PARAMETER;
    
    // Check if type already exists
    cogutil_type_t* existing = cogutil_type_get_by_name(registry, name);
    if (existing) {
        *type_id = existing->type_id;
        return COGUTIL_SUCCESS;
    }
    
    // Expand capacity if needed
    if (registry->type_count >= registry->type_capacity) {
        size_t new_capacity = registry->type_capacity * 2;
        cogutil_type_t* new_types = realloc(registry->types, 
                                           new_capacity * sizeof(cogutil_type_t));
        if (!new_types) return COGUTIL_ERROR_MEMORY_ALLOCATION;
        
        registry->types = new_types;
        registry->type_capacity = new_capacity;
    }
    
    // Add new type
    cogutil_type_t* new_type = &registry->types[registry->type_count];
    new_type->type_id = registry->next_type_id++;
    cogutil_safe_strcpy(new_type->name, COGUTIL_MAX_TYPE_NAME, name);
    new_type->is_node = is_node;
    new_type->is_link = is_link;
    new_type->parent_type_id = parent_type_id;
    
    *type_id = new_type->type_id;
    registry->type_count++;
    
    return COGUTIL_SUCCESS;
}

cogutil_type_t* cogutil_type_get_by_id(
    cogutil_type_registry_t* registry,
    uint16_t type_id) {
    
    if (!registry || !registry->initialized) return NULL;
    
    for (size_t i = 0; i < registry->type_count; i++) {
        if (registry->types[i].type_id == type_id) {
            return &registry->types[i];
        }
    }
    
    return NULL;
}

cogutil_type_t* cogutil_type_get_by_name(
    cogutil_type_registry_t* registry,
    const char* name) {
    
    if (!registry || !name || !registry->initialized) return NULL;
    
    for (size_t i = 0; i < registry->type_count; i++) {
        if (strcmp(registry->types[i].name, name) == 0) {
            return &registry->types[i];
        }
    }
    
    return NULL;
}

bool cogutil_type_is_subtype(
    cogutil_type_registry_t* registry,
    uint16_t child_type_id,
    uint16_t parent_type_id) {
    
    if (!registry || !registry->initialized) return false;
    if (child_type_id == parent_type_id) return true;
    
    cogutil_type_t* child_type = cogutil_type_get_by_id(registry, child_type_id);
    if (!child_type) return false;
    
    // Traverse up the inheritance hierarchy
    uint16_t current_parent = child_type->parent_type_id;
    while (current_parent != 0) {
        if (current_parent == parent_type_id) return true;
        
        cogutil_type_t* parent = cogutil_type_get_by_id(registry, current_parent);
        if (!parent) break;
        
        current_parent = parent->parent_type_id;
    }
    
    return false;
}

// ==== Configuration Management Implementation ====

cogutil_error_t cogutil_config_init(cogutil_config_t* config) {
    if (!config) return COGUTIL_ERROR_NULL_POINTER;
    
    config->entry_capacity = 64;  // Start with reasonable capacity
    config->entries = calloc(config->entry_capacity, sizeof(cogutil_config_entry_t));
    if (!config->entries) return COGUTIL_ERROR_MEMORY_ALLOCATION;
    
    config->entry_count = 0;
    config->initialized = true;
    
    return COGUTIL_SUCCESS;
}

void cogutil_config_free(cogutil_config_t* config) {
    if (!config || !config->initialized) return;
    
    free(config->entries);
    config->entries = NULL;
    config->entry_count = 0;
    config->entry_capacity = 0;
    config->initialized = false;
}

static cogutil_config_entry_t* find_config_entry(
    cogutil_config_t* config,
    const char* key) {
    
    if (!config || !key) return NULL;
    
    for (size_t i = 0; i < config->entry_count; i++) {
        if (strcmp(config->entries[i].key, key) == 0) {
            return &config->entries[i];
        }
    }
    
    return NULL;
}

static cogutil_error_t add_config_entry(
    cogutil_config_t* config,
    const char* key) {
    
    if (!config || !key) return COGUTIL_ERROR_NULL_POINTER;
    
    // Expand capacity if needed
    if (config->entry_count >= config->entry_capacity) {
        size_t new_capacity = config->entry_capacity * 2;
        cogutil_config_entry_t* new_entries = realloc(config->entries,
                                                      new_capacity * sizeof(cogutil_config_entry_t));
        if (!new_entries) return COGUTIL_ERROR_MEMORY_ALLOCATION;
        
        config->entries = new_entries;
        config->entry_capacity = new_capacity;
    }
    
    cogutil_config_entry_t* entry = &config->entries[config->entry_count];
    cogutil_safe_strcpy(entry->key, COGUTIL_MAX_CONFIG_KEY, key);
    config->entry_count++;
    
    return COGUTIL_SUCCESS;
}

cogutil_error_t cogutil_config_set_string(
    cogutil_config_t* config,
    const char* key,
    const char* value) {
    
    if (!config || !key || !value) return COGUTIL_ERROR_NULL_POINTER;
    if (!config->initialized) return COGUTIL_ERROR_INVALID_PARAMETER;
    
    cogutil_config_entry_t* entry = find_config_entry(config, key);
    if (!entry) {
        cogutil_error_t result = add_config_entry(config, key);
        if (result != COGUTIL_SUCCESS) return result;
        entry = &config->entries[config->entry_count - 1];
    }
    
    cogutil_safe_strcpy(entry->value, COGUTIL_MAX_CONFIG_VALUE, value);
    entry->is_numeric = false;
    
    return COGUTIL_SUCCESS;
}

cogutil_error_t cogutil_config_set_number(
    cogutil_config_t* config,
    const char* key,
    double value) {
    
    if (!config || !key) return COGUTIL_ERROR_NULL_POINTER;
    if (!config->initialized) return COGUTIL_ERROR_INVALID_PARAMETER;
    
    cogutil_config_entry_t* entry = find_config_entry(config, key);
    if (!entry) {
        cogutil_error_t result = add_config_entry(config, key);
        if (result != COGUTIL_SUCCESS) return result;
        entry = &config->entries[config->entry_count - 1];
    }
    
    entry->numeric_value = value;
    entry->is_numeric = true;
    snprintf(entry->value, COGUTIL_MAX_CONFIG_VALUE, "%.6f", value);
    
    return COGUTIL_SUCCESS;
}

cogutil_error_t cogutil_config_set_bool(
    cogutil_config_t* config,
    const char* key,
    bool value) {
    
    if (!config || !key) return COGUTIL_ERROR_NULL_POINTER;
    if (!config->initialized) return COGUTIL_ERROR_INVALID_PARAMETER;
    
    cogutil_config_entry_t* entry = find_config_entry(config, key);
    if (!entry) {
        cogutil_error_t result = add_config_entry(config, key);
        if (result != COGUTIL_SUCCESS) return result;
        entry = &config->entries[config->entry_count - 1];
    }
    
    entry->bool_value = value;
    entry->is_numeric = false;
    cogutil_safe_strcpy(entry->value, COGUTIL_MAX_CONFIG_VALUE, value ? "true" : "false");
    
    return COGUTIL_SUCCESS;
}

cogutil_error_t cogutil_config_get_string(
    cogutil_config_t* config,
    const char* key,
    const char** value) {
    
    if (!config || !key || !value) return COGUTIL_ERROR_NULL_POINTER;
    if (!config->initialized) return COGUTIL_ERROR_INVALID_PARAMETER;
    
    cogutil_config_entry_t* entry = find_config_entry(config, key);
    if (!entry) return COGUTIL_ERROR_CONFIG_NOT_FOUND;
    
    *value = entry->value;
    return COGUTIL_SUCCESS;
}

cogutil_error_t cogutil_config_get_number(
    cogutil_config_t* config,
    const char* key,
    double* value) {
    
    if (!config || !key || !value) return COGUTIL_ERROR_NULL_POINTER;
    if (!config->initialized) return COGUTIL_ERROR_INVALID_PARAMETER;
    
    cogutil_config_entry_t* entry = find_config_entry(config, key);
    if (!entry) return COGUTIL_ERROR_CONFIG_NOT_FOUND;
    
    if (entry->is_numeric) {
        *value = entry->numeric_value;
    } else {
        // Try to parse as number
        char* endptr;
        double parsed = strtod(entry->value, &endptr);
        if (*endptr != '\0') return COGUTIL_ERROR_INVALID_PARAMETER;
        *value = parsed;
    }
    
    return COGUTIL_SUCCESS;
}

cogutil_error_t cogutil_config_get_bool(
    cogutil_config_t* config,
    const char* key,
    bool* value) {
    
    if (!config || !key || !value) return COGUTIL_ERROR_NULL_POINTER;
    if (!config->initialized) return COGUTIL_ERROR_INVALID_PARAMETER;
    
    cogutil_config_entry_t* entry = find_config_entry(config, key);
    if (!entry) return COGUTIL_ERROR_CONFIG_NOT_FOUND;
    
    if (strcmp(entry->value, "true") == 0 || strcmp(entry->value, "1") == 0) {
        *value = true;
    } else if (strcmp(entry->value, "false") == 0 || strcmp(entry->value, "0") == 0) {
        *value = false;
    } else {
        return COGUTIL_ERROR_INVALID_PARAMETER;
    }
    
    return COGUTIL_SUCCESS;
}

cogutil_error_t cogutil_config_load_file(
    cogutil_config_t* config,
    const char* filename) {
    
    if (!config || !filename) return COGUTIL_ERROR_NULL_POINTER;
    if (!config->initialized) return COGUTIL_ERROR_INVALID_PARAMETER;
    
    FILE* file = fopen(filename, "r");
    if (!file) return COGUTIL_ERROR_INVALID_PARAMETER;
    
    char line[512];
    cogutil_error_t result = COGUTIL_SUCCESS;
    
    while (fgets(line, sizeof(line), file)) {
        // Skip comments and empty lines
        char* trimmed = line;
        while (*trimmed == ' ' || *trimmed == '\t') trimmed++;
        if (*trimmed == '#' || *trimmed == '\n' || *trimmed == '\0') continue;
        
        // Parse key=value pairs
        char* equals = strchr(trimmed, '=');
        if (!equals) continue;
        
        *equals = '\0';
        char* key = trimmed;
        char* value = equals + 1;
        
        // Trim trailing whitespace and newline
        char* end = key + strlen(key) - 1;
        while (end > key && (*end == ' ' || *end == '\t')) *end-- = '\0';
        
        end = value + strlen(value) - 1;
        while (end > value && (*end == ' ' || *end == '\t' || *end == '\n')) *end-- = '\0';
        
        result = cogutil_config_set_string(config, key, value);
        if (result != COGUTIL_SUCCESS) break;
    }
    
    fclose(file);
    return result;
}

// ==== Logging System Implementation ====

cogutil_error_t cogutil_log_init(
    cogutil_log_level_t min_level,
    const char* log_file) {
    
    g_log_level = min_level;
    
    if (log_file) {
        g_log_file = fopen(log_file, "a");
        if (!g_log_file) return COGUTIL_ERROR_INVALID_PARAMETER;
    } else {
        g_log_file = stderr;
    }
    
    g_log_initialized = true;
    return COGUTIL_SUCCESS;
}

static const char* level_to_string(cogutil_log_level_t level) {
    switch (level) {
        case COGUTIL_LOG_TRACE: return "TRACE";
        case COGUTIL_LOG_DEBUG: return "DEBUG";
        case COGUTIL_LOG_INFO:  return "INFO";
        case COGUTIL_LOG_WARN:  return "WARN";
        case COGUTIL_LOG_ERROR: return "ERROR";
        case COGUTIL_LOG_FATAL: return "FATAL";
        default: return "UNKNOWN";
    }
}

void cogutil_log(
    cogutil_log_level_t level,
    const char* format,
    ...) {
    
    if (!g_log_initialized || level < g_log_level) return;
    
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(g_log_file, "[%s] %s: ", timestamp, level_to_string(level));
    
    va_list args;
    va_start(args, format);
    vfprintf(g_log_file, format, args);
    va_end(args);
    
    fprintf(g_log_file, "\n");
    fflush(g_log_file);
}

void cogutil_log_location(
    cogutil_log_level_t level,
    const char* file,
    int line,
    const char* function,
    const char* format,
    ...) {
    
    if (!g_log_initialized || level < g_log_level) return;
    
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    
    char timestamp[32];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(g_log_file, "[%s] %s [%s:%d %s]: ", 
            timestamp, level_to_string(level), file, line, function);
    
    va_list args;
    va_start(args, format);
    vfprintf(g_log_file, format, args);
    va_end(args);
    
    fprintf(g_log_file, "\n");
    fflush(g_log_file);
}

void cogutil_log_set_level(cogutil_log_level_t level) {
    g_log_level = level;
}

cogutil_log_level_t cogutil_log_get_level(void) {
    return g_log_level;
}

// ==== Error Handling Implementation ====

const char* cogutil_error_message(cogutil_error_t error) {
    switch (error) {
        case COGUTIL_SUCCESS: return "Success";
        case COGUTIL_ERROR_NULL_POINTER: return "Null pointer error";
        case COGUTIL_ERROR_INVALID_TYPE: return "Invalid type error";
        case COGUTIL_ERROR_MEMORY_ALLOCATION: return "Memory allocation error";
        case COGUTIL_ERROR_INVALID_PARAMETER: return "Invalid parameter error";
        case COGUTIL_ERROR_CONFIG_NOT_FOUND: return "Configuration not found";
        case COGUTIL_ERROR_BUFFER_OVERFLOW: return "Buffer overflow error";
        case COGUTIL_ERROR_TYPE_NOT_FOUND: return "Type not found";
        default: return "Unknown error";
    }
}

bool cogutil_is_success(cogutil_error_t error) {
    return error == COGUTIL_SUCCESS;
}

bool cogutil_is_error(cogutil_error_t error) {
    return error != COGUTIL_SUCCESS;
}

// ==== Memory Management Implementation ====

cogutil_error_t cogutil_memory_pool_init(
    cogutil_memory_pool_t* pool,
    size_t block_size,
    size_t alignment) {
    
    if (!pool || block_size == 0) return COGUTIL_ERROR_NULL_POINTER;
    if (alignment == 0) alignment = sizeof(void*);
    
    pool->memory_block = aligned_alloc(alignment, block_size);
    if (!pool->memory_block) return COGUTIL_ERROR_MEMORY_ALLOCATION;
    
    pool->block_size = block_size;
    pool->used_size = 0;
    pool->alignment = alignment;
    pool->initialized = true;
    
    return COGUTIL_SUCCESS;
}

void cogutil_memory_pool_free(cogutil_memory_pool_t* pool) {
    if (!pool || !pool->initialized) return;
    
    free(pool->memory_block);
    pool->memory_block = NULL;
    pool->block_size = 0;
    pool->used_size = 0;
    pool->initialized = false;
}

void* cogutil_memory_pool_alloc(
    cogutil_memory_pool_t* pool,
    size_t size) {
    
    if (!pool || !pool->initialized || size == 0) return NULL;
    
    // Align size to pool alignment
    size_t aligned_size = (size + pool->alignment - 1) & ~(pool->alignment - 1);
    
    if (pool->used_size + aligned_size > pool->block_size) {
        return NULL;  // Not enough space
    }
    
    void* ptr = (char*)pool->memory_block + pool->used_size;
    pool->used_size += aligned_size;
    
    return ptr;
}

void cogutil_memory_pool_reset(cogutil_memory_pool_t* pool) {
    if (!pool || !pool->initialized) return;
    pool->used_size = 0;
}

void cogutil_memory_pool_stats(
    cogutil_memory_pool_t* pool,
    size_t* total_size,
    size_t* used_size,
    size_t* free_size) {
    
    if (!pool || !pool->initialized) {
        if (total_size) *total_size = 0;
        if (used_size) *used_size = 0;
        if (free_size) *free_size = 0;
        return;
    }
    
    if (total_size) *total_size = pool->block_size;
    if (used_size) *used_size = pool->used_size;
    if (free_size) *free_size = pool->block_size - pool->used_size;
}

// ==== String Utilities Implementation ====

cogutil_error_t cogutil_string_init(
    cogutil_string_t* str,
    size_t initial_capacity) {
    
    if (!str) return COGUTIL_ERROR_NULL_POINTER;
    if (initial_capacity == 0) initial_capacity = 64;
    
    str->data = malloc(initial_capacity);
    if (!str->data) return COGUTIL_ERROR_MEMORY_ALLOCATION;
    
    str->data[0] = '\0';
    str->length = 0;
    str->capacity = initial_capacity;
    
    return COGUTIL_SUCCESS;
}

void cogutil_string_free(cogutil_string_t* str) {
    if (!str) return;
    
    free(str->data);
    str->data = NULL;
    str->length = 0;
    str->capacity = 0;
}

cogutil_error_t cogutil_string_set(
    cogutil_string_t* str,
    const char* content) {
    
    if (!str || !content) return COGUTIL_ERROR_NULL_POINTER;
    
    size_t content_len = strlen(content);
    
    // Resize if needed
    if (content_len + 1 > str->capacity) {
        size_t new_capacity = content_len + 1;
        char* new_data = realloc(str->data, new_capacity);
        if (!new_data) return COGUTIL_ERROR_MEMORY_ALLOCATION;
        
        str->data = new_data;
        str->capacity = new_capacity;
    }
    
    strcpy(str->data, content);
    str->length = content_len;
    
    return COGUTIL_SUCCESS;
}

cogutil_error_t cogutil_string_append(
    cogutil_string_t* str,
    const char* content) {
    
    if (!str || !content) return COGUTIL_ERROR_NULL_POINTER;
    
    size_t content_len = strlen(content);
    size_t new_length = str->length + content_len;
    
    // Resize if needed
    if (new_length + 1 > str->capacity) {
        size_t new_capacity = (new_length + 1) * 2;  // Double the capacity
        char* new_data = realloc(str->data, new_capacity);
        if (!new_data) return COGUTIL_ERROR_MEMORY_ALLOCATION;
        
        str->data = new_data;
        str->capacity = new_capacity;
    }
    
    strcat(str->data, content);
    str->length = new_length;
    
    return COGUTIL_SUCCESS;
}

cogutil_error_t cogutil_string_format(
    cogutil_string_t* str,
    const char* format,
    ...) {
    
    if (!str || !format) return COGUTIL_ERROR_NULL_POINTER;
    
    va_list args;
    va_start(args, format);
    
    // First, calculate required size
    va_list args_copy;
    va_copy(args_copy, args);
    int needed = vsnprintf(NULL, 0, format, args_copy);
    va_end(args_copy);
    
    if (needed < 0) {
        va_end(args);
        return COGUTIL_ERROR_INVALID_PARAMETER;
    }
    
    // Resize if needed
    size_t required_capacity = needed + 1;
    if (required_capacity > str->capacity) {
        char* new_data = realloc(str->data, required_capacity);
        if (!new_data) {
            va_end(args);
            return COGUTIL_ERROR_MEMORY_ALLOCATION;
        }
        
        str->data = new_data;
        str->capacity = required_capacity;
    }
    
    // Format the string
    vsnprintf(str->data, str->capacity, format, args);
    str->length = needed;
    
    va_end(args);
    return COGUTIL_SUCCESS;
}

size_t cogutil_string_length(const cogutil_string_t* str) {
    return str ? str->length : 0;
}

const char* cogutil_string_data(const cogutil_string_t* str) {
    return str ? str->data : NULL;
}

int cogutil_string_compare(
    const cogutil_string_t* str1,
    const cogutil_string_t* str2) {
    
    if (!str1 || !str2) return 0;
    return strcmp(str1->data, str2->data);
}

// ==== Validation Utilities Implementation ====

cogutil_error_t cogutil_validate_pointer(const void* ptr) {
    return ptr ? COGUTIL_SUCCESS : COGUTIL_ERROR_NULL_POINTER;
}

cogutil_error_t cogutil_validate_string(const char* str) {
    if (!str) return COGUTIL_ERROR_NULL_POINTER;
    return (strlen(str) > 0) ? COGUTIL_SUCCESS : COGUTIL_ERROR_INVALID_PARAMETER;
}

cogutil_error_t cogutil_validate_bounds(
    size_t index,
    size_t array_size) {
    
    return (index < array_size) ? COGUTIL_SUCCESS : COGUTIL_ERROR_INVALID_PARAMETER;
}

cogutil_error_t cogutil_validate_type_id(
    cogutil_type_registry_t* registry,
    uint16_t type_id) {
    
    if (!registry) return COGUTIL_ERROR_NULL_POINTER;
    
    cogutil_type_t* type = cogutil_type_get_by_id(registry, type_id);
    return type ? COGUTIL_SUCCESS : COGUTIL_ERROR_TYPE_NOT_FOUND;
}

// ==== Utility Functions Implementation ====

uint64_t cogutil_get_timestamp_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

uint64_t cogutil_get_timestamp_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

void cogutil_sleep_ms(uint32_t milliseconds) {
    usleep(milliseconds * 1000);
}

uint32_t cogutil_random_range(uint32_t min, uint32_t max) {
    if (min >= max) return min;
    return min + (rand() % (max - min + 1));
}

uint32_t cogutil_hash_string(const char* str) {
    if (!str) return 0;
    
    uint32_t hash = 5381;
    int c;
    
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;  // hash * 33 + c
    }
    
    return hash;
}

cogutil_error_t cogutil_safe_strcpy(
    char* dest,
    size_t dest_size,
    const char* src) {
    
    if (!dest || !src || dest_size == 0) return COGUTIL_ERROR_NULL_POINTER;
    
    size_t src_len = strlen(src);
    if (src_len >= dest_size) return COGUTIL_ERROR_BUFFER_OVERFLOW;
    
    strcpy(dest, src);
    return COGUTIL_SUCCESS;
}

cogutil_error_t cogutil_safe_strcat(
    char* dest,
    size_t dest_size,
    const char* src) {
    
    if (!dest || !src || dest_size == 0) return COGUTIL_ERROR_NULL_POINTER;
    
    size_t dest_len = strlen(dest);
    size_t src_len = strlen(src);
    
    if (dest_len + src_len >= dest_size) return COGUTIL_ERROR_BUFFER_OVERFLOW;
    
    strcat(dest, src);
    return COGUTIL_SUCCESS;
}