#ifndef BIG_INT_H
#define BIG_INT_H

#include <stdint.h>

#ifndef NULL
#define NULL 0
#endif

#ifndef BOOL
#define BOOL int8_t
#endif

typedef struct BigInt {
    unsigned char* digits; // Array of digits 0-9.  Greater indices hold more significant digits.
    unsigned int num_digits; // Number of digits actually in the number.
    unsigned int num_allocated_digits; // digits array has space for this many digits
    BOOL is_negative; // Nonzero if this BigInt is negative, zero otherwise.
} BigInt;

//============================================================================
// Construction and assignment
//============================================================================

// Returns a pointer to a new BigInt initialized to the specified value.
// Caller is responsible for freeing the new BigInt with a
// corresponding call to BigInt_free.
// returns NULL on memory allocation failure.
BigInt* BigInt_construct(int value);

// Returns a pointer to a new BigInt initialized from the supplied BigInt.
// Caller is responsible for freeing the new BigInt with a
// corresponding call to BigInt_free.
// returns NULL on memory allocation failure.
BigInt* BigInt_clone(const BigInt* big_int, unsigned int num_allocated_digits);

// Returns a pointer to a new BigInt initialized from digits in the specified
// zero-terminated string. Caller is responsible for freeing the new BigInt
// with a corresponding call to BigInt_free
BigInt* BigInt_from_string(const char* str);

// Frees the memory for a BigInt allocated using BigInt_construct.
void BigInt_free(BigInt* big_int);

///Sets the value of the target BigInt to the value of the source BigInt.
// Assumes that target and source already point to valid BigInts.
// returns non-zero on success or 0 on failure
BOOL BigInt_assign(BigInt* target, const BigInt* source);

///Sets the value of the target BigInt to the value of the source int.
// returns non-zero on success or 0 on failure
BOOL BigInt_assign_int(BigInt* target, const int source);

// Prints the contents of big_int to stdout.
void BigInt_print(const BigInt* big_int);

// Prints the contents of big_int to output stream.
void BigInt_fprint(FILE *dest, const BigInt* big_int);

// what would be the length of a string if this BigInt were converted to a string
// (see BigInt_to_string() below)
unsigned int BigInt_strlen(const BigInt* big_int);

// write BigInt to a string buffer, returns non-zero on success
// returns zero if BigInt doesn't fit into buf.
// buf_size *must* include the terminating zero byte
BOOL BigInt_to_string(const BigInt* big_int, char* buf, unsigned int buf_size);

// convert BigInt to a newly allocated string.
// returns NULL on failure.
char* BigInt_to_new_string(const BigInt* big_int);

//============================================================================
// Basic mathematical operations
//============================================================================

// Returns -1 if a < b, 0 if a == b, 1 if a > b 
int BigInt_compare(const BigInt* a, const BigInt* b);

// Returns -1 if a < b, 0 if a == b, 1 if a > b
int BigInt_compare_int(const BigInt* a, int b);

// Adds the value in addend to big_int.  Places the result in big_int.
// returns non-zero on success or 0 on failure
BOOL BigInt_add(BigInt* big_int, const BigInt* addend);
BOOL BigInt_add_int(BigInt* big_int, const int addend);

// Subtracts the value of to_subtract from big_int.
// Places the result in big_int.
// returns non-zero on success or 0 on failure
BOOL BigInt_subtract(BigInt* big_int, const BigInt* to_subtract);
BOOL BigInt_subtract_int(BigInt* big_int, const int to_subtract);

// Multiplies the value in big_int by multiplier.  Places the
// result in big_int.
// returns non-zero on success or 0 on failure
BOOL BigInt_multiply(BigInt* big_int, const BigInt* multiplier);
BOOL BigInt_multiply_int(BigInt* big_int, const int multiplier);

// Divides dividend by divisor.
// If only quotient is desired, remainder can be NULL
// If only remainder is desired, quotient can be NULL
// if both quotient and remainder are NULL, this is just a fancy way of burning cpu
// returns non-zero on success or 0 on failure
BOOL BigInt_divide(
    BigInt* dividend, BigInt* divisor,
    BigInt* quotient, BigInt* remainder
);

// Sets result to the value of big_int as an integer if the
// value of big_int fits within the size of result's type on the target
// environment.  returns non-zero on success or 0 on failure.
// result is undefined on failure.
BOOL BigInt_to_int(const BigInt* big_int, int* result);

//============================================================================
// Internal helpers
//============================================================================

// Ensure that big_int has space allocated for at least digits_needed digits.
// returns non-zero on success or 0 on failure
BOOL BigInt_ensure_digits(BigInt* big_int, unsigned int digits_needed);

// Performs an unsigned comparison of the two BigInt parameters; that is, the
// comparison is of their absolute values.  Returns 1 if |a| > |b|, 0 if |a| == |b|,
// and -1 if |a| < |b|.
int BigInt_compare_digits(const BigInt* a, const BigInt* b);

// Performs an unsigned addition of to_add to big_int; adds the digits without regard
// for the sign of either parameter. 
// returns non-zero on success or 0 on failure
BOOL BigInt_add_digits(BigInt* big_int, const BigInt* to_add);

// Performs an unsigned subtraction of to_subtract from big_int; subtracts the digits
// without regard for the sign of either parameter. 
// returns non-zero on success or 0 on failure
BOOL BigInt_subtract_digits(BigInt* big_int, const BigInt* to_subtract);

#endif // BIG_INT_H

