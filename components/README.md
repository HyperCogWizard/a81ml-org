# Component Dependency System

This directory contains component specifications for the cognitive architecture components in the ggml ecosystem. Each component is defined in a YAML file that specifies its dependencies, interface, and build requirements.

## Overview

The component dependency system ensures that:
- Components explicitly declare their dependencies
- Dependencies are validated at build time
- Circular dependencies are detected and prevented
- Components are built in the correct order
- Missing dependencies cause build failures

## Component Structure

Each component specification contains:

```yaml
name: component-name
version: "1.0.0"
type: component-type
description: "Component description"

source:
  header: "path/to/header.h"
  implementation: "path/to/implementation.c"

requires:
  - dependency1
  - dependency2

provides:
  - type1
  - type2

functions:
  - function1
  - function2

build:
  cmake_target: target-name
  compile_definitions:
    - DEFINE1
  link_libraries:
    - lib1

tests:
  - test1
  - test2

documentation:
  - "doc1.md"

tags:
  - tag1
  - tag2
```

## Available Components

### cogutil
- **Type**: utility-library
- **Description**: OpenCog utility library providing essential foundation services
- **Dependencies**: None (foundation component)
- **Provides**: Type system, configuration, logging, memory management, validation

### atomspace
- **Type**: cognitive-component
- **Description**: OpenCog AtomSpace implementation with hypergraph knowledge representation
- **Dependencies**: cogutil
- **Provides**: AtomSpace, atoms, truth values, attention values, PLN reasoning

## Dependency Validation

The component dependency system is validated using the `scripts/validate-components.py` script, which is automatically run during the CMake configuration phase.

### Manual Validation

You can manually validate components using:

```bash
# List all components
python3 scripts/validate-components.py --list

# Validate dependencies
python3 scripts/validate-components.py --validate

# Check if source files exist
python3 scripts/validate-components.py --check-files

# Show dependency order
python3 scripts/validate-components.py --dependency-order

# Run all validations
python3 scripts/validate-components.py --validate --check-files
```

### Build Integration

The validation is integrated into the CMake build system:

1. **Configure Phase**: Dependencies are validated when CMake is run
2. **Build Target**: `make validate-components` can be run to validate manually
3. **Failure Handling**: Configuration fails if dependencies are invalid

## Adding New Components

To add a new component:

1. Create a new YAML file in the `components/` directory
2. Follow the component specification format
3. List any dependencies in the `requires` section
4. Run validation to ensure no circular dependencies
5. Update this README if needed

### Example: Adding a new component

```yaml
name: my-component
version: "1.0.0"
type: cognitive-component
description: "My new cognitive component"

source:
  header: "ggml/include/ggml-my-component.h"
  implementation: "ggml/src/ggml-my-component.c"

requires:
  - cogutil
  - atomspace

provides:
  - my_component_t

functions:
  - my_component_init
  - my_component_process

build:
  cmake_target: ggml-base

tests:
  - test_my_component

documentation:
  - "docs/my-component.md"

tags:
  - cognitive
  - processing
```

## Dependency Rules

1. **Foundation Rule**: `cogutil` is the foundation component with no dependencies
2. **Acyclic Rule**: No circular dependencies are allowed
3. **Explicit Rule**: All dependencies must be explicitly declared
4. **Validation Rule**: All dependencies must exist and be valid
5. **Order Rule**: Components must be buildable in dependency order

## Error Handling

If dependency validation fails:

1. **Missing Dependency**: Listed dependency doesn't exist
2. **Circular Dependency**: Circular reference detected
3. **Missing Files**: Source files don't exist
4. **Invalid YAML**: Component specification is malformed

All errors will cause the build to fail with descriptive error messages.

## Implementation Details

The dependency system consists of:

- **Component Specifications**: YAML files defining each component
- **Validation Script**: Python script that validates dependencies
- **CMake Integration**: Build system integration for automatic validation
- **Documentation**: This README and component-specific docs

This ensures that the cognitive architecture maintains proper component isolation and dependency management, making the system more maintainable and preventing subtle dependency-related bugs.