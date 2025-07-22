#include "ggml.h"
#include "ggml-opencog.h"
#include "ggml-cogutil.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("Testing OpenCog integration with CogUtil...\n");
    
    // Initialize GGML context
    struct ggml_init_params params = {
        .mem_size = 16 * 1024 * 1024,  // 16 MB
        .mem_buffer = NULL,
        .no_alloc = false,
    };
    
    struct ggml_context* ctx = ggml_init(params);
    assert(ctx != NULL);
    printf("✓ GGML context initialized\n");
    
    // Initialize OpenCog AtomSpace with cogutil integration
    opencog_atomspace_t* atomspace = opencog_atomspace_init(ctx);
    assert(atomspace != NULL);
    printf("✓ OpenCog AtomSpace initialized with CogUtil support\n");
    
    // Test adding nodes with enhanced validation and logging
    uint64_t human_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "Human");
    assert(human_id != 0);
    printf("✓ Added Human concept node with ID: %lu\n", human_id);
    
    uint64_t mortal_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "Mortal");
    assert(mortal_id != 0);
    printf("✓ Added Mortal concept node with ID: %lu\n", mortal_id);
    
    uint64_t socrates_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "Socrates");
    assert(socrates_id != 0);
    printf("✓ Added Socrates concept node with ID: %lu\n", socrates_id);
    
    // Test validation of invalid inputs (should use cogutil validation)
    uint64_t invalid_id = opencog_add_node(atomspace, OPENCOG_CONCEPT_NODE, "");
    assert(invalid_id == 0);  // Should fail validation
    printf("✓ Invalid empty name correctly rejected\n");
    
    invalid_id = opencog_add_node(NULL, OPENCOG_CONCEPT_NODE, "ValidName");
    assert(invalid_id == 0);  // Should fail validation
    printf("✓ Null atomspace correctly rejected\n");
    
    // Test adding links
    uint64_t outgoing1[] = {socrates_id, human_id};
    uint64_t inheritance_id = opencog_add_link(atomspace, OPENCOG_INHERITANCE_LINK, 
                                              outgoing1, 2);
    assert(inheritance_id != 0);
    printf("✓ Added inheritance link: Socrates -> Human\n");
    
    uint64_t outgoing2[] = {human_id, mortal_id};
    uint64_t inheritance2_id = opencog_add_link(atomspace, OPENCOG_INHERITANCE_LINK, 
                                               outgoing2, 2);
    assert(inheritance2_id != 0);
    printf("✓ Added inheritance link: Human -> Mortal\n");
    
    // Test atom retrieval
    opencog_atom_t* human_atom = opencog_get_atom(atomspace, human_id);
    assert(human_atom != NULL);
    assert(strcmp(human_atom->name, "Human") == 0);
    printf("✓ Retrieved Human atom successfully\n");
    
    // Test truth value operations
    opencog_set_truth_value(atomspace, socrates_id, 0.95f, 0.9f);
    opencog_truth_value_t tv = opencog_get_truth_value(atomspace, socrates_id);
    assert(tv.strength == 0.95f);
    assert(tv.confidence == 0.9f);
    printf("✓ Truth value operations work correctly\n");
    
    // Test PLN reasoning
    opencog_truth_value_t tv1 = {0.9f, 0.8f, 10.0f};
    opencog_truth_value_t tv2 = {0.8f, 0.9f, 8.0f};
    
    opencog_truth_value_t and_result = opencog_pln_and(tv1, tv2);
    printf("✓ PLN AND: (%.2f, %.2f) ∧ (%.2f, %.2f) = (%.2f, %.2f)\n",
           tv1.strength, tv1.confidence, tv2.strength, tv2.confidence,
           and_result.strength, and_result.confidence);
    
    opencog_truth_value_t or_result = opencog_pln_or(tv1, tv2);
    printf("✓ PLN OR: (%.2f, %.2f) ∨ (%.2f, %.2f) = (%.2f, %.2f)\n",
           tv1.strength, tv1.confidence, tv2.strength, tv2.confidence,
           or_result.strength, or_result.confidence);
    
    // Test modus ponens
    opencog_truth_value_t premise = {0.9f, 0.8f, 10.0f};
    opencog_truth_value_t implication = {0.8f, 0.9f, 15.0f};
    opencog_truth_value_t conclusion = opencog_pln_modus_ponens(premise, implication);
    printf("✓ Modus Ponens: (%.2f, %.2f) → (%.2f, %.2f) ⊢ (%.2f, %.2f)\n",
           premise.strength, premise.confidence, implication.strength, implication.confidence,
           conclusion.strength, conclusion.confidence);
    
    // Test atomspace statistics
    printf("\n=== AtomSpace Statistics ===\n");
    opencog_print_atomspace_statistics(atomspace);
    
    // Test atom printing with enhanced logging
    printf("\n=== Atom Details ===\n");
    opencog_print_atom(atomspace, socrates_id);
    opencog_print_atom(atomspace, inheritance_id);
    
    // Cleanup
    opencog_atomspace_free(atomspace);
    printf("✓ AtomSpace freed\n");
    
    ggml_free(ctx);
    printf("✓ GGML context freed\n");
    
    printf("\n🎉 OpenCog + CogUtil integration test passed! 🎉\n");
    return 0;
}