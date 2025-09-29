# Cognitive Grammar Examples

This document provides practical examples of cognitive grammars that can be used with the distributed agent system, building on the existing GBNF grammar system in llama.cpp.

## Basic Cognitive Grammar Rules

### 1. Task Decomposition Grammar

```gbnf
# Cognitive task decomposition grammar
root ::= cognitive-task

cognitive-task ::= 
    "task(" task-name ")" ws
    "preconditions(" precondition-list ")" ws
    "decomposition(" subtask-list ")" ws
    "postconditions(" postcondition-list ")"

task-name ::= [a-zA-Z_][a-zA-Z0-9_]*

precondition-list ::= precondition (ws "," ws precondition)*
precondition ::= logical-statement | tensor-condition

subtask-list ::= subtask (ws "," ws subtask)*
subtask ::= cognitive-task | primitive-action

postcondition-list ::= postcondition (ws "," ws postcondition)*
postcondition ::= logical-statement | tensor-condition

logical-statement ::= 
    "belief(" concept "," truth-value "," confidence-value ")" |
    "knowledge(" concept "," embedding-ref ")" |
    "goal(" objective "," priority-value ")"

tensor-condition ::= 
    "tensor_similarity(" tensor-ref "," tensor-ref "," threshold ")" |
    "tensor_norm(" tensor-ref "," norm-type "," threshold ")"

primitive-action ::= 
    "send_message(" agent-id "," message-content ")" |
    "update_memory(" concept "," new-value ")" |
    "allocate_attention(" amount "," cognitive-function ")"

concept ::= [a-zA-Z][a-zA-Z0-9_]*
truth-value ::= [0-9]+ "." [0-9]+
confidence-value ::= [0-9]+ "." [0-9]+
priority-value ::= [0-9]+ "." [0-9]+
threshold ::= [0-9]+ "." [0-9]+
agent-id ::= [0-9]+
amount ::= [0-9]+ "." [0-9]+
embedding-ref ::= "embedding_" [0-9]+
tensor-ref ::= "tensor_" [0-9]+
norm-type ::= "l1" | "l2" | "inf"
cognitive-function ::= "memory" | "reasoning" | "communication" | "attention"
objective ::= [a-zA-Z][a-zA-Z0-9_\s]*
message-content ::= "\"" [^"]* "\""

ws ::= [ \t\n]*
```

### 2. Reasoning Pattern Grammar

```gbnf
# PLN-style reasoning grammar
root ::= reasoning-pattern

reasoning-pattern ::= 
    deduction-rule |
    induction-rule |
    abduction-rule |
    analogy-rule

deduction-rule ::= 
    "deduction(" ws
        "premise1(" logical-statement ")" ws
        "premise2(" logical-statement ")" ws
        "conclusion(" logical-statement ")" ws
        "strength(" strength-value ")" ws
    ")"

induction-rule ::= 
    "induction(" ws
        "observations(" observation-list ")" ws
        "pattern(" pattern-description ")" ws
        "generalization(" logical-statement ")" ws
        "confidence(" confidence-value ")" ws
    ")"

abduction-rule ::= 
    "abduction(" ws
        "observation(" logical-statement ")" ws
        "hypothesis(" logical-statement ")" ws
        "plausibility(" plausibility-value ")" ws
    ")"

analogy-rule ::= 
    "analogy(" ws
        "source_domain(" domain-description ")" ws
        "target_domain(" domain-description ")" ws
        "mapping(" mapping-list ")" ws
        "inference(" logical-statement ")" ws
    ")"

observation-list ::= logical-statement (ws "," ws logical-statement)*
pattern-description ::= "\"" [^"]* "\""
domain-description ::= "\"" [^"]* "\""
mapping-list ::= mapping (ws "," ws mapping)*
mapping ::= concept "->" concept

strength-value ::= [0-9]+ "." [0-9]+
plausibility-value ::= [0-9]+ "." [0-9]+

logical-statement ::= 
    "belief(" concept "," truth-value "," confidence-value ")" |
    "relation(" concept "," relation-type "," concept "," strength-value ")" |
    "property(" concept "," property-name "," property-value ")"

relation-type ::= "is_a" | "part_of" | "causes" | "similar_to" | "implies"
property-name ::= [a-zA-Z][a-zA-Z0-9_]*
property-value ::= [a-zA-Z0-9_]+ | [0-9]+ "." [0-9]+
```

### 3. Attention Allocation Grammar

```gbnf
# Attention economy grammar
root ::= attention-command

attention-command ::= 
    allocate-attention |
    reallocate-attention |
    attention-query

allocate-attention ::= 
    "allocate(" ws
        "amount(" attention-amount ")" ws
        "target(" attention-target ")" ws
        "priority(" priority-level ")" ws
        "duration(" time-duration ")" ws
    ")"

reallocate-attention ::= 
    "reallocate(" ws
        "from(" attention-source ")" ws
        "to(" attention-target ")" ws
        "amount(" attention-amount ")" ws
        "reason(" reallocation-reason ")" ws
    ")"

attention-query ::= 
    "query_attention(" ws
        "target(" attention-target ")" ws
        "metric(" attention-metric ")" ws
    ")"

attention-target ::= 
    cognitive-function |
    specific-task |
    agent-communication

attention-source ::= attention-target

cognitive-function ::= "memory" | "reasoning" | "planning" | "communication"

specific-task ::= 
    "task(" task-identifier ")" |
    "goal(" goal-identifier ")"

agent-communication ::= 
    "agent(" agent-id ")" |
    "message_type(" message-type ")"

attention-amount ::= [0-9]+ "." [0-9]+
priority-level ::= "critical" | "high" | "medium" | "low"
time-duration ::= [0-9]+ time-unit
time-unit ::= "ms" | "s" | "min" | "hour"
reallocation-reason ::= "performance_drop" | "new_priority" | "resource_constraint"
attention-metric ::= "current_allocation" | "utilization" | "efficiency"
task-identifier ::= [a-zA-Z_][a-zA-Z0-9_]*
goal-identifier ::= [a-zA-Z_][a-zA-Z0-9_]*
message-type ::= "query" | "response" | "broadcast" | "update"
```

## Practical Examples

### Example 1: Complex Problem Solving

```
task(solve_consciousness_question)
preconditions(
    knowledge(consciousness, embedding_1),
    knowledge(neural_networks, embedding_2),
    tensor_similarity(tensor_1, tensor_2, 0.7)
)
decomposition(
    task(gather_definitions),
    task(analyze_perspectives),
    task(synthesize_answer)
)
postconditions(
    belief(consciousness_understood, 0.8, 0.7),
    goal(share_knowledge, 0.9)
)
```

### Example 2: Collaborative Reasoning

```
task(collaborative_reasoning)
preconditions(
    belief(problem_complex, 0.9, 0.8),
    knowledge(other_agents, embedding_3)
)
decomposition(
    send_message(agent_2, "Request expertise on consciousness"),
    task(local_analysis),
    task(integrate_responses),
    update_memory(collaboration_history, new_entry)
)
postconditions(
    belief(solution_quality_improved, 0.85, 0.9)
)
```

### Example 3: Attention Management

```
allocate(
    amount(0.4),
    target(memory),
    priority(high),
    duration(5000ms)
)

reallocate(
    from(task(routine_processing)),
    to(agent(urgent_communication)),
    amount(0.2),
    reason(new_priority)
)
```

### Example 4: Reasoning Chain

```
deduction(
    premise1(belief(humans_conscious, 0.9, 0.95)),
    premise2(relation(consciousness, requires, self_awareness, 0.8)),
    conclusion(belief(humans_self_aware, 0.8, 0.9)),
    strength(0.85)
)

analogy(
    source_domain("human consciousness"),
    target_domain("artificial intelligence"),
    mapping(
        neural_activity -> computational_process,
        self_awareness -> recursive_self_modeling,
        consciousness -> emergent_intelligence
    ),
    inference(belief(ai_can_be_conscious, 0.6, 0.7))
)
```

## Integration with Code

### Grammar Parser Integration

```c++
#include "llama-grammar.h"

// Cognitive grammar parser
struct cognitive_grammar_parser {
    struct llama_grammar* grammar;
    char* grammar_rules;
    
    // Parsing state
    cognitive_parse_state state;
    parsed_cognitive_command* current_command;
};

// Parse cognitive grammar command
parsed_cognitive_command* parse_cognitive_command(
    cognitive_grammar_parser* parser, 
    const char* input) {
    
    // Use llama.cpp grammar system
    struct llama_grammar* grammar = llama_grammar_init_impl(
        parser->grammar_rules, "root");
    
    // Parse input according to cognitive grammar
    bool parse_success = llama_grammar_accept(grammar, input, strlen(input));
    
    if (!parse_success) {
        return NULL;
    }
    
    // Extract structured command
    parsed_cognitive_command* cmd = extract_command_structure(parser, input);
    
    llama_grammar_free_impl(grammar);
    return cmd;
}

// Execute parsed cognitive command
void execute_cognitive_command(cognitive_agent* agent, 
                             parsed_cognitive_command* cmd) {
    switch (cmd->type) {
        case COGNITIVE_CMD_TASK:
            execute_task_command(agent, &cmd->task);
            break;
            
        case COGNITIVE_CMD_REASONING:
            execute_reasoning_command(agent, &cmd->reasoning);
            break;
            
        case COGNITIVE_CMD_ATTENTION:
            execute_attention_command(agent, &cmd->attention);
            break;
    }
}
```

### Task Execution Engine

```c++
// Execute task decomposition
void execute_task_command(cognitive_agent* agent, parsed_task_command* task) {
    // Check preconditions
    if (!check_preconditions(agent, task->preconditions)) {
        defer_task(agent, task);
        return;
    }
    
    // Execute subtasks
    for (size_t i = 0; i < task->subtask_count; i++) {
        subtask* sub = &task->subtasks[i];
        
        if (sub->type == SUBTASK_PRIMITIVE) {
            execute_primitive_action(agent, &sub->action);
        } else if (sub->type == SUBTASK_COMPLEX) {
            // Recursive task execution
            execute_task_command(agent, &sub->task);
        }
    }
    
    // Verify postconditions
    verify_postconditions(agent, task->postconditions);
}

// Execute primitive actions
void execute_primitive_action(cognitive_agent* agent, primitive_action* action) {
    switch (action->type) {
        case ACTION_SEND_MESSAGE:
            send_cognitive_tensor(agent, action->target_agent, 
                                action->message_tensor, action->attention_weight);
            break;
            
        case ACTION_UPDATE_MEMORY:
            update_hypergraph_memory(agent->memory, action->concept, 
                                   action->new_value);
            break;
            
        case ACTION_ALLOCATE_ATTENTION:
            allocate_attention(agent->attention, action->amount, 
                             action->target_function);
            break;
    }
}
```

### Grammar-Guided Generation

```c++
// Generate cognitive response using grammar constraints
char* generate_cognitive_response(cognitive_agent* agent, 
                                 cognitive_rpc_message* input_msg) {
    // Determine appropriate response grammar
    const char* response_grammar = select_response_grammar(input_msg);
    
    // Use constrained generation (similar to llama.cpp grammar)
    struct llama_grammar* grammar = llama_grammar_init_impl(
        response_grammar, "root");
    
    // Generate response with grammar constraints
    char* response = generate_with_grammar_constraints(
        agent->reasoning->language_model, 
        input_msg->context,
        grammar
    );
    
    llama_grammar_free_impl(grammar);
    return response;
}
```

## Testing Cognitive Grammars

### Grammar Validation

```c++
void test_cognitive_grammars() {
    // Test task decomposition grammar
    const char* valid_task = 
        "task(solve_problem) "
        "preconditions(knowledge(math, embedding_1)) "
        "decomposition(task(analyze), task(synthesize)) "
        "postconditions(belief(solved, 0.9, 0.8))";
    
    cognitive_grammar_parser* parser = init_grammar_parser();
    parsed_cognitive_command* cmd = parse_cognitive_command(parser, valid_task);
    
    assert(cmd != NULL);
    assert(cmd->type == COGNITIVE_CMD_TASK);
    assert(cmd->task.subtask_count == 2);
    
    cleanup_grammar_parser(parser);
    
    // Test reasoning grammar
    const char* valid_reasoning = 
        "deduction("
        "premise1(belief(humans_mortal, 0.99, 0.95)) "
        "premise2(belief(socrates_human, 0.95, 0.9)) "
        "conclusion(belief(socrates_mortal, 0.94, 0.85)) "
        "strength(0.89)"
        ")";
    
    cmd = parse_cognitive_command(parser, valid_reasoning);
    assert(cmd != NULL);
    assert(cmd->type == COGNITIVE_CMD_REASONING);
}
```

### Integration Test

```c++
void test_grammar_execution() {
    cognitive_agent* agent = create_cognitive_agent("localhost:8000");
    
    // Parse and execute a complete cognitive task
    const char* complex_task = 
        "task(collaborative_analysis) "
        "preconditions(knowledge(domain_expertise, embedding_5)) "
        "decomposition("
            "send_message(agent_2, \"Request collaboration\"), "
            "allocate_attention(0.6, reasoning), "
            "task(local_processing)"
        ") "
        "postconditions(belief(analysis_complete, 0.8, 0.9))";
    
    cognitive_grammar_parser* parser = init_grammar_parser();
    parsed_cognitive_command* cmd = parse_cognitive_command(parser, complex_task);
    
    // Execute the command
    execute_cognitive_command(agent, cmd);
    
    // Verify execution results
    assert(agent->attention->reasoning_allocation >= 0.6);
    // Additional assertions...
    
    cleanup_cognitive_agent(agent);
    cleanup_grammar_parser(parser);
}
```

## Meta-Meta-Reasoning Grammar

The meta-meta-reasoning grammar extends the cognitive grammar system to support reasoning about reasoning processes themselves.

### 4. Meta-Meta-Reasoning Grammar

```gbnf
# Meta-meta-reasoning grammar for self-reflective cognitive processes
root ::= meta-meta-reasoning

meta-meta-reasoning ::= 
    meta-reasoning-loop |
    meta-pattern-analysis |
    meta-reflection |
    meta-evolution

meta-reasoning-loop ::=
    "meta_meta_loop(" ws
        "description(" string-literal ")" ws
        "reasoning_patterns(" pattern-list ")" ws
        "self_awareness(" awareness-level ")" ws
        "coherence_target(" coherence-threshold ")" ws
    ")"

meta-pattern-analysis ::=
    "analyze_reasoning_patterns(" ws
        "patterns(" pattern-list ")" ws
        "effectiveness_threshold(" threshold ")" ws
        "evolution_rate(" rate-value ")" ws
        "temporal_consistency(" consistency-value ")" ws
    ")"

meta-reflection ::=
    "self_reflect(" ws
        "reasoning_process(" reasoning-type ")" ws
        "awareness_level(" awareness-level ")" ws
        "confidence(" confidence-value ")" ws
        "coherence_check(" boolean-value ")" ws
    ")"

meta-evolution ::=
    "evolve_reasoning(" ws
        "target_patterns(" pattern-list ")" ws
        "evolution_strategy(" strategy-type ")" ws
        "learning_rate(" rate-value ")" ws
        "success_threshold(" threshold ")" ws
    ")"

pattern-list ::= reasoning-pattern (ws "," ws reasoning-pattern)*

reasoning-pattern ::=
    "pattern(" ws
        "type(" reasoning-type ")" ws
        "description(" string-literal ")" ws
        "strength(" strength-value ")" ws
        "confidence(" confidence-value ")" ws
        "success_rate(" rate-value ")" ws
    ")"

reasoning-type ::= 
    "deduction" | 
    "induction" | 
    "abduction" | 
    "analogy" | 
    "reflection"

strategy-type ::=
    "gradient_ascent" |
    "pattern_mutation" |
    "coherence_optimization" |
    "self_modification"

awareness-level ::= [0-9]+ "." [0-9]+
coherence-threshold ::= [0-9]+ "." [0-9]+
consistency-value ::= [0-9]+ "." [0-9]+
strength-value ::= [0-9]+ "." [0-9]+
rate-value ::= [0-9]+ "." [0-9]+
boolean-value ::= "true" | "false"
string-literal ::= "\"" [^"]* "\""

ws ::= [ \t\n\r]*
```

### Example 5: Meta-Meta-Reasoning Chain

```
meta_meta_loop(
    description("Self-reflective reasoning optimizer"),
    reasoning_patterns(
        pattern(
            type(reflection),
            description("Self-awareness analysis"),
            strength(0.8),
            confidence(0.7),
            success_rate(0.85)
        ),
        pattern(
            type(analogy),
            description("Cross-domain reasoning transfer"),
            strength(0.6),
            confidence(0.65),
            success_rate(0.72)
        )
    ),
    self_awareness(0.75),
    coherence_target(0.8)
)

analyze_reasoning_patterns(
    patterns(
        pattern(type(deduction), description("Logical inference"), strength(0.9), confidence(0.95), success_rate(0.88)),
        pattern(type(induction), description("Pattern generalization"), strength(0.7), confidence(0.8), success_rate(0.75))
    ),
    effectiveness_threshold(0.7),
    evolution_rate(0.1),
    temporal_consistency(0.85)
)

self_reflect(
    reasoning_process(reflection),
    awareness_level(0.8),
    confidence(0.75),
    coherence_check(true)
)

evolve_reasoning(
    target_patterns(
        pattern(type(abduction), description("Hypothesis generation"), strength(0.5), confidence(0.6), success_rate(0.65)
    ),
    evolution_strategy(coherence_optimization),
    learning_rate(0.05),
    success_threshold(0.8)
)
```

This cognitive grammar system provides a structured way to express and execute complex cognitive behaviors while leveraging the existing GBNF infrastructure from llama.cpp. The grammars ensure that cognitive operations are syntactically valid and semantically meaningful within the distributed agent network.

The meta-meta-reasoning grammar extension enables the system to reason about its own reasoning processes, creating recursive loops of self-improvement and self-awareness that can continuously optimize cognitive performance.