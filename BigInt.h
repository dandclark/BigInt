#ifndef BIG_INT_H
#define BIG_INT_H

#ifndef NULL
#define NULL 0
#endif

#define BUILD_BIGINT_TESTS

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
void BigInt_add_digits(BigInt* big_int, const BigInt* to_add);
void BigInt_subtract_digits(BigInt* big_int, const BigInt* to_subtract);

#ifdef BUILD_BIGINT_TESTS

void BigInt_test_basic();
void BigInt_test_construct(int value);
void BigInt_test_compare(int a, int b);
void BigInt_test_compare_helper(int a, int b);
void BigInt_test_add(int a, int b);
void BigInt_test_subtract(int a, int b);
void BigInt_test_subtract_helper(int a, int b);


#endif // BUILD_BIGINT_TESTS


#endif // BIG_INT_H

