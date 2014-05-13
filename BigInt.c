#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _DEBUG
#include "CMemLeak.h"

#include "BigInt.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define MIN(x, y) ((x) < (y) ? (x) : (y))

void BigInt_realloc_digits(BigInt* big_int, unsigned int num_digits) {
    assert(num_digits > big_int->num_digits);
}

BigInt* BigInt_construct(int value) {

    BigInt* new_big_int = malloc(sizeof(BigInt));
    
    if(value < 0) {
        new_big_int->is_negative = 1;
        value *= -1;
    } else {
        new_big_int->is_negative = 0;
    }

    new_big_int->num_digits = floor(log10(value)) + 1;

    // Special case for 0
    if(new_big_int->num_digits == 0) {
        new_big_int->num_digits = 1;
    }

    // printf("Constructing BigInt with %i digits\n", new_big_int->num_digits);

    new_big_int->num_allocated_digits = new_big_int->num_digits;
    new_big_int->digits = (unsigned char*)malloc(new_big_int->num_allocated_digits * sizeof(unsigned char));

    int i;
    for(i = 0; i < new_big_int->num_digits; i++) {
        new_big_int->digits[i] = value % 10;
        value /= 10;
    }

    return new_big_int;
}

void BigInt_free(BigInt* big_int) {
    free(big_int->digits);
    free(big_int);
}


int BigInt_compare(const BigInt* a, const BigInt* b) {
    // Quick return if one is negative and the other isn't
    if(a->num_digits > 0 || a->digits[0] > 0 || b->num_digits > 0 || b->digits[0] > 0) {
        if (a->is_negative && !b->is_negative) {
            return -1;
        } else if (!a->is_negative && b->is_negative) {
            return 1;
        }
    }

    return a->is_negative ? BigInt_compare_digits(b, a) : BigInt_compare_digits(a, b);
}

int BigInt_compare_digits(const BigInt* a, const BigInt* b) {
    // Not looking at the sign here, comparing the digits only.

    // Quick return if one number has more digits than the other
    if(a->num_digits > b->num_digits) {
       return 1; 
    } else if(a->num_digits < b->num_digits) {
       return -1; 
    }

    // Both have the same number of digits, so we actually have to loop through until we
    // find one that doesn't match.
    int i;
    for(i = a->num_digits - 1; i >= 0; --i) {
        if(a->digits[i] > b->digits[i]) {
            return 1;
        } else if(a->digits[i] < b->digits[i]) {
            return -1;
        }
    }

    // All digits match; numbers are equal
    return 0;
}

void BigInt_add(BigInt* big_int, const BigInt* addend) {
    if(big_int->is_negative == addend->is_negative) {
        // Sign will never change in this case so just leave
        // it as-is.
        BigInt_add_digits(big_int, addend);
    } else {
        // Figure out the sign.  Need to do this before calculating the digits of
        // the digits result because changing those in big_int will affect the result
        // of the compare.
        unsigned int result_is_negative = BigInt_compare_digits(big_int, addend) > 0 ?
                big_int->is_negative : addend->is_negative;
    
        BigInt_subtract_digits(big_int, addend);
        big_int->is_negative = result_is_negative;
    }
}

void BigInt_add_digits(BigInt* big_int, const BigInt* addend) {
    unsigned int digits_needed = MAX(big_int->num_digits, addend->num_digits) + 1;
    BigInt_ensure_digits(big_int, digits_needed);

    int i;
    int carry = 0;
    for(i = 0; i < addend->num_digits || carry > 0; ++i) {
        // Append another digit if necessary 
        if(i == big_int->num_digits) {
            ++big_int->num_digits;
            big_int->digits[i] = 0;
        }

        unsigned int addend_digit = i < addend->num_digits ? addend->digits[i] : 0; 
        unsigned int total = big_int->digits[i] + addend_digit + carry;
        big_int->digits[i] = total % 10;
        carry = (total >= 10) ? 1 : 0; 
    }
}

void BigInt_subtract(BigInt* big_int, const BigInt* to_subtract) {
    // Figure out the sign.  Need to do this before calculating the digits of
    // the digits result because changing those in big_int will affect the result
    // of the compare.
    unsigned int result_is_negative = BigInt_compare(big_int, to_subtract) > 0 ? 0 : 1;
    
    // Calculate the digits
    if(big_int->is_negative == to_subtract->is_negative) {
        BigInt_subtract_digits(big_int, to_subtract);
    } else {
        BigInt_add_digits(big_int, to_subtract);
    }
    
    // Figure out the sign
    big_int->is_negative = result_is_negative;
}


void BigInt_subtract_digits(BigInt* big_int, const BigInt* to_subtract) {

    unsigned int digits_needed = MAX(big_int->num_digits, to_subtract->num_digits) + 1;
    BigInt_ensure_digits(big_int, digits_needed);
    
    // Determine the larger int.  This will go on "top"
    // of the subtraction.  Sign doesn't matter here since we've already
    // determined the sign of the final result above.
    unsigned char* greater_int_digits;
    unsigned char* smaller_int_digits;
    unsigned int smaller_int_num_digits;
    unsigned int greater_int_num_digits;

    if(BigInt_compare_digits(big_int, to_subtract) > 0) {
        greater_int_digits = big_int->digits;
        greater_int_num_digits = big_int->num_digits;
        smaller_int_digits = to_subtract->digits;
        smaller_int_num_digits = to_subtract->num_digits;
    } else {
        greater_int_digits = to_subtract->digits;
        greater_int_num_digits = to_subtract->num_digits;
        smaller_int_digits = big_int->digits;
        smaller_int_num_digits = big_int->num_digits;
    }

    // Actually carry out the subtraction. 
    int i;
    int carry = 0;
    big_int->num_digits = 1;

    for(i = 0; i < greater_int_num_digits; ++i) {
        int new_digit;
        if(i < smaller_int_num_digits) {
            new_digit = (int)greater_int_digits[i] - (int)smaller_int_digits[i] + carry;
        } else {
            new_digit = (int)greater_int_digits[i] + carry;
        } 

        // Carry 10 from the next digit if necessary
        if(new_digit < 0) {
            carry = -1;
            new_digit += 10;
        } else {
            carry = 0;
        }

        assert(new_digit >= 0);
        big_int->digits[i] = new_digit;
        if(new_digit != 0) {
            big_int->num_digits = i + 1;
        }
    }

    assert(carry == 0);
}

int BigInt_to_int(const BigInt* big_int) {
    int value = 0;
    int tens_multiplier = 1;

    int i;
    for(i = 0; i < big_int->num_digits; i++) {
        value += big_int->digits[i] * tens_multiplier;
        tens_multiplier *= 10;
    } 

    if (big_int->is_negative) {
        value *= -1;
    }

    return value;

}

void BigInt_print(const BigInt* big_int) {
    int i;
    for(i = big_int->num_digits - 1; i >= 0; --i) {
        printf("%i", big_int->digits[i]);
    }
}

void BigInt_ensure_digits(BigInt* big_int, unsigned int digits_needed) {
    if(big_int->num_allocated_digits < digits_needed) {
        unsigned char* digits = big_int->digits;

        big_int->digits = malloc(digits_needed * sizeof(unsigned char));
        memcpy(big_int->digits, digits, big_int->num_digits);    
        big_int->num_allocated_digits = digits_needed;

        free(digits);
    }
}

#ifdef BUILD_BIGINT_TESTS

const char* OPERATION_NAMES[] = {"Addition", "Subtraction", "Comparison"};

void BigInt_test_basic() {

    printf("Testing construction\n");
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
    printf("Testing digit reallocation\n");
    BigInt* big_int = BigInt_construct(42);
    assert(BigInt_to_int(big_int) == 42);
    BigInt_ensure_digits(big_int, 1000);
    assert(BigInt_to_int(big_int) == 42);
    BigInt_ensure_digits(big_int, 1);
    assert(BigInt_to_int(big_int) == 42);
    BigInt_free(big_int);

    // Test addition, subtraction, and comparison for all positive and
    // negative permutations of these integers
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
            case SUBTRACT:
                BigInt_test_permutations((Generic_function)BigInt_subtract, operation_type, a, b); 
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
    
    if(BIGINT_TEST_LOGGING) {
        printf("Testing %s for %i, %i\n", OPERATION_NAMES[operation_type], a, b);
    }

    BigInt* big_int_a = BigInt_construct(a);
    BigInt* big_int_b = BigInt_construct(b);

    int result; 

    switch(operation_type) {
        case ADD:
        case SUBTRACT:
            ((void(*)(BigInt*, const BigInt*))(*BigInt_operation_to_test))(big_int_a, big_int_b);
            result = BigInt_to_int(big_int_a);
            break;
        case COMPARE:
            result = ((int(*)(const BigInt*, const BigInt*))(*BigInt_operation_to_test))(big_int_a, big_int_b);
            break;
        default:
            printf("Unsupported operation\n");
            assert(0);
    }
 
    if(BIGINT_TEST_LOGGING) {
        printf("%s result: %i\n", OPERATION_NAMES[operation_type], result);
    }

    switch(operation_type) {
        case ADD:
            assert(result == a + b);
            break;
        case SUBTRACT:
            assert(result == a - b);
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

#endif // BUILD_BIGINT_TESTS



