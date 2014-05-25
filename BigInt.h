#ifndef BIG_INT_H
#define BIG_INT_H

#ifndef NULL
#define NULL 0
#endif

// Specifies the amount of logging in the unit test suite.
// Set to 0 to disable all logging.
// Set to 1 for minimal logging.
// Set to 2 for verbose logging. 
#define BIGINT_TEST_LOGGING 1

typedef struct BigInt {
    unsigned char* digits; // Array of digits 0-9.  Greater indices hold more significant digits.
    unsigned int num_digits; // Number of digits actually in the number.
    unsigned int num_allocated_digits; // digits array has space for this many digits
    int is_negative; // Nonzero if this BigInt is negative, zero otherwise.
} BigInt;

//============================================================================
// Construction and assignment
//============================================================================

// Returns a pointer to a new BigInt initialized to the specified value.
// Caller is responsible for freeing the new BigInt with a
// corresponding call to BigInt_free.
BigInt* BigInt_construct(int value);

// Frees the memory for a BigInt allocated using BigInt_construct.
void BigInt_free(BigInt* big_int);

///Sets the value of the target BigInt to the value of the source BigInt.
// Assumes that target and source already point to valid BigInts. 
void BigInt_assign(BigInt* target, const BigInt* source);

///Sets the value of the target BigInt to the value of the source int.
void BigInt_assign_int(BigInt* target, const int source);

//============================================================================
// Basic mathematical operations
//============================================================================

// Returns -1 if a < b, 0 if a == b, 1 if a > b 
int BigInt_compare(const BigInt* a, const BigInt* b);

// Adds the value in addend to big_int.  Places the result in big_int.
void BigInt_add(BigInt* big_int, const BigInt* addend);
void BigInt_add_int(BigInt* big_int, const int addend);

// Subtracts the value of to_subtract from big_int. 
// Places the result in big_int.
void BigInt_subtract(BigInt* big_int, const BigInt* to_subtract);
void BigInt_subtract_int(BigInt* big_int, const int to_subtract);

// Multiplies the value in big_int by multiplier.  Places the
// result in big_int.
void BigInt_multiply(BigInt* big_int, const BigInt* multiplier);
void BigInt_multiply_int(BigInt* big_int, const int multiplier);

// Returns the value of big_int as an integer.  Requires that the
// value of big_int fits within the size of an int on the target
// environment.  Result is undefined if this is not the case.
int BigInt_to_int(const BigInt* big_int);

// Prints the contents of big_int to stdout.
void BigInt_print(const BigInt* big_int);

//============================================================================
// Internal helpers
//============================================================================

// Ensure that big_int has space allocated for at least digits_needed digits.
void BigInt_ensure_digits(BigInt* big_int, unsigned int digits_needed);

// Performs an unsigned comparison of the two BigInt parameters; that is, the
// comparison is of their absolute values.  Returns 1 if |a| > |b|, 0 if |a| == |b|,
// and -1 if |a| < |b|.
int BigInt_compare_digits(const BigInt* a, const BigInt* b);

// Performs an unsigned addition of to_add to big_int; adds the digits without regard
// for the sign of either parameter. 
void BigInt_add_digits(BigInt* big_int, const BigInt* to_add);

// Performs an unsigned subtraction of to_subtract from big_int; subtracts the digits
// without regard for the sign of either parameter. 
void BigInt_subtract_digits(BigInt* big_int, const BigInt* to_subtract);

//============================================================================
// Unit tests
//============================================================================

typedef enum { ADD, ADD_INT, SUBTRACT, SUBTRACT_INT, MULTIPLY, MULTIPLY_INT,
        COMPARE, OPERATION_TYPE_MAX} OPERATION_TYPE;
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

#endif // BIG_INT_H

