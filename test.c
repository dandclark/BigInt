
#include <stdio.h>

#define _DEBUG
#include "CMemLeak.h"

#include "BigInt.h"

int main() {

    printf("Hello, world!\n");
    printf("Size: %i\n", sizeof(struct BigInt));


    BigInt* big_int;
    big_int = BigInt_construct(42);

    BigInt_add(big_int, big_int);

    printf("Number is: ");
    BigInt_print(big_int);
    printf("\n");

    int as_int = BigInt_to_int(big_int);
    printf("Number as int: %i\n", as_int);

    BigInt_test_basic();

    BigInt_free(big_int); 

    return 0;
}



