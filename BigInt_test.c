#include <assert.h>
#include <stdio.h>

#include "BigInt.h"

const char* OPERATION_NAMES[] = {"Addition", "Addition with int",
        "Subtraction", "Subtraction with int", "Multiplication",
        "Multiplication with int", "Comparison"};

void BigInt_test_basic() {

    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing construction\n");
    }
    BigInt_test_construct(0);
    BigInt_test_construct(1);
    BigInt_test_construct(-1);
    BigInt_test_construct(2);
    BigInt_test_construct(10);
    BigInt_test_construct(100);
    BigInt_test_construct(1000000000);
    BigInt_test_construct(1000000001);
    BigInt_test_construct(990000000);

    // Ensure that reallocating digits doesn't make us
    // lose data.
    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing digit reallocation\n");
    }
    BigInt* big_int = BigInt_construct(42);
    assert(BigInt_to_int(big_int) == 42);
    BigInt_ensure_digits(big_int, 1000);
    assert(BigInt_to_int(big_int) == 42);
    BigInt_ensure_digits(big_int, 1);
    assert(BigInt_to_int(big_int) == 42);
    BigInt_free(big_int);

    // Test addition, subtraction, and comparison for all positive and
    // negative permutations of these integers
    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing basic 2-argument operations\n");
    }
    BigInt_test_operations(0, 0);
    BigInt_test_operations(1, 1);
    BigInt_test_operations(5, 5);
    BigInt_test_operations(5, 6);
    BigInt_test_operations(10, 2);
    BigInt_test_operations(14, 16);
    BigInt_test_operations(16, 18);
    BigInt_test_operations(11, 111);
    BigInt_test_operations(50, 50);
    BigInt_test_operations(51, 50);
    BigInt_test_operations(64, 46);
    BigInt_test_operations(1000, 999);
    BigInt_test_operations(30, 28);
    BigInt_test_operations(1, 50);
    BigInt_test_operations(100, 101);
    BigInt_test_operations(1000, 999);
    BigInt_test_operations(123456, 1234);
    BigInt_test_operations(999999999, 1);
    BigInt_test_operations(0, 12345678);
    BigInt_test_operations(1000, 1);
    BigInt_test_operations(2546, 2546);
    BigInt_test_operations(1234, 4321);
}

void BigInt_test_construct(int value) {
    BigInt* big_int = BigInt_construct(value);
    assert(BigInt_to_int(big_int) == value);
    BigInt_free(big_int);
}

void BigInt_test_operations(int a, int b) {

    OPERATION_TYPE operation_type;

    for(operation_type = 0; operation_type < OPERATION_TYPE_MAX; operation_type++) {
        switch(operation_type) {
            case ADD:
                BigInt_test_permutations((Generic_function)BigInt_add, operation_type, a, b); 
                break;
            case ADD_INT:
                BigInt_test_permutations((Generic_function)BigInt_add_int, operation_type, a, b); 
                break;
            case SUBTRACT:
                BigInt_test_permutations((Generic_function)BigInt_subtract, operation_type, a, b); 
                break;
            case SUBTRACT_INT:
                BigInt_test_permutations((Generic_function)BigInt_subtract_int, operation_type, a, b); 
                break;
            case MULTIPLY:
                BigInt_test_permutations((Generic_function)BigInt_multiply, operation_type, a, b); 
                break;
            case MULTIPLY_INT:
                BigInt_test_permutations((Generic_function)BigInt_multiply_int, operation_type, a, b); 
                break;
            case COMPARE:
                BigInt_test_permutations((Generic_function)BigInt_compare, operation_type, a, b); 
                break;
            default:
                printf("Unsupported operation: %i\n", operation_type);
                assert(0);
        }
    }
}

// Calls the specified BigInt 2-operand function for all 
// permutations of positive, negative, and order-reversals
// of the values a and b.
void BigInt_test_permutations(Generic_function BigInt_operation_to_test,
        OPERATION_TYPE operation_type, int a, int b) {

    BigInt_test_single_operation(BigInt_operation_to_test, operation_type, a, b);
    BigInt_test_single_operation(BigInt_operation_to_test, operation_type, -a, b);
    BigInt_test_single_operation(BigInt_operation_to_test, operation_type, a, -b);
    BigInt_test_single_operation(BigInt_operation_to_test, operation_type, -a, -b);
    BigInt_test_single_operation(BigInt_operation_to_test, operation_type, b, a);
    BigInt_test_single_operation(BigInt_operation_to_test, operation_type, -b, a);
    BigInt_test_single_operation(BigInt_operation_to_test, operation_type, b, -a);
    BigInt_test_single_operation(BigInt_operation_to_test, operation_type, -b, -a);
}

void BigInt_test_single_operation(Generic_function BigInt_operation_to_test,
        OPERATION_TYPE operation_type, int a, int b) {
    
    if(BIGINT_TEST_LOGGING > 1) {
        printf("Testing %s for %i, %i\n", OPERATION_NAMES[operation_type], a, b);
    }

    BigInt* big_int_a = BigInt_construct(a);
    BigInt* big_int_b = BigInt_construct(b);

    int result; 

    switch(operation_type) {
        case ADD:
        case SUBTRACT:
        case MULTIPLY:
            ((void(*)(BigInt*, const BigInt*))(*BigInt_operation_to_test))(big_int_a, big_int_b);
            result = BigInt_to_int(big_int_a);
            break;
        case ADD_INT:
        case SUBTRACT_INT:
        case MULTIPLY_INT:
            ((void(*)(BigInt*, const int))(*BigInt_operation_to_test))(big_int_a, b);
            result = BigInt_to_int(big_int_a);
            break;
        case COMPARE:
            result = ((int(*)(const BigInt*, const BigInt*))(*BigInt_operation_to_test))(big_int_a, big_int_b);
            break;
        default:
            printf("Unsupported operation\n");
            assert(0);
    }
 
    if(BIGINT_TEST_LOGGING > 1) {
        printf("%s result: %i\n", OPERATION_NAMES[operation_type], result);
    }

    switch(operation_type) {
        case ADD:
        case ADD_INT:
            assert(result == a + b);
            break;
        case SUBTRACT:
        case SUBTRACT_INT:
            assert(result == a - b);
            break;
        case MULTIPLY:
        case MULTIPLY_INT:
            assert(result == a * b);
            break;
        case COMPARE:
            assert((a > b) ? (result == 1) : (a < b) ? (result == -1) : (result == 0));
            break;
        default:
            printf("Unsupported operation\n");
            assert(0);
    }

    BigInt_free(big_int_a);
    BigInt_free(big_int_b);
}

