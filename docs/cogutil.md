# CogUtil - OpenCog Utility Library for GGML

CogUtil provides essential utility functions that form the foundation of OpenCog systems within the GGML infrastructure. This library offers core data structures, configuration management, logging, error handling, and memory management utilities specifically designed for cognitive architectures.

## Features

### 🧠 Type System
- Enhanced type registry for OpenCog atom types
- Inheritance hierarchy support
- Type validation and lookup by name or ID
- Thread-safe type registration

### ⚙️ Configuration Management
- Flexible configuration system supporting strings, numbers, and booleans
- File-based configuration loading
- Runtime configuration updates
- Type-safe value retrieval

### 📝 Logging System
- Structured logging with multiple levels (TRACE, DEBUG, INFO, WARN, ERROR, FATAL)
- File output support with timestamps
- Location tracking (file, line, function)
- Convenient logging macros

### 🛡️ Error Handling
- Comprehensive error codes with descriptive messages
- Success/error status checking
- Consistent error propagation

### 🧵 String Utilities
- Dynamic string management with automatic resizing
- Safe string operations preventing buffer overflows
- Format string support with variable arguments
- String comparison and manipulation

### 💾 Memory Management
- Efficient memory pool allocation
- Aligned memory allocation support
- Memory usage statistics
- Pool reset for reuse

### ✅ Validation Utilities
- Pointer validation
- String validation
- Array bounds checking
- Type ID validation

## Quick Start

```c
#include "ggml-cogutil.h"

int main() {
    // Initialize logging
    cogutil_log_init(COGUTIL_LOG_INFO, "cognitive.log");
    
    // Log a message
    COGUTIL_LOG_INFO("Starting cognitive system");
    
    // Initialize type registry
    cogutil_type_registry_t registry;
    cogutil_type_registry_init(&registry);
    
    // Register a type
    uint16_t concept_type_id;
    cogutil_type_register(&registry, "ConceptNode", true, false, 1, &concept_type_id);
    
    // Initialize configuration
    cogutil_config_t config;
    cogutil_config_init(&config);
    cogutil_config_set_number(&config, "attention_decay", 0.95);
    
    double decay_rate;
    cogutil_config_get_number(&config, "attention_decay", &decay_rate);
    
    // Cleanup
    cogutil_config_free(&config);
    cogutil_type_registry_free(&registry);
    
    return 0;
}
```

## Integration with OpenCog

The OpenCog AtomSpace integration automatically uses CogUtil for:

- **Type Management**: All atom types are registered and validated through CogUtil
- **Logging**: AtomSpace operations are logged with structured messages
- **Validation**: Input parameters are validated using CogUtil utilities
- **Error Handling**: Consistent error reporting throughout the system

### Example OpenCog Usage

```c
#include "ggml-opencog.h"

// Initialize AtomSpace (automatically uses CogUtil)
struct ggml_context* ctx = ggml_init(params);
opencog_atomspace_t* atomspace = opencog_atomspace_init(ctx);

// Add atoms (with enhanced validation and logging)
uint64_t human_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "Human");
uint64_t mortal_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "Mortal");

// Create links
uint64_t outgoing[] = {human_id, mortal_id};
uint64_t inheritance_id = opencog_add_link(atomspace, OPENCOG_INHERITANCE_LINK, 
                                          outgoing, 2);
```

## API Reference

### Type System Functions

- `cogutil_type_registry_init()` - Initialize type registry
- `cogutil_type_register()` - Register new atom type
- `cogutil_type_get_by_id()` - Lookup type by ID
- `cogutil_type_get_by_name()` - Lookup type by name
- `cogutil_type_is_subtype()` - Check inheritance relationship

### Configuration Functions

- `cogutil_config_init()` - Initialize configuration manager
- `cogutil_config_set_string/number/bool()` - Set configuration values
- `cogutil_config_get_string/number/bool()` - Get configuration values
- `cogutil_config_load_file()` - Load configuration from file

### Logging Functions

- `cogutil_log_init()` - Initialize logging system
- `COGUTIL_LOG_INFO()` - Log info message with location
- `COGUTIL_LOG_DEBUG()` - Log debug message with location
- `COGUTIL_LOG_ERROR()` - Log error message with location

### Memory Management Functions

- `cogutil_memory_pool_init()` - Initialize memory pool
- `cogutil_memory_pool_alloc()` - Allocate from pool
- `cogutil_memory_pool_reset()` - Reset pool for reuse
- `cogutil_memory_pool_stats()` - Get usage statistics

### String Utilities

- `cogutil_string_init()` - Initialize dynamic string
- `cogutil_string_set()` - Set string content
- `cogutil_string_append()` - Append to string
- `cogutil_string_format()` - Format string with printf-style

### Validation Functions

- `cogutil_validate_pointer()` - Check for null pointers
- `cogutil_validate_string()` - Check for empty strings
- `cogutil_validate_bounds()` - Check array bounds
- `cogutil_safe_strcpy()` - Safe string copying

## Building

CogUtil is automatically built as part of the GGML build system:

```bash
cd ggml
mkdir build && cd build
cmake ..
make -j8
```

The library is included in `libggml-base` and available when linking with GGML applications.

## Performance Considerations

- **Memory Pools**: Use for frequent allocations to reduce malloc overhead
- **String Utilities**: Minimize reallocations by reserving appropriate capacity
- **Logging**: Higher log levels (DEBUG, TRACE) may impact performance in release builds
- **Type Registry**: Type lookups are O(n) - cache frequently used type IDs

## Thread Safety

- **Logging System**: Thread-safe when writing to the same log file
- **Type Registry**: Not thread-safe - initialize once at startup
- **Configuration**: Not thread-safe - use locking for concurrent access
- **Memory Pools**: Not thread-safe - use separate pools per thread

## Error Handling

All CogUtil functions return `cogutil_error_t` status codes:

```c
cogutil_error_t result = cogutil_config_set_string(&config, "key", "value");
if (result != COGUTIL_SUCCESS) {
    COGUTIL_LOG_ERROR("Configuration failed: %s", cogutil_error_message(result));
    return -1;
}
```

## Contributing

When extending CogUtil:

1. Follow the existing naming conventions (`cogutil_*`)
2. Return `cogutil_error_t` for error handling
3. Add comprehensive parameter validation
4. Include structured logging for debugging
5. Update this documentation

---

CogUtil provides the essential foundation for building robust, scalable cognitive architectures on GGML infrastructure, enabling the OpenCog framework to operate efficiently within distributed tensor computation environments.