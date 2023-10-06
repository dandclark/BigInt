#include <assert.h>
#include <stdio.h>

#include "BigInt.h"

// Demo of some basic BigInt functionality

int main() {
    // Obtain a BigInt initialized to 42
    BigInt* new_big_int = BigInt_construct(42); assert(new_big_int);
    
    // Get a normal int back (the BigInt must fit in an int type)
    int as_int;
    assert(BigInt_to_int(new_big_int, &as_int));

    // Print to stdout with BigInt_print 
    printf("BigInt is: ");
    BigInt_print(new_big_int);
    printf("\nAs int: %i\n", as_int);

    // The caller is responsible for freeing a BigInt allocated with
    // BigInt_construct with a call to BigInt_free:
    BigInt_free(new_big_int);

    // BigInt operations take two BigInt parameters and place the result in the first parameter:
    BigInt* a = BigInt_construct(15); assert(a);
    BigInt* b = BigInt_construct(-20); assert(b);
    assert(BigInt_add(a, b));
    printf("Addition result: ");
    BigInt_print(a); // Prints -5
    printf("\n");

    FILE *test = fopen("test.dat", "w");
    
    BigInt_fprint(test, a);
    fprintf(test, "\n");

    fclose(test);

    // The exception is BigInt_compare; this takes two BigInt parameters, changes neither, and returns the value of the comparison:
    assert(BigInt_assign_int(a, 15));
    assert(BigInt_assign_int(b, -20));
    printf("Comparison results:\n");
    printf("%i\n", BigInt_compare(a, b)); // prints 1
    printf("%i\n", BigInt_compare(b, a)); // prints -1
    printf("%i\n", BigInt_compare(a, a)); // prints 0

    BigInt_free(a);
    BigInt_free(b);

    return 0;
}



