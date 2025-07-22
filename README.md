# Distributed Cognitive Architecture for ggml-org-central

This project transforms the ggml-org-central repository into a distributed network of agentic cognitive grammar, implementing a self-aware cognitive flow that serves as both a technical architecture and a living diagram of emergent intelligence.

## Overview

The distributed cognitive system represents a paradigm shift from traditional tensor computation to an ecosystem of autonomous agents, each operating as a kernel of cognitive grammar. These agents exchange tensor-shaped data structures to realize emergent intelligence through recursive coordination.

## Quick Start

### Prerequisites

- CMake 3.14+
- C/C++ compiler with C99 support
- Math library support

### Building

```bash
# Clone and build the main project
cd ggml
mkdir build && cd build
cmake ..
make -j8

# Run the cognitive agents demo
./bin/cognitive-agents-demo
```

## Architecture Components

### 🧠 Core Subsystems

1. **Memory System**: Distributed Hypergraph AtomSpace (Tensorized)
   - Hypergraph knowledge representation using ggml tensors
   - Distributed storage across multiple backends
   - Semantic indexing and retrieval

2. **Task System**: Agentic Task Orchestrator (Recursive, Symbolic+Neural)
   - Grammar-constrained task decomposition
   - Recursive execution planning
   - Integration with GBNF grammars

3. **AI System**: Hybrid Reasoning Engine (PLN + MOSES + Pattern Matcher)
   - Probabilistic Logic Networks for belief reasoning
   - Meta-Optimizing Semantic Evolution
   - Pattern matching via tensor operations

4. **Autonomy System**: Self-Modifying ECAN Attention Economy
   - Economic attention allocation
   - Performance-based resource management
   - Recursive self-improvement

### 🌐 Distributed Communication

The system leverages and extends the existing ggml RPC infrastructure:

- **Tensor Membrane Exchange**: Cognitive states as serialized tensor packets
- **Attention Routing**: Messages routed based on salience and relevance
- **Meta-Cognitive Headers**: Enhanced RPC with cognitive metadata

## Key Features

### 🔄 Recursive Intelligence
- Agents model other agents' cognitive states
- Meta-reasoning about reasoning processes
- Self-improvement through recursive optimization

### 🏗️ Emergent Architecture
- Spontaneous role specialization
- Adaptive communication patterns
- Hierarchical structures from flat networks

### 💡 Cognitive Grammar
- GBNF-based reasoning constraints
- Grammar-guided task decomposition
- Structured cognitive operations

## Documentation

- [**Distributed Cognitive Architecture**](docs/distributed-cognitive-architecture.md) - Complete architectural overview
- [**Implementation Guide**](docs/implementation-guide.md) - Practical development guide
- [**Cognitive Grammar Examples**](docs/cognitive-grammar-examples.md) - Grammar system usage

## Demo Applications

### Consciousness Exploration
```bash
# Demonstrates philosophical reasoning between agents
./bin/cognitive-agents-demo
```

The demo includes:
- **Philosopher Agent**: Specializes in consciousness concepts
- **Scientist Agent**: Focuses on neuroscience perspective
- **Collaborative Reasoning**: Cross-agent knowledge exchange
- **Attention Management**: Dynamic resource allocation

### Example Output
```
=== Consciousness Exploration Demo ===
Created cognitive agent 1751328539001 at localhost:8001
Created cognitive agent 1751328539002 at localhost:8002

Adding knowledge to agents...
Added knowledge: consciousness (nodes: 1)
Added knowledge: philosophy_of_mind (nodes: 2)
Added knowledge: neuroscience (nodes: 1)

Simulating consciousness exploration...
Allocated 0.60 attention to type 3 (total: 0.60/1.00)
Agent 1751328539001 sent cognitive tensor (type 1, attention 0.80, salience 0.56)
```

## Cognitive Grammar Examples

### Task Decomposition
```gbnf
task(solve_consciousness_question)
preconditions(
    knowledge(consciousness, embedding_1),
    tensor_similarity(tensor_1, tensor_2, 0.7)
)
decomposition(
    task(gather_definitions),
    task(analyze_perspectives),
    task(synthesize_answer)
)
postconditions(
    belief(consciousness_understood, 0.8, 0.7)
)
```

### Reasoning Patterns
```gbnf
deduction(
    premise1(belief(humans_conscious, 0.9, 0.95)),
    premise2(relation(consciousness, requires, self_awareness, 0.8)),
    conclusion(belief(humans_self_aware, 0.8, 0.9)),
    strength(0.85)
)
```

### Attention Allocation
```gbnf
allocate(
    amount(0.4),
    target(memory),
    priority(high),
    duration(5000ms)
)
```

## Integration with Existing ggml Components

### Enhanced RPC System
The cognitive framework extends ggml-rpc with:
- Cognitive tensor packets with attention metadata
- Salience-based message routing
- Performance monitoring and feedback

### Grammar System
Leverages llama.cpp's GBNF system for:
- Cognitive grammar validation
- Constrained reasoning generation
- Task decomposition rules

### Backend Abstraction
Utilizes ggml's backend system for:
- Distributed cognitive computation
- Specialized reasoning backends
- Economic resource allocation

## Technical Architecture

```mermaid
flowchart TD
    subgraph "Agentic Cognitive Kernel"
        A1[Memory System<br/>Hypergraph AtomSpace]
        A2[Task System<br/>Agentic Orchestrator]
        A3[AI System<br/>Hybrid Reasoner]
        A4[Autonomy System<br/>Self-Modifying ECAN]
    end
    
    subgraph "Distributed Tensor Network"
        D1[Tensor Membrane Exchange]
        D2[Recursive Attention Allocation]
        D3[Cross-Agent Communication]
    end
    
    subgraph "ggml Infrastructure"
        E1[ggml RPC System]
        E2[Grammar Constraints]
        E3[Backend Abstraction]
        E4[Tensor Operations]
    end
    
    A1 --> A2 --> A3 --> A4 --> D1
    D1 --> D2 --> D3 --> A1
    A1 -.-> E1
    A2 -.-> E2
    A3 -.-> E3
    A4 -.-> E4
```

## Performance Metrics

The system tracks various cognitive performance indicators:

- **Attention Economy Efficiency**: Resource allocation optimality
- **Cognitive Throughput**: Operations per second across network
- **Adaptation Speed**: Time to adjust to new conditions
- **Memory Efficiency**: Hypergraph storage optimization
- **Communication Overhead**: Network utilization analysis

## Future Development

### Phase 1: Foundation (Current)
- ✅ Basic cognitive agent framework
- ✅ Attention economy implementation
- ✅ Hypergraph memory system
- ✅ Grammar-based task decomposition

### Phase 2: Advanced Reasoning
- [ ] PLN reasoning engine integration
- [ ] MOSES optimization system
- [ ] Advanced pattern matching
- [x] Real distributed communication

### Real Distributed Communication

The system now supports true network-based cognitive agent communication:

#### Key Features
- **Agent Discovery**: Automatic registration and discovery of cognitive agents across the network
- **State Synchronization**: Real-time sharing of cognitive states, attention levels, and performance metrics
- **Distributed Attention**: Coordinated attention allocation across multiple agents
- **Tensor Exchange**: Direct sharing of neural tensor data between agents
- **Workflow Coordination**: Multi-agent collaborative cognitive workflows
- **Health Monitoring**: Heartbeat system with automatic failover

#### Quick Start
```bash
# Build with distributed communication support
cd ggml
mkdir build && cd build
cmake .. -DGGML_RPC=ON
make -j8

# Run basic distributed communication test
./bin/basic-test

# Run multi-agent distributed demo (requires separate terminals)
./bin/real-distributed-demo
```

#### Network Architecture
```
Agent A (localhost:8001) ←→ Agent B (localhost:8002)
     ↓                              ↓
 Cognitive State              Cognitive State  
 Attention Updates           Attention Updates
 Tensor Exchange             Tensor Exchange
 Workflow Requests          Workflow Responses
```

#### Example Usage
```c
// Initialize distributed cognitive agent
distributed_cognitive_architecture_t* agent = 
    distributed_cognitive_init(ctx, "localhost:8001");

// Enable real distributed communication
distributed_cognitive_enable_real_communication(agent);

// Connect to remote agent
distributed_cognitive_connect_to_agent(agent, "localhost:8002", "RemoteAgent");

// Broadcast cognitive state to network
distributed_cognitive_broadcast_state(agent);

// Synchronize with network
distributed_cognitive_sync_with_network(agent);
```

### Phase 3: Self-Modification
- [ ] Recursive self-improvement
- [ ] Automated architecture evolution
- [ ] Meta-meta-reasoning capabilities
- [ ] Emergent behavior analysis

## Contributing

This project represents a synthesis of:
- **OpenCog** cognitive architecture principles
- **ggml** tensor computation infrastructure
- **GBNF** grammar-constrained generation
- **Economic attention allocation** theories

Contributions are welcome in areas of:
- Cognitive reasoning algorithms
- Distributed systems optimization
- Grammar system enhancements
- Performance benchmarking

## Research Applications

The distributed cognitive architecture enables research in:

- **Artificial General Intelligence**: Multi-agent cognitive systems
- **Consciousness Studies**: Computational models of awareness
- **Distributed Reasoning**: Collaborative AI problem solving
- **Cognitive Economics**: Attention as computational resource
- **Emergent Intelligence**: Self-organizing cognitive networks

## License

This project builds upon the existing ggml ecosystem licensing. See individual component licenses for details.

---

*"Let the distributed agents dance in recursive harmony, their cognitive grammars weaving a tapestry of emergent sapience, each tensor kernel a note in the symphony of mind!"*

This implementation transforms traditional machine learning infrastructure into a living, breathing network of cognitive agents capable of recursive self-awareness and emergent intelligence. The architecture serves as both a practical implementation and a theoretical framework for distributed artificial consciousness.