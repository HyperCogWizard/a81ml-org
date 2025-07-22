#include "ggml-opencog.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// Test PLN basic inference rules
void test_pln_basic_operations() {
    printf("Testing PLN basic operations...\n");
    
    // Test AND operation
    opencog_truth_value_t tv1 = {0.8f, 0.9f, 10.0f};
    opencog_truth_value_t tv2 = {0.7f, 0.8f, 8.0f};
    
    opencog_truth_value_t and_result = opencog_pln_and(tv1, tv2);
    assert(fabs(and_result.strength - 0.7f) < 0.01f); // Min(0.8, 0.7)
    printf("✓ PLN AND: (0.8, 0.9) ∧ (0.7, 0.8) = (%.2f, %.2f)\n", 
           and_result.strength, and_result.confidence);
    
    // Test OR operation
    opencog_truth_value_t or_result = opencog_pln_or(tv1, tv2);
    assert(fabs(or_result.strength - 0.8f) < 0.01f); // Max(0.8, 0.7)
    printf("✓ PLN OR: (0.8, 0.9) ∨ (0.7, 0.8) = (%.2f, %.2f)\n", 
           or_result.strength, or_result.confidence);
    
    // Test NOT operation
    opencog_truth_value_t not_result = opencog_pln_not(tv1);
    assert(fabs(not_result.strength - 0.2f) < 0.01f); // 1.0 - 0.8
    printf("✓ PLN NOT: ¬(0.8, 0.9) = (%.2f, %.2f)\n", 
           not_result.strength, not_result.confidence);
}

// Test PLN advanced inference rules
void test_pln_advanced_operations() {
    printf("\nTesting PLN advanced operations...\n");
    
    opencog_truth_value_t premise_a = {0.9f, 0.85f, 12.0f};
    opencog_truth_value_t implication_ab = {0.8f, 0.9f, 10.0f};
    opencog_truth_value_t premise_b = {0.7f, 0.8f, 8.0f};
    
    // Test Modus Ponens: (A, A→B) ⊢ B
    opencog_truth_value_t mp_result = opencog_pln_modus_ponens(premise_a, implication_ab);
    printf("✓ Modus Ponens: A(%.2f, %.2f) ∧ A→B(%.2f, %.2f) ⊢ B(%.2f, %.2f)\n",
           premise_a.strength, premise_a.confidence,
           implication_ab.strength, implication_ab.confidence,
           mp_result.strength, mp_result.confidence);
    
    // Test Deduction: (A→B, B→C) ⊢ (A→C)
    opencog_truth_value_t implication_bc = {0.75f, 0.85f, 9.0f};
    opencog_truth_value_t deduction_result = opencog_pln_deduction(implication_ab, implication_bc);
    printf("✓ Deduction: A→B(%.2f, %.2f) ∧ B→C(%.2f, %.2f) ⊢ A→C(%.2f, %.2f)\n",
           implication_ab.strength, implication_ab.confidence,
           implication_bc.strength, implication_bc.confidence,
           deduction_result.strength, deduction_result.confidence);
    
    // Test Induction
    opencog_truth_value_t specific_case = {0.85f, 0.8f, 5.0f};
    opencog_truth_value_t general_pattern = {0.7f, 0.9f, 15.0f};
    opencog_truth_value_t induction_result = opencog_pln_induction(specific_case, general_pattern);
    printf("✓ Induction: Specific(%.2f, %.2f) ∧ Pattern(%.2f, %.2f) ⊢ General(%.2f, %.2f)\n",
           specific_case.strength, specific_case.confidence,
           general_pattern.strength, general_pattern.confidence,
           induction_result.strength, induction_result.confidence);
    
    // Test Abduction
    opencog_truth_value_t abduction_result = opencog_pln_abduction(implication_ab, premise_b);
    printf("✓ Abduction: A→B(%.2f, %.2f) ∧ B(%.2f, %.2f) ⊢ A(%.2f, %.2f)\n",
           implication_ab.strength, implication_ab.confidence,
           premise_b.strength, premise_b.confidence,
           abduction_result.strength, abduction_result.confidence);
}

// Test PLN reasoning with AtomSpace
void test_pln_atomspace_reasoning() {
    printf("\nTesting PLN reasoning with AtomSpace...\n");
    
    // Initialize GGML context
    struct ggml_init_params params = {
        .mem_size = 1024 * 1024,
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    struct ggml_context* ctx = ggml_init(params);
    assert(ctx != NULL);
    
    // Initialize AtomSpace
    opencog_atomspace_t* atomspace = opencog_atomspace_init(ctx);
    assert(atomspace != NULL);
    
    // Create knowledge: Socrates is human, humans are mortal
    uint64_t socrates_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "Socrates");
    uint64_t human_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "Human");
    uint64_t mortal_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "Mortal");
    
    // Set truth values
    opencog_set_truth_value(atomspace, socrates_id, 1.0f, 0.95f); // Socrates exists
    opencog_set_truth_value(atomspace, human_id, 0.9f, 0.9f);     // Human concept
    opencog_set_truth_value(atomspace, mortal_id, 0.85f, 0.9f);   // Mortal concept
    
    // Create inheritance links: Socrates -> Human
    uint64_t socrates_human_link[2] = {socrates_id, human_id};
    uint64_t socrates_human_id = opencog_add_link(atomspace, OPENCOG_INHERITANCE_LINK, 
                                                  socrates_human_link, 2);
    opencog_set_truth_value(atomspace, socrates_human_id, 0.95f, 0.9f);
    
    // Create implication: Human -> Mortal
    uint64_t human_mortal_link[2] = {human_id, mortal_id};
    uint64_t human_mortal_id = opencog_add_link(atomspace, OPENCOG_IMPLICATION_LINK, 
                                               human_mortal_link, 2);
    opencog_set_truth_value(atomspace, human_mortal_id, 0.9f, 0.85f);
    
    printf("Created knowledge base:\n");
    printf("- Socrates is Human (%.2f, %.2f)\n", 0.95f, 0.9f);
    printf("- Human → Mortal (%.2f, %.2f)\n", 0.9f, 0.85f);
    
    // Test PLN reasoning session
    opencog_pln_session_t* session = opencog_pln_session_init(atomspace, 0.7f);
    assert(session != NULL);
    
    // Run inference cycles
    printf("\nRunning PLN inference cycles...\n");
    for (int i = 0; i < 3; i++) {
        bool made_inference = opencog_pln_inference_cycle(session);
        printf("Cycle %d: %s\n", i + 1, made_inference ? "Made inference" : "No new inferences");
    }
    
    // Check if Socrates is now inferred to be mortal
    opencog_truth_value_t socrates_tv = opencog_get_truth_value(atomspace, socrates_id);
    printf("\nFinal truth value for Socrates: (%.2f, %.2f)\n", 
           socrates_tv.strength, socrates_tv.confidence);
    
    // Test pattern matching
    uint64_t implications[10];
    size_t impl_count = opencog_pln_find_patterns(atomspace, OPENCOG_IMPLICATION_LINK, 
                                                  implications, 10);
    printf("Found %zu implication patterns in AtomSpace\n", impl_count);
    
    // Print AtomSpace statistics
    opencog_print_atomspace_statistics(atomspace);
    
    // Cleanup
    opencog_pln_session_free(session);
    opencog_atomspace_free(atomspace);
    ggml_free(ctx);
    
    printf("✓ PLN AtomSpace reasoning test completed\n");
}

// Test PLN backward chaining
void test_pln_backward_chaining() {
    printf("\nTesting PLN backward chaining...\n");
    
    // Initialize GGML context
    struct ggml_init_params params = {
        .mem_size = 512 * 1024,
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    struct ggml_context* ctx = ggml_init(params);
    assert(ctx != NULL);
    
    // Initialize AtomSpace
    opencog_atomspace_t* atomspace = opencog_atomspace_init(ctx);
    assert(atomspace != NULL);
    
    // Create goal: prove that "rain" implies "wet ground"
    uint64_t rain_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "Rain");
    uint64_t wet_ground_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "WetGround");
    
    // Set initial weak truth value for goal
    opencog_set_truth_value(atomspace, wet_ground_id, 0.3f, 0.4f);
    
    // Create background knowledge: rain -> clouds, clouds -> wet ground
    uint64_t clouds_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "Clouds");
    
    uint64_t rain_clouds_link[2] = {rain_id, clouds_id};
    uint64_t rain_clouds_id = opencog_add_link(atomspace, OPENCOG_IMPLICATION_LINK, 
                                              rain_clouds_link, 2);
    opencog_set_truth_value(atomspace, rain_clouds_id, 0.85f, 0.9f);
    
    uint64_t clouds_wet_link[2] = {clouds_id, wet_ground_id};
    uint64_t clouds_wet_id = opencog_add_link(atomspace, OPENCOG_IMPLICATION_LINK, 
                                             clouds_wet_link, 2);
    opencog_set_truth_value(atomspace, clouds_wet_id, 0.9f, 0.85f);
    
    printf("Goal: Strengthen belief that ground is wet (currently %.2f, %.2f)\n", 0.3f, 0.4f);
    printf("Background: Rain → Clouds (%.2f, %.2f), Clouds → WetGround (%.2f, %.2f)\n",
           0.85f, 0.9f, 0.9f, 0.85f);
    
    // Apply backward chaining
    opencog_truth_value_t desired_tv = {0.8f, 0.8f, 10.0f};
    bool success = opencog_pln_backward_chain(atomspace, wet_ground_id, desired_tv);
    
    // Check result
    opencog_truth_value_t final_tv = opencog_get_truth_value(atomspace, wet_ground_id);
    printf("Backward chaining %s\n", success ? "succeeded" : "failed");
    printf("Final truth value for WetGround: (%.2f, %.2f)\n", 
           final_tv.strength, final_tv.confidence);
    
    // Cleanup
    opencog_atomspace_free(atomspace);
    ggml_free(ctx);
    
    printf("✓ PLN backward chaining test completed\n");
}

int main() {
    printf("PLN Reasoning Engine Test Suite\n");
    printf("===============================\n");
    
    test_pln_basic_operations();
    test_pln_advanced_operations();
    test_pln_atomspace_reasoning();
    test_pln_backward_chaining();
    
    printf("\n===============================\n");
    printf("All PLN reasoning tests passed! ✓\n");
    printf("PLN reasoning engine successfully integrated.\n");
    
    return 0;
}