#include <assert.h>
#include <math.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "BigInt.h"
#include "safe_math_impl.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))

#if UNIT_MAX >> 32 == 0
#    define check_add_int_int check_add_int32_int32
#    define check_add_uint_uint check_add_uint32_uint32
#    define check_mul_int_int check_mul_int32_int32
#    define check_mul_uint_uint check_mul_uint32_uint32
#else
#    if UNIT_MAX >> 64 == 0
#        define check_add_int_int check_add_int64_int64
#        define check_add_uint_uint check_add_uint64_uint64
#        define check_mul_int_int check_mul_int64_int64
#        define check_mul_uint_uint check_mul_uint64_uint64
#    else
#        error unsupported integer size
#    endif
#endif

#ifndef BIGINT_REDZONE
#define BIGINT_REDZONE 0
#endif//BIGINT_REDZONE

#if BIGINT_REDZONE
// if BIGINT_REDZONE is set to a value, that value is the number of bytes
// of extra allocation at the front and the back of the digits buffer.
// those "redzones" will then be filled with an uncommon value (0x42)
// when freed, those "redzones" will be checked to make sure they weren't modified
unsigned char* malloc_digits(unsigned int num_digits) {
    unsigned int bytes;
    if(
        !check_mul_uint_uint(num_digits, sizeof(unsigned char), &bytes)
        || !check_add_uint_uint(bytes, BIGINT_REDZONE * 2, &bytes)
    ) {
        errno = ENOMEM;
        return NULL;
    }
    unsigned char* p = malloc(bytes);
    memset(p, 0x42, bytes);
    return p + BIGINT_REDZONE;
}

unsigned char* okay_digits(unsigned char* digits, unsigned int num_digits) {
    unsigned char* rz1 = digits - BIGINT_REDZONE;
    unsigned char* rz2 = digits + num_digits * sizeof(unsigned char);
    for(unsigned int i = 0; i < BIGINT_REDZONE; i++) {
        if(rz1[i] != 0x42) {
            fprintf(stderr, "redzone underflow\n");
            return NULL;
        }
        if(rz2[i] != 0x42) {
            fprintf(stderr, "redzone overflow\n");
            return NULL;
        }
    }
    return rz1;
}

void free_digits(unsigned char* digits, unsigned int num_digits) {
    if(!digits) {
        return;
    }
    unsigned char* p = okay_digits(digits, num_digits);
    assert(p); // redzone violation
    free(p);
}
#else
#define malloc_digits(num_digits) malloc((num_digits) * sizeof(unsigned char))
#define okay_digits(digits,num_digits) 1
#define free_digits(digits,num_digits) free(digits)
#endif

BigInt* BigInt_construct(int value) {

    BigInt* new_big_int = malloc(sizeof(BigInt));
    if(!new_big_int) {
        return NULL;
    }
    unsigned int value2;
    if(value < 0) {
        new_big_int->is_negative = 1;
        value2 = -value;
    } else {
        new_big_int->is_negative = 0;
        value2 = value;
    }

    new_big_int->num_digits = floor(log10(value2)) + 1;

    // Special case for 0
    if(new_big_int->num_digits == 0) {
        new_big_int->num_digits = 1;
    }

    new_big_int->num_allocated_digits = new_big_int->num_digits;
    new_big_int->digits = malloc_digits(new_big_int->num_allocated_digits);
    if(!new_big_int->digits) {
        free(new_big_int);
        return NULL;
    }

    unsigned int count = new_big_int->num_digits;
    unsigned char* digits = new_big_int->digits;
    while(count--) {
        (*digits++) = value2 % 10;
        value2 /= 10;
    }

    return new_big_int;
}

BigInt* BigInt_clone(const BigInt* big_int, unsigned int num_allocated_digits) {
    if(num_allocated_digits < big_int->num_digits) {
        num_allocated_digits = big_int->num_digits;
    }
    BigInt* new_big_int = malloc(sizeof(BigInt));
    if(!new_big_int) {
        return NULL;
    }
    new_big_int->digits = malloc_digits(num_allocated_digits);
    if(!new_big_int->digits) {
        free(new_big_int);
        return NULL;
    }
    new_big_int->num_allocated_digits = num_allocated_digits;
    new_big_int->is_negative = big_int->is_negative;
    new_big_int->num_digits = big_int->num_digits;
    memmove(new_big_int->digits, big_int->digits, big_int->num_digits * sizeof(unsigned char));
    assert(okay_digits(new_big_int->digits, new_big_int->num_allocated_digits));
    return new_big_int;
}

BigInt* BigInt_from_string(const char* str) {
    BOOL is_negative = (*str == '-');
    if(is_negative) {
        str++;
    }
    while(*str == '0' && *str != 0) { // remove leading zeros
        str++;
    }
    unsigned int num_digits = strlen( str );
    BigInt* new_big_int = malloc(sizeof(BigInt));
    if(!new_big_int){
        return NULL;
    }
    new_big_int->is_negative = is_negative;
    new_big_int->num_allocated_digits = num_digits;
    new_big_int->digits = malloc_digits(num_digits);
    if(!new_big_int->digits){
        free(new_big_int);
        return NULL;
    }
    const char* end = str + num_digits - 1;
    unsigned char* digits = new_big_int->digits;
    while( end >= str ){
        unsigned char digit = *(end--);
        if(digit < '0' || digit > '9'){
            BigInt_free(new_big_int);
            errno = EINVAL;
            return NULL;
        }
        *digits++ = digit - '0';
    }
    new_big_int->num_digits = digits - new_big_int->digits;
    assert(okay_digits(new_big_int->digits, new_big_int->num_allocated_digits));
    return new_big_int;
}

void BigInt_free(BigInt* big_int) {
    if(big_int) {
        free_digits(big_int->digits, big_int->num_allocated_digits);
        free(big_int);
    }
}

BOOL BigInt_assign(BigInt* target, const BigInt* source)
{
    if(!BigInt_ensure_digits(target, source->num_digits)) {
        return 0;
    }

    memmove(target->digits, source->digits, source->num_digits * sizeof(unsigned char));

    target->is_negative = source->is_negative;
    target->num_digits = source->num_digits;
    return 1;
}

BOOL BigInt_assign_int(BigInt* target, const int source) {
    unsigned int value;
    if(value < 0) {
        target->is_negative = 1;
        value = -source;
    } else {
        target->is_negative = 0;
        value = source;
    }

    target->num_digits = floor(log10(value)) + 1;

    // Special case for 0
    if(target->num_digits == 0) {
        target->num_digits = 1;
    }

    if(!BigInt_ensure_digits(target, target->num_digits)) {
        return 0;
    }

    unsigned int count = target->num_digits;
    unsigned char* digits = target->digits;
    while(count--) {
        *(digits++) = value % 10;
        value /= 10;
    }
    return 1;
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

int BigInt_compare_int(const BigInt* a, int b) {
    int aa;
    if(!BigInt_to_int(a, &aa)) {
        // a is too big to fit into an integer:
        return a->is_negative ? -1 : 1;
    }
    if(aa == b) {
        return 0;
    }
    return aa < b ? -1 : 1;
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
    unsigned int count = a->num_digits;
    const unsigned char* pa = &a->digits[count-1];
    const unsigned char* pb = &b->digits[count-1];
    while(count--) {
        char da = *(pa--);
        char db = *(pb--);
        if(da > db) {
            return 1;
        } else if(da < db) {
            return -1;
        }
    }

    // All digits match; numbers are equal
    return 0;
}

BOOL BigInt_add(BigInt* big_int, const BigInt* addend) {
    if(big_int->is_negative == addend->is_negative) {
        // Sign will never change in this case so just leave
        // it as-is.
        if(!BigInt_add_digits(big_int, addend)) {
            return 0;
        }
    } else {
        // Figure out the sign.  Need to do this before calculating the digits of
        // the digits result because changing those in big_int will affect the result
        // of the compare.
        unsigned int result_is_negative = BigInt_compare_digits(big_int, addend) > 0 ?
                big_int->is_negative : addend->is_negative;

        if(!BigInt_subtract_digits(big_int, addend)) {
            return 0;
        }
        big_int->is_negative = result_is_negative;
    }
    return 1;
}

BOOL BigInt_add_int(BigInt* big_int, const int addend) {
    BigInt* big_int_addend = BigInt_construct(addend);
    if(!big_int_addend) {
        return 0;
    }
    BOOL result = BigInt_add(big_int, big_int_addend);
    BigInt_free(big_int_addend);
    return result;
}

BOOL BigInt_add_digits(BigInt* big_int, const BigInt* addend) {
    unsigned int digits_needed = MAX(big_int->num_digits, addend->num_digits) + 1; // TODO FIXME: this can overflow...
    if(!BigInt_ensure_digits(big_int, digits_needed)) {
        return 0;
    }

    int i;
    int carry = 0;
    for(i = 0; i < addend->num_digits || carry > 0; ++i) { // TODO FIXME: refactor to protect from integer overflow
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
    return 1;
}

BOOL BigInt_subtract(BigInt* big_int, const BigInt* to_subtract) {
    // Figure out the sign.  Need to do this before calculating the digits of
    // the digits result because changing those in big_int will affect the result
    // of the compare.
    unsigned int result_is_negative = BigInt_compare(big_int, to_subtract) >= 0 ? 0 : 1;
    
    // Calculate the digits
    if(big_int->is_negative == to_subtract->is_negative) {
        if(!BigInt_subtract_digits(big_int, to_subtract)) {
            return 0;
        }
    } else {
        if(!BigInt_add_digits(big_int, to_subtract)) {
            return 0;
        }
    }
    
    // Figure out the sign
    big_int->is_negative = result_is_negative;
    return 1;
}


BOOL BigInt_subtract_int(BigInt* big_int, const int to_subtract) {
    BigInt* big_int_to_subtract = BigInt_construct(to_subtract);
    if(!big_int_to_subtract) {
        return 0;
    }
    BOOL result = BigInt_subtract(big_int, big_int_to_subtract);
    BigInt_free(big_int_to_subtract);
    return result;
}

BOOL BigInt_subtract_digits(BigInt* big_int, const BigInt* to_subtract) {

    unsigned int digits_needed = MAX(big_int->num_digits, to_subtract->num_digits) + 1;
    if(!BigInt_ensure_digits(big_int, digits_needed)) {
        return 0;
    }

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
    return 1;
}

// Multiply using the pencil and paper method.  Complexity is O(n*m) where n, m are
// the number of digits in big_int and multiplier, respectively.
BOOL BigInt_multiply(BigInt* big_int, const BigInt* multiplier) {

    // Need to keep track of the result in a separate variable because we need
    // big_int to retain its original value throughout the course of the calculation.
    BigInt* result = BigInt_construct(0);
    if(!result) {
        return 0;
    }

    // addend will hold the amount to be added to the result for each step of
    // the multiplication.
    BigInt* addend = BigInt_construct(0);
    if(!addend) {
        BigInt_free(result);
        return 0;
    }

    unsigned int digits_needed = big_int->num_digits + multiplier->num_digits + 1;
    if(!BigInt_ensure_digits(addend, digits_needed)) {
        BigInt_free(result);
        BigInt_free(addend);
        return 0;
    }

    int i, j;
    int carry = 0;
    for(i = 0; i < multiplier->num_digits; ++i) {

        if(i > 0) {
            addend->num_digits = i;
            addend->digits[i - 1] = 0;
        }

        for(j = 0; j < big_int->num_digits || carry > 0; ++j) { // TODO FIXME: potential infinite loop
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

        if(!BigInt_add(result, addend)) {
            BigInt_free(result);
            BigInt_free(addend);
            return 0;
        }
    }

    result->is_negative = big_int->is_negative != multiplier->is_negative;

    // don't leave 0's in highest digit
    while(result->num_digits > 1 && !result->digits[result->num_digits-1]) {
        result->num_digits--;
    }

    // Place the result in big_int and clean things up
    BOOL success = BigInt_assign(big_int, result);
    BigInt_free(result);
    BigInt_free(addend);
    return success;
}

BOOL BigInt_multiply_int(BigInt* big_int, const int multiplier) {
    BigInt* big_int_multiplier = BigInt_construct(multiplier);
    if(!big_int_multiplier) {
        return 0;
    }
    BOOL result = BigInt_multiply(big_int, big_int_multiplier);
    BigInt_free(big_int_multiplier);
    return result;
}

BOOL BigInt_divide(
    BigInt* dividend, BigInt* divisor,
    BigInt* quotient, BigInt* remainder)
{
    int result = 0; // default to failure
    BigInt* div2 = NULL;
    BigInt* div3 = NULL;
    BigInt* div4 = NULL;
    BigInt* div5 = NULL;
    BigInt* div6 = NULL;
    BigInt* div7 = NULL;
    BigInt* div8 = NULL;
    BigInt* div9 = NULL;
    BigInt* ten = NULL;
    BigInt* _quotient = NULL;
    BigInt* _remainder = NULL;

    if(!BigInt_compare_int(divisor, 0)) {
        errno = ERANGE; // even BigInt can't represent infinity
        goto cleanup;
    }
    div2 = BigInt_clone(divisor, divisor->num_digits + 1);
    if(!div2 || !BigInt_multiply_int(div2, 2)) {
        goto cleanup;
    }
    div3 = BigInt_clone(divisor, divisor->num_digits + 1);
    if(!div3 || !BigInt_multiply_int(div3, 3)) {
        goto cleanup;
    }
    div4 = BigInt_clone(divisor, divisor->num_digits + 1);
    if(!div4 || !BigInt_multiply_int(div4, 4)) {
        goto cleanup;
    }
    div5 = BigInt_clone(divisor, divisor->num_digits + 1);
    if(!div5 || !BigInt_multiply_int(div5, 5)) {
        goto cleanup;
    }
    div6 = BigInt_clone(divisor, divisor->num_digits + 1);
    if(!div6 || !BigInt_multiply_int(div6, 6)) {
        goto cleanup;
    }
    div7 = BigInt_clone(divisor, divisor->num_digits + 1);
    if(!div7 || !BigInt_multiply_int(div7, 7)) {
        goto cleanup;
    }
    div8 = BigInt_clone(divisor, divisor->num_digits + 1);
    if(!div8 || !BigInt_multiply_int(div8, 8)) {
        goto cleanup;
    }
    div9 = BigInt_clone(divisor, divisor->num_digits + 1);
    if(!div9 || !BigInt_multiply_int(div9, 9)) {
        goto cleanup;
    }

    ten = BigInt_construct( 10 );
    _quotient = BigInt_construct( 0 );
    _remainder = BigInt_construct( 0 );
    if(!ten || !_quotient || !_remainder) {
        goto cleanup;
    }

    BigInt* divs[10];
    // NOTE: divs[0] intentionally unused so that new_digit == index below
    divs[1] = divisor;
    divs[2] = div2;
    divs[3] = div3;
    divs[4] = div4;
    divs[5] = div5;
    divs[6] = div6;
    divs[7] = div7;
    divs[8] = div8;
    divs[9] = div9;

    const unsigned char* base = dividend->digits;
    const unsigned char* digits = &base[dividend->num_digits-1];

    while(digits >= base) {
        if(!BigInt_multiply(_remainder, ten)) {
            goto cleanup;
        }
        if(!BigInt_add_int(_remainder, *digits)) {
            goto cleanup;
        }
        int new_digit = 0;
        for(int i = 9; i >= 1; i--) {
            if(BigInt_compare(_remainder, divs[i]) >= 0) {
                if(!BigInt_subtract(_remainder, divs[i])) {
                    goto cleanup;
                }
                new_digit = i;
                break;
            }
        }
        if(!BigInt_multiply(_quotient, ten) || !BigInt_add_int(_quotient, new_digit)) {
            goto cleanup;
        }
        digits--;
    }
    
    if(quotient) {
        if(!BigInt_assign(quotient, _quotient)) {
            goto cleanup;
        }
    }
    if(remainder) {
        if(!BigInt_assign(remainder, _remainder)) {
            goto cleanup;
        }
    }
    
    result = 1;
cleanup:
    BigInt_free(div2);
    BigInt_free(div3);
    BigInt_free(div4);
    BigInt_free(div5);
    BigInt_free(div6);
    BigInt_free(div7);
    BigInt_free(div8);
    BigInt_free(div9);
    BigInt_free(ten);
    BigInt_free(_remainder);
    BigInt_free(_quotient);
    return result;
}

BOOL BigInt_to_int(const BigInt* big_int, int* value) {
    *value = 0;
    int tens_multiplier = 1;

    unsigned int num_digits = big_int->num_digits;
    const unsigned char* digits = big_int->digits;
    
    // don't process any most significant digits that happen to be zero
    // (avoids unnecessary tens_multiplier overflow)
    while(num_digits && !digits[num_digits-1]) {
        num_digits--;
    }

    // prefill value so that we can avoid an unnecessary tens_multiplier overflow
    if(num_digits) {
        *value = *(digits++);
        num_digits--;
    }
    while(num_digits--) {
        int digit = *(digits++);
        if(
            !check_mul_int_int(tens_multiplier, 10, &tens_multiplier)
            || !check_mul_int_int(digit, tens_multiplier, &digit)
            || !check_add_int_int(*value, digit, value)
        ) {
            errno = ERANGE;
            return 0;
        }
    }

    if (big_int->is_negative) {
        if(!check_mul_int_int(*value, -1, value)) {
            errno = ERANGE;
            return 0;
        }
    }

    return 1;

}

void BigInt_print(const BigInt* big_int) {
    BigInt_fprint(stdout, big_int);
}

void BigInt_fprint(FILE *dest, const BigInt* big_int) {
    const unsigned char* base = big_int->digits;
    const unsigned char* digits = &base[big_int->num_digits-1];
    if (big_int->is_negative) fputc('-', dest);
    while(digits >= base) {
        fputc('0' + *(digits--), dest);
    }
}

unsigned int BigInt_strlen(const BigInt* big_int){
    unsigned int len = big_int->num_digits;
    if( big_int->is_negative ){
        len++;
    }
    return len;
}

BOOL BigInt_to_string(const BigInt* big_int, char* buf, unsigned int buf_size){
    const unsigned char* base = big_int->digits;
    const unsigned char* digits = &base[big_int->num_digits-1];
    if (big_int->is_negative){
        if(!buf_size--){
            errno = ERANGE;
            return 0;
        }
        *buf++ = '-';
    }

    while( digits >= base ){
        if(!buf_size--){
            errno = ERANGE;
            return 0;
        }
        *buf++ = '0' + *(digits--);
    }

    // write 0 terminator:
    if(!buf_size--){
        errno = ERANGE;
        return 0;
    }
    *buf++ = 0;

    return 1;
}

char* BigInt_to_new_string(const BigInt* big_int){
    unsigned int buf_size = BigInt_strlen(big_int) + 1;
    char* buf = malloc(buf_size);
    if(!buf) {
        return NULL;
    }
    BOOL result = BigInt_to_string(big_int, buf, buf_size);
    assert(result);
    return buf;
}

BOOL BigInt_ensure_digits(BigInt* big_int, unsigned int digits_needed) {
    if(big_int->num_allocated_digits < digits_needed) {
        assert(okay_digits(big_int->digits, big_int->num_allocated_digits));
        unsigned char* new_digits = malloc_digits(digits_needed);
        if(!new_digits) {
            return 0;
        }
        assert(okay_digits(new_digits, digits_needed));
        unsigned int old_allocated = big_int->num_allocated_digits;
        unsigned char* old_digits = big_int->digits;
        memcpy(new_digits, old_digits, big_int->num_digits * sizeof(unsigned char));
        big_int->digits = new_digits;
        big_int->num_allocated_digits = digits_needed;

        free_digits(old_digits, old_allocated);
        assert(okay_digits(big_int->digits, big_int->num_allocated_digits));
    }
    return 1;
}

