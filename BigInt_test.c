#include <assert.h>
#include <errno.h>
#include <limits.h>
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
        printf("Testing assign_int\n");
    }
    BigInt_test_assign_int();

    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing to_int\n");
    }
    BigInt_test_to_int();

    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing strings\n");
    }
    BigInt_test_strings();

    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing division\n");
    }
    BigInt_test_division();

    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing clone\n");
    }
    BigInt_test_clone();

    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing compare\n");
    }
    BigInt_test_compare();

    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing negative zero\n");
    }
    BigInt_test_negative_zero();

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

    if(BIGINT_TEST_LOGGING > 0) {
        printf("Testing addition digit-count overflow\n");
    }
    BigInt_test_add_digits_overflow();

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

void BigInt_test_assign_int() {
    const int values[] = { 0, 1, -1, 7, -7, -20, 42, -42, 100, -100,
                           999999999, -999999999, INT_MAX, INT_MIN };

    for(unsigned int i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        int v = values[i];

        // Start from an arbitrary non-zero value to make sure assign_int
        // overwrites both the digits and the sign.
        BigInt* b = BigInt_construct(-777);
        assert(b);
        assert(BigInt_assign_int(b, v));

        // The sign flag must match the sign of the source.
        assert(b->is_negative == (v < 0));

        // The value must round-trip through its decimal string.  (A string
        // comparison also covers INT_MIN, which BigInt_to_int cannot yet
        // represent.)
        char expected[16];
        snprintf(expected, sizeof(expected), "%d", v);
        char* actual = BigInt_to_new_string(b);
        assert(actual);
        if(strcmp(expected, actual)) {
            printf("BigInt_assign_int(%d) produced %s, expected %s\n",
                    v, actual, expected);
            assert(0);
        }

        free(actual);
        BigInt_free(b);
    }
}

void BigInt_test_add_digits_overflow() {
    // The one-byte buffers are intentional. Digit-count overflow must be
    // rejected before BigInt_add_digits accesses the actual digit buffers.
    unsigned char target_digit = 1;
    unsigned char addend_digit = 1;
    BigInt target = {
        .digits = &target_digit,
        .num_digits = UINT_MAX,
        .num_allocated_digits = UINT_MAX,
        .is_negative = 0
    };
    const BigInt addend = {
        .digits = &addend_digit,
        .num_digits = 1,
        .num_allocated_digits = 1,
        .is_negative = 0
    };

    errno = 0;
    assert(!BigInt_add_digits(&target, &addend));
    assert(errno == ERANGE);
    assert(target.digits == &target_digit);
    assert(target.num_digits == UINT_MAX);
    assert(target.num_allocated_digits == UINT_MAX);
    assert(target_digit == 1);
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
    
    // Zero and its variants must yield a well-formed single-digit zero:
    // a valid decimal string "0" with no negative-zero sign.
    big_int = BigInt_from_string("0");
    assert(big_int);
    assert(BigInt_to_int(big_int, &value));
    assert(value == 0);
    assert(big_int->num_digits == 1);
    str = BigInt_to_new_string(big_int);
    assert(str && !strcmp(str, "0"));
    free(str);
    BigInt_free(big_int);

    // We should treat the empty string as a zero.
    big_int = BigInt_from_string("");
    assert(big_int);
    assert(BigInt_to_int(big_int, &value));
    assert(value == 0);
    assert(big_int->num_digits == 1);
    assert(!big_int->is_negative);
    str = BigInt_to_new_string(big_int);
    assert(str && !strcmp(str, "0"));
    free(str);
    BigInt_free(big_int);
    
    big_int = BigInt_from_string("-0");
    assert(big_int);
    assert(BigInt_to_int(big_int, &value));
    assert(value == 0);
    assert(!big_int->is_negative); // -0 normalizes to +0
    str = BigInt_to_new_string(big_int);
    assert(str && !strcmp(str, "0"));
    free(str);
    BigInt_free(big_int);
    
    big_int = BigInt_from_string("0000");
    assert(big_int);
    assert(BigInt_to_int(big_int, &value));
    assert(value == 0);
    str = BigInt_to_new_string(big_int);
    assert(str && !strcmp(str, "0"));
    free(str);
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
	// Basic division:
	_BigInt_test_division( "1000", "10", "100", "0" );
	
	// Too big to fit into 32-bit int:
	_BigInt_test_division( "963096309630", "30", "32103210321", "0" );
	_BigInt_test_division( "300000000000000000000", "3000000000000000", "100000", "0" );
	
	// Test remainder:
	_BigInt_test_division( "10", "3", "3", "1" );

	// For negative operands, the quotient's sign is the XOR of the operand
    // signs and the remainder takes the dividend's sign.
	_BigInt_test_division( "-10", "3", "-3", "-1" );
	_BigInt_test_division( "10", "-3", "-3", "1" );
	_BigInt_test_division( "-10", "-3", "3", "-1" );

	// Exact division with negatives: a zero remainder must be non-negative.
	_BigInt_test_division( "-1000", "10", "-100", "0" );
	_BigInt_test_division( "1000", "-10", "-100", "0" );
	_BigInt_test_division( "-1000", "-10", "100", "0" );

	// Zero dividend divided by a negative divisor stays a clean zero.
	_BigInt_test_division( "0", "-7", "0", "0" );

	// |dividend| < |divisor|: quotient is 0, remainder keeps the dividend's sign.
	_BigInt_test_division( "-3", "10", "0", "-3" );
	_BigInt_test_division( "3", "-10", "0", "3" );

	// Negative operands too big for a 32-bit int:
	_BigInt_test_division( "-963096309630", "30", "-32103210321", "0" );
	_BigInt_test_division( "963096309630", "-30", "-32103210321", "0" );
	_BigInt_test_division( "-300000000000000000001", "3000000000000000", "-100000", "-1" );
}

// Build a BigInt from `value`, clone it requesting `requested` allocated
// digits, and verify the clone is an independent, correct copy whose
// allocation covers all of its digits.
static void _BigInt_check_clone(const char* value, unsigned int requested) {
    BigInt* original = BigInt_from_string(value);
    assert(original);

    BigInt* clone = BigInt_clone(original, requested);
    assert(clone);

    // Allocation must always cover num_digits, and a larger request
    // must be honored rather than clamped down.
    unsigned int expected_alloc =
        requested < original->num_digits ? original->num_digits : requested;
    assert(clone->num_allocated_digits >= clone->num_digits);
    assert(clone->num_allocated_digits == expected_alloc);

    // The clone must match the original's value, sign, and length.
    assert(clone->num_digits == original->num_digits);
    assert(clone->is_negative == original->is_negative);
    assert(BigInt_compare(clone, original) == 0);

    char* clone_str = BigInt_to_new_string(clone);
    assert(clone_str && !strcmp(clone_str, value));

    // The clone must own a separate buffer: mutating the clone must leave the
    // original untouched.
    assert(clone->digits != original->digits);
    assert(BigInt_add_int(clone, 1));
    char* mutated = BigInt_to_new_string(clone);
    assert(mutated && strcmp(mutated, value) != 0);
    char* original_str = BigInt_to_new_string(original);
    assert(original_str && !strcmp(original_str, value));

    free(clone_str);
    free(mutated);
    free(original_str);
    BigInt_free(clone);
    BigInt_free(original);
}

void BigInt_test_clone() {
    // Under-allocation: request fewer digits than the value has.  The clone must
    // still allocate room for every digit it copies.
    _BigInt_check_clone("123456789", 2);
    _BigInt_check_clone("123456789", 0);

    // Exact and over-allocation (a larger request must be preserved).
    _BigInt_check_clone("123456789", 9);
    _BigInt_check_clone("123456789", 20);

    // Negative values and single digits.
    _BigInt_check_clone("-123456789", 3);
    _BigInt_check_clone("7", 0);
    _BigInt_check_clone("-7", 5);

    // Zero.
    _BigInt_check_clone("0", 0);
    _BigInt_check_clone("0", 10);
}

// Verifies BigInt_compare(a, b) == expected (-1/0/1) for values built from the
// given decimal strings, and checks antisymmetry and reflexivity.
static void _BigInt_check_compare(const char* a_str, const char* b_str, int expected) {
    BigInt* a = BigInt_from_string(a_str);
    BigInt* b = BigInt_from_string(b_str);
    assert(a && b);

    assert(BigInt_compare(a, b) == expected);
    assert(BigInt_compare(b, a) == -expected); // antisymmetry
    assert(BigInt_compare(a, a) == 0);         // reflexivity
    assert(BigInt_compare(b, b) == 0);

    BigInt_free(a);
    BigInt_free(b);
}

void BigInt_test_compare() {
    // Equal values, various signs and magnitudes.
    _BigInt_check_compare("0", "0", 0);
    _BigInt_check_compare("5", "5", 0);
    _BigInt_check_compare("-5", "-5", 0);
    _BigInt_check_compare("123456789", "123456789", 0);

    // Same sign, differing magnitude.
    _BigInt_check_compare("5", "6", -1);
    _BigInt_check_compare("100", "99", 1);    // more digits is greater
    _BigInt_check_compare("-5", "-6", 1);     // -5 > -6
    _BigInt_check_compare("-100", "-99", -1); // -100 < -99

    // Different signs.
    _BigInt_check_compare("1", "-1", 1);
    _BigInt_check_compare("-1", "1", -1);
    _BigInt_check_compare("1", "0", 1);
    _BigInt_check_compare("-1", "0", -1);
    _BigInt_check_compare("0", "1", -1);
    _BigInt_check_compare("0", "-1", 1);

    // Values that do not fit in an int, to exercise real BigInt comparison
    // rather than something an int round-trip could shortcut.
    _BigInt_check_compare("98765432109876543210", "98765432109876543211", -1);
    _BigInt_check_compare("100000000000000000000", "99999999999999999999", 1);
    _BigInt_check_compare("-98765432109876543210", "98765432109876543210", -1);

    // Negative zero must compare equal to positive zero.
    // A negative zero cannot be produced via from_string
    // (it normalizes -0 to +0),
    // so build one directly by flagging a zero as negative.
    BigInt* neg_zero = BigInt_construct(0);
    assert(neg_zero);
    neg_zero->is_negative = 1;
    BigInt* pos_zero = BigInt_construct(0);
    assert(pos_zero);

    assert(BigInt_compare(neg_zero, pos_zero) == 0);
    assert(BigInt_compare(pos_zero, neg_zero) == 0);
    assert(BigInt_compare(neg_zero, neg_zero) == 0);

    // A negative zero must still order correctly against non-zero values.
    BigInt* five = BigInt_construct(5);
    BigInt* neg_five = BigInt_construct(-5);
    assert(five && neg_five);
    assert(BigInt_compare(neg_zero, five) == -1);     // 0 < 5
    assert(BigInt_compare(neg_zero, neg_five) == 1);  // 0 > -5

    BigInt_free(neg_zero);
    BigInt_free(pos_zero);
    BigInt_free(five);
    BigInt_free(neg_five);
}

// Arithmetic whose result is zero must produce a canonical, non-negative zero
// (stringifies to "0", never "-0").
void BigInt_test_negative_zero() {
    struct { int a; int b; char op; } cases[] = {
        {-5, 0, '*'},   // negative times zero
        {0, -123, '*'}, // zero times negative
        {5, -5, '+'},   // opposite signs summing to zero
        {-5, 5, '+'},
        {5, 5, '-'},    // equal values subtracting to zero
        {-5, -5, '-'},
    };

    for(unsigned int i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        BigInt* x = BigInt_construct(cases[i].a);
        BigInt* y = BigInt_construct(cases[i].b);
        assert(x && y);

        switch(cases[i].op) {
            case '*': assert(BigInt_multiply(x, y)); break;
            case '+': assert(BigInt_add(x, y)); break;
            case '-': assert(BigInt_subtract(x, y)); break;
        }

        assert(!x->is_negative);
        char* s = BigInt_to_new_string(x);
        assert(s && !strcmp(s, "0"));

        free(s);
        BigInt_free(x);
        BigInt_free(y);
    }
}

// Verifies BigInt_to_int covers the full int range and
// reports ERANGE for values that do not fit.
void BigInt_test_to_int() {
    struct { const char* s; int expected; } ok_cases[] = {
        {"0", 0},
        {"42", 42},
        {"-42", -42},
        {"2147483647", INT_MAX},
        {"-2147483647", -2147483647},
        {"-2147483648", INT_MIN},
    };
    for(unsigned int i = 0; i < sizeof(ok_cases) / sizeof(ok_cases[0]); i++) {
        BigInt* b = BigInt_from_string(ok_cases[i].s);
        assert(b);
        int v;
        assert(BigInt_to_int(b, &v));
        assert(v == ok_cases[i].expected);
        BigInt_free(b);
    }

    const char* range_cases[] = {
        "2147483648",   // INT_MAX + 1
        "-2147483649",  // INT_MIN - 1
        "99999999999",  // far beyond int range
        "-99999999999",
    };
    for(unsigned int i = 0; i < sizeof(range_cases) / sizeof(range_cases[0]); i++) {
        BigInt* b = BigInt_from_string(range_cases[i]);
        assert(b);
        int v;
        errno = 0;
        assert(!BigInt_to_int(b, &v));
        assert(errno == ERANGE);
        BigInt_free(b);
    }
}

void BigInt_test_operations(int a, int b) {

    OPERATION_TYPE operation_type;

    for(operation_type = 0; operation_type < OPERATION_TYPE_COUNT; operation_type++) {
        BigInt_test_permutations(operation_type, a, b);
    }
}

// Calls the specified BigInt 2-operand function for all
// permutations of positive, negative, and order-reversals
// of the values a and b.
void BigInt_test_permutations(OPERATION_TYPE operation_type, int a, int b) {

    BigInt_test_single_operation(operation_type, a, b);
    BigInt_test_single_operation(operation_type, -a, b);
    BigInt_test_single_operation(operation_type, a, -b);
    BigInt_test_single_operation(operation_type, -a, -b);
    BigInt_test_single_operation(operation_type, b, a);
    BigInt_test_single_operation(operation_type, -b, a);
    BigInt_test_single_operation(operation_type, b, -a);
    BigInt_test_single_operation(operation_type, -b, -a);
}

void BigInt_test_single_operation(OPERATION_TYPE operation_type, int a, int b) {
    
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
            assert(BigInt_add(big_int_a, big_int_b));
            assert(BigInt_to_int(big_int_a, &result));
            break;
        case SUBTRACT:
            assert(BigInt_subtract(big_int_a, big_int_b));
            assert(BigInt_to_int(big_int_a, &result));
            break;
        case MULTIPLY:
            assert(BigInt_multiply(big_int_a, big_int_b));
            assert(BigInt_to_int(big_int_a, &result));
            break;
        case ADD_INT:
            assert(BigInt_add_int(big_int_a, b));
            assert(BigInt_to_int(big_int_a, &result));
            break;
        case SUBTRACT_INT:
            assert(BigInt_subtract_int(big_int_a, b));
            assert(BigInt_to_int(big_int_a, &result));
            break;
        case MULTIPLY_INT:
            assert(BigInt_multiply_int(big_int_a, b));
            assert(BigInt_to_int(big_int_a, &result));
            break;
        case COMPARE:
            result = BigInt_compare(big_int_a, big_int_b);
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
