#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BigInt.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))

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

    new_big_int->num_allocated_digits = new_big_int->num_digits;
    new_big_int->digits = malloc(new_big_int->num_allocated_digits * sizeof(unsigned char));

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

void BigInt_assign(BigInt* target, const BigInt* source)
{
    BigInt_ensure_digits(target, source->num_digits);

    int i;
    for(i = 0; i < source->num_digits; ++i) {
        target->digits[i] = source->digits[i];
    }

    target->is_negative = source->is_negative;
    target->num_digits = source->num_digits;
}

void BigInt_assign_int(BigInt* target, const int source) {
    int value = source;

    if(value < 0) {
        target->is_negative = 1;
        value *= -1;
    } else {
        target->is_negative = 0;
    }

    target->num_digits = floor(log10(value)) + 1;

    // Special case for 0
    if(target->num_digits == 0) {
        target->num_digits = 1;
    }

    BigInt_ensure_digits(target, target->num_digits);

    int i;
    for(i = 0; i < target->num_digits; i++) {
        target->digits[i] = value % 10;
        value /= 10;
    }
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

void BigInt_add_int(BigInt* big_int, const int addend) {
    BigInt* big_int_addend = BigInt_construct(addend);
    BigInt_add(big_int, big_int_addend);
    BigInt_free(big_int_addend);
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


void BigInt_subtract_int(BigInt* big_int, const int to_subtract) {
    BigInt* big_int_to_subtract = BigInt_construct(to_subtract);
    BigInt_subtract(big_int, big_int_to_subtract);
    BigInt_free(big_int_to_subtract);
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

// Multiply using the pencil and paper method.  Complexity is O(n*m) where n, m are
// the number of digits in big_int and multiplier, respectively.
void BigInt_multiply(BigInt* big_int, const BigInt* multiplier) {

    // Need to keep track of the result in a separate variable because we need
    // big_int to retain its original value throughout the course of the calculation.
    BigInt* result = BigInt_construct(0);

    // addend will hold the amount to be added to the result for each step of
    // the multiplication.
    BigInt* addend = BigInt_construct(0);    

    unsigned int digits_needed = big_int->num_digits + addend->num_digits + 1;
    BigInt_ensure_digits(addend, digits_needed);

    int i, j;
    int carry = 0;
    for(i = 0; i < multiplier->num_digits; ++i) {

        if(i > 0) {
            addend->num_digits = i;
            addend->digits[i - 1] = 0;
        }

        for(j = 0; j < big_int->num_digits || carry > 0; ++j) {
            if(j + i == addend->num_digits) {
                ++addend->num_digits;
            }

            assert(digits_needed >= j + 1);
           
            int total; 
            if(j < big_int->num_digits) {
                total = (big_int->digits[j] * multiplier->digits[i]) + carry;
            } else {
                total = carry;
            }

            addend->digits[i + j] = total % 10;
            carry = total / 10;
        }

        BigInt_add(result, addend);
    }

    result->is_negative = big_int->is_negative != multiplier->is_negative;    

    // Place the result in big_int and clean things up
    BigInt_assign(big_int, result);
    BigInt_free(result);
    BigInt_free(addend);
}

void BigInt_multiply_int(BigInt* big_int, const int multiplier) {
    BigInt* big_int_multiplier = BigInt_construct(multiplier);
    BigInt_multiply(big_int, big_int_multiplier);
    BigInt_free(big_int_multiplier);
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

