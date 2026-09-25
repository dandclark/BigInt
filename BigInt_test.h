#ifndef BIG_INT_TEST_H
#define BIG_INT_TEST_H

#ifndef NULL
#define NULL 0
#endif

// Specifies the amount of logging in the unit test suite.
// Set to 0 to disable all logging.
// Set to 1 for minimal logging.
// Set to 2 for verbose logging.
#ifndef BIGINT_TEST_LOGGING
#define BIGINT_TEST_LOGGING 1
#endif//BIGINT_TEST_LOGGING

typedef enum { ADD, ADD_INT, SUBTRACT, SUBTRACT_INT, MULTIPLY, MULTIPLY_INT,
        COMPARE, OPERATION_TYPE_COUNT} OPERATION_TYPE;
extern const char* OPERATION_NAMES[];

void BigInt_test_basic();
void BigInt_test_big_multiplication();
void BigInt_test_construct(int value);
void BigInt_test_assign_int();
void BigInt_test_add_digits_overflow();
void BigInt_test_multiply_digits_overflow();
void BigInt_test_to_int();
void BigInt_test_signs();
void BigInt_test_multiply_optimized();
void BigInt_test_strings();
void BigInt_test_operations(int a, int b);
void BigInt_test_compare();
void BigInt_test_negative_zero();
void BigInt_test_permutations(OPERATION_TYPE operation_type, int a, int b);
void BigInt_test_single_operation(OPERATION_TYPE operation_type, int a, int b);
void BigInt_test_print();
void BigInt_test_division();
void BigInt_test_clone();

#endif // BIG_INT_TEST_H
