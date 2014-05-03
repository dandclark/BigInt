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


    printf("Constructing BigInt with %i digits\n", new_big_int->num_digits);

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


int BigInt_compare(BigInt* a, BigInt* b) {
    // Quick return if one is negative and the other isn't
    if(a->num_digits > 0 || a->digits[0] > 0 || b->num_digits > 0 || b->digits[0] > 0) {
        if (a->is_negative && !b->is_negative) {
            return -1;
        } else if (!a->is_negative && b->is_negative) {
            return 1;
        }
    }

    // Another quick return if one number has more digits than the other
    if(a->num_digits > b->num_digits) {
       return a->is_negative ? -1 : 1; 
    } else if(a->num_digits < b->num_digits) {
       return a->is_negative ? 1 : -1; 
    }

    // Both have the same number of digits, so we actually have to loop through until we
    // find one that doesn't match.
    int i;
    for(i = a->num_digits - 1; i >= 0; --i) {
        if(a->digits[i] > b->digits[i]) {
            return a->is_negative ? -1 : 1;
        } else if(a->digits[i] < b->digits[i]) {
            return a->is_negative ? 1 : -1;
        }
    }

    // All digits match; numbers are equal
    return 0;
}


void BigInt_add(BigInt* big_int, BigInt* addend) {
    // TODO: Handle negative numbers?
    int i;
    int carry = 0;

    unsigned int digits_needed = MAX(big_int->num_digits, addend->num_digits) + 1;
    BigInt_ensure_digits(big_int, digits_needed);


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

void BigInt_subtract(BigInt* big_int, BigInt* subtraction) {
   /* 
    // TODO: Handle negative numbers?
    int i;
    int carry = 0;

    unsigned int digits_needed = MAX(big_int->num_digits, addend->num_digits) + 1;
    BigInt_ensure_digits(big_int, digits_needed);


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
*/
}

int BigInt_to_int(BigInt* big_int) {
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

void BigInt_print(BigInt* big_int) {
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

void BigInt_test_basic() {
    // TODO: Add tests for negative numbers


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
    printf("Testing digit realoocation\n");
    BigInt* big_int = BigInt_construct(42);
    assert(BigInt_to_int(big_int) == 42);
    BigInt_ensure_digits(big_int, 1000);
    assert(BigInt_to_int(big_int) == 42);
    BigInt_ensure_digits(big_int, 1);
    assert(BigInt_to_int(big_int) == 42);
    BigInt_free(big_int);

    printf("Testing addition\n");
    BigInt_test_add(0, 0);
    BigInt_test_add(1, 1);
    BigInt_test_add(5, 5);
    BigInt_test_add(5, 6);
    BigInt_test_add(10, 2);
    BigInt_test_add(14, 16);
    BigInt_test_add(16, 18);
    BigInt_test_add(11, 111);
    BigInt_test_add(123456, 1234);
    BigInt_test_add(999999999, 1);
    BigInt_test_add(0, 12345678);
    BigInt_test_add(12345678, 0);
   
    // TODO: test harness should automatically swap,
    // test both directions 
    printf("Testing comparison\n");
    BigInt_test_compare(0, 0);
    BigInt_test_compare(1, 1);
    BigInt_test_compare(50, 50);
    BigInt_test_compare(51, 50);
    BigInt_test_compare(64, 46);
    BigInt_test_compare(46, 64);
    BigInt_test_compare(1000, 999);
    BigInt_test_compare(999, 1000);
    BigInt_test_compare(5555, 5556);
    BigInt_test_compare(-1, 1);
    BigInt_test_compare(-1, -1);
    BigInt_test_compare(-30, -28);
    BigInt_test_compare(1, -50);
    BigInt_test_compare(-1, 50);
    BigInt_test_compare(-46, 64);
    BigInt_test_compare(-100, -101);
    BigInt_test_compare(-1000, -999);
    BigInt_test_compare(-999, -1000);
    BigInt_test_compare(-5555, -5556);

    /*printf("Testing subtraction\n");
    BigInt_test_subtract(0, 0);
    BigInt_test_subtract(1, 1);
    BigInt_test_subtract(5, 5);
    BigInt_test_subtract(5, 6);
    BigInt_test_subtract(10, 2);
    BigInt_test_subtract(14, 16);
    BigInt_test_subtract(16, 18);
    BigInt_test_subtract(11, 111);
    BigInt_test_subtract(123456, 1234);
    BigInt_test_subtract(999999999, 1);
    BigInt_test_subtract(0, 12345678);
    BigInt_test_subtract(12345678, 0);*/
}

void BigInt_test_construct(int value) {
    BigInt* big_int = BigInt_construct(value);
    assert(BigInt_to_int(big_int) == value);
    BigInt_free(big_int);
}

// 
void BigInt_test_compare(int a, int b) {

    // TODO: test harness should automatically swap,
    // test both directions 
    BigInt* big_int_a = BigInt_construct(a);
    BigInt* big_int_b = BigInt_construct(b);
   
    int compare_result = BigInt_compare(big_int_a, big_int_b);
    if(a > b) {
        assert(compare_result == 1);
    } else if(a < b) {
        assert(compare_result == -1);
    } else {
        assert(compare_result == 0);
    }

    BigInt_free(big_int_a);
    BigInt_free(big_int_b);
}

void BigInt_test_add(int a, int b) {

    BigInt* big_int_a = BigInt_construct(a);
    BigInt* big_int_b = BigInt_construct(b);
    
    BigInt_add(big_int_a, big_int_b);
    assert(BigInt_to_int(big_int_a) == a + b);

    BigInt_free(big_int_a);
    BigInt_free(big_int_b);
}

void BigInt_test_subtract(int a, int b) {

    BigInt* big_int_a = BigInt_construct(a);
    BigInt* big_int_b = BigInt_construct(b);
    
    BigInt_subtract(big_int_a, big_int_b);
    assert(BigInt_to_int(big_int_a) == a - b);

    BigInt_free(big_int_a);
    BigInt_free(big_int_b);
}

#endif // BUILD_BIGINT_TESTS



