# BigInt

C library for operations with signed integers of arbitrary size.

## Introduction

BigInt supports various basic mathematical operations: addition, subtraction, multiplication, and comparison.  The primary use case is when very large numbers are needed; BigInts can contain up to the number of digits as the maximum value of an unsigned long (typically 2^32 - 1 on a 32-bit machine).

A BigInt is a struct with the following fields:
* digits -- An array of digits 0-9.  The lowest index is the least significant digit.
* num_digits -- The number of digits in the digits array.
* is_negative -- Nonzero if the BigInt is negative, zero if the BigInt is positive.
* num_allocated_digits -- The amount of space allocated for digits; caller doesn't need to care about this.

## Usage

Obtain a pointer to a new BigInt through a call to BigInt_construct:
```
BigInt* new_big_int = BigInt_construct(42); // Obtains a BigInt initialized to 42
```

The caller is responsible for freeing a BigInt allocated with BigInt_construct with a call to BigInt_free:
```
BigInt_free(new_big_int);
```

The contents of a BigInt can be printed to stdout with a call to BigInt_print.
```
BigInt_print(big_int);
```

Get a normal int back from a BigInt with using BigInt_to_int; but be careful; if the BigInt doesn't fit in an int type, the function will return 0 and the value in the int argument will be undefined.
```
int a;
assert(BigInt_to_int(big_int, &a));
```

BigInt operations take two BigInt parameters and place the result in the first parameter:
```
BigInt* a = BigInt_construct(15);
BigInt* b = BigInt_construct(-20);
BigInt_add(a, b);
BigInt_print(a); // Prints -5
```

The exception is BigInt_compare; this takes two BigInt parameters, changes neither, and returns the value of the comparison:
```
BigInt a = BigInt_construct(15);
BigInt b = BigInt_construct(-20);
printf("%i\n", BigInt_compare(a, b)); // prints 1
printf("%i\n", BigInt_compare(b, a)); // prints -1
printf("%i\n", BigInt_compare(a, a)); // prints 0
```
