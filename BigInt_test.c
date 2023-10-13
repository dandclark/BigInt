#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strerror

#include "BigInt.h"
#include "BigInt_test.h"

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

    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing strings\n");
    }
    BigInt_test_strings();

    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing division\n");
    }
    BigInt_test_division();

    // Ensure that reallocating digits doesn't make us
    // lose data.
    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing digit reallocation\n");
    }
    BigInt* big_int = BigInt_construct(42);
    assert(big_int);
    int value;
    assert(BigInt_to_int(big_int, &value) && value == 42);
    assert(BigInt_ensure_digits(big_int, 1000));
    assert(BigInt_to_int(big_int, &value) && value == 42);
    assert(BigInt_ensure_digits(big_int, 1));
    assert(BigInt_to_int(big_int, &value) && value == 42);
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
    //BigInt_test_operations(999999999, 123456789);
    
    BigInt_test_signs();
    BigInt_test_multiply_optimized();
}

// This is basically a stress-test for multiplication.
// Try to root out any allocation issues by building up
// to multiplications of very large numbers, which
// will likely crash if we're not allocating enough space.
void BigInt_test_big_multiplication() {
    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing big multiplications\n");
    }

    BigInt* a = BigInt_construct(99999);
    assert(a);
    BigInt* b = BigInt_construct(12345);
    assert(b);
    for(int i = 0; i < 50; ++i) {
        assert(BigInt_multiply(a, b));
    }
    for(int i = 0; i < 50; ++i) {
        assert(BigInt_multiply(b, a));
    }
    BigInt_free(a);
    BigInt_free(b);
}

void BigInt_test_construct(int value) {
    BigInt* big_int = BigInt_construct(value);
    int value2;
    assert(BigInt_to_int(big_int, &value2) && value2 == value);
    BigInt_free(big_int);
}


void BigInt_test_signs() {
    BigInt* a = BigInt_construct(0);
    assert(a);
    BigInt* b = BigInt_construct(0);
    assert(b);
    assert(BigInt_subtract(a, b));
    char* s = BigInt_to_new_string(a);
    assert(s);
    //printf("%s\n", s);
    assert(!strcmp(s, "0"));
    free(s);
    BigInt_free(a);
    BigInt_free(b);
}

void BigInt_test_multiply_optimized() {
    // found a bug in BigInt_multiply where it was outputting too many 0's
    BigInt* a = BigInt_construct(0);
    assert(a);
    BigInt* b = BigInt_construct(10);
    assert(b);
    assert(BigInt_multiply(a, b));
    char* s = BigInt_to_new_string(a);
    assert(s);
    //printf("%s\n", s);
    assert(!strcmp(s, "0"));
    free(s);
    BigInt_free(a);
    BigInt_free(b);
}

void BigInt_test_strings() {
    int value;

    // test really big numbers that won't fit in an int:
    BigInt* big_int = BigInt_from_string("9876543210123456789");
    assert(big_int);
    assert(BigInt_multiply_int(big_int, -10));
    assert(!BigInt_to_int(big_int, &value));
    assert(errno == ERANGE); // value too big to fit into int
    char* str = BigInt_to_new_string(big_int);
    assert(str);
    assert(!strcmp(str, "-98765432101234567890"));
    free(str);
    BigInt_free(big_int);

    big_int = BigInt_from_string("-9876543210123456789");
    assert(big_int);
    assert(BigInt_multiply_int(big_int, -10));
    assert(!BigInt_to_int(big_int, &value));
    assert(errno == ERANGE); // value too big to fit into int
    str = BigInt_to_new_string(big_int);
    assert(str);
    assert(!strcmp(str, "98765432101234567890"));
    free(str);
    BigInt_free(big_int);
    
    big_int = BigInt_from_string("0");
    assert(big_int);
    assert(BigInt_to_int(big_int, &value));
    assert(value == 0);
    BigInt_free(big_int);
    
    big_int = BigInt_from_string("-0");
    assert(big_int);
    assert(BigInt_to_int(big_int, &value));
    assert(value == 0);
    BigInt_free(big_int);
    
    big_int = BigInt_from_string("0000");
    assert(big_int);
    assert(BigInt_to_int(big_int, &value));
    assert(value == 0);
    BigInt_free(big_int);
}

void _BigInt_test_division( const char* dividend, const char* divisor, const char* quotient, const char* remainder ) {
	BigInt* _dividend = BigInt_from_string(dividend);
	assert(_dividend);
	BigInt* _divisor = BigInt_from_string(divisor);
	assert(_divisor);
	BigInt* _quotient = BigInt_construct(0);
	BigInt* _remainder = BigInt_construct(0);
	assert(BigInt_divide(_dividend, _divisor, _quotient, _remainder));
	char* quotient2 = BigInt_to_new_string(_quotient);
	assert(quotient2);
	char* remainder2 = BigInt_to_new_string(_remainder);
	assert(remainder2);
	if(strcmp(quotient, quotient2)) {
		printf("BigInt_test_division failed: expecting quotient=%s but got %s\n", quotient, quotient2);
		assert(0);
	}
	assert(!strcmp(remainder, remainder2));
	BigInt_free(_dividend);
	BigInt_free(_divisor);
	BigInt_free(_quotient);
	BigInt_free(_remainder);
	free(quotient2);
	free(remainder2);
}

void BigInt_test_division() {
	// basic division:
	_BigInt_test_division( "1000", "10", "100", "0" );
	
	// too big to fit into 32-bit int:
	_BigInt_test_division( "963096309630", "30", "32103210321", "0" );
	_BigInt_test_division( "300000000000000000000", "3000000000000000", "100000", "0" );
	
	// test remainder:
	_BigInt_test_division( "10", "3", "3", "1" );
}

void BigInt_test_operations(int a, int b) {

    OPERATION_TYPE operation_type;

    for(operation_type = 0; operation_type < OPERATION_TYPE_COUNT; operation_type++) {
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
    assert(big_int_a);
    assert(big_int_b);

    int result;

    switch(operation_type) {
        case ADD:
        case SUBTRACT:
        case MULTIPLY:
            assert(((BOOL(*)(BigInt*, const BigInt*))(*BigInt_operation_to_test))(big_int_a, big_int_b));
            assert(BigInt_to_int(big_int_a, &result));
            break;
        case ADD_INT:
        case SUBTRACT_INT:
        case MULTIPLY_INT:
            assert(((BOOL(*)(BigInt*, const int))(*BigInt_operation_to_test))(big_int_a, b));
            assert(BigInt_to_int(big_int_a, &result));
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

void BigInt_test_print() {
    BigInt* big_int = BigInt_construct(10000);
    assert(big_int);
    assert(BigInt_subtract_int(big_int, 9900));

    printf("num_digits=%d, alloc_digits=%d, should output '100': ", big_int->num_digits, big_int->num_allocated_digits);
    BigInt_print(big_int);
    printf("\n");
    BigInt_free(big_int);
}
