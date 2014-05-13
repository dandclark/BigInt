#ifndef BIG_INT_H
#define BIG_INT_H

#ifndef NULL
#define NULL 0
#endif

#define BUILD_BIGINT_TESTS
#define BIGINT_TEST_LOGGING 1

typedef struct BigInt {

    unsigned char* digits;
    unsigned int num_digits;
    unsigned int num_allocated_digits;
    int is_negative;

} BigInt;

BigInt* BigInt_construct(int value);
void BigInt_free(BigInt* big_int);

/* Returns -1 if a < b, 0 if a == b, 1 if a > b */
int BigInt_compare(const BigInt* a, const BigInt* b);

void BigInt_add(BigInt* big_int, const BigInt* addend);
void BigInt_subtract(BigInt* big_int, const BigInt* to_subtract);

int BigInt_to_int(const BigInt* big_int);

void BigInt_print(const BigInt* big_int);

void BigInt_ensure_digits(BigInt* big_int, unsigned int digits_needed);


// Internal helpers
int BigInt_compare_digits(const BigInt* a, const BigInt* b);
void BigInt_add_digits(BigInt* big_int, const BigInt* to_add);
void BigInt_subtract_digits(BigInt* big_int, const BigInt* to_subtract);

#ifdef BUILD_BIGINT_TESTS

//enum OPERATION_TYPE { ADD, SUBTRACT, COMPARE };
typedef enum { ADD, SUBTRACT, COMPARE, OPERATION_TYPE_MAX} OPERATION_TYPE;
extern const char* OPERATION_NAMES[];

// Used to cast various BigInt functions to a generic type that can
// be passed through the test helpers BigInt_test_permutations and
// BigInt_test_single_operation.  The function pointers are cast
// back to the correct type in BigInt_test_single_operation using
// the specified OPERATION_TYPE.
typedef void*(*Generic_function)(void*);

void BigInt_test_basic();
void BigInt_test_construct(int value);
void BigInt_test_operations(int a, int b);
void BigInt_test_permutations(Generic_function BigInt_operation_to_test,
        OPERATION_TYPE operation_type, int a, int b); 
void BigInt_test_single_operation(Generic_function BigInt_operation_to_test,
        OPERATION_TYPE operation_type, int a, int b);

#endif // BUILD_BIGINT_TESTS


#endif // BIG_INT_H

