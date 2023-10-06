
#include <stdio.h>

#include "BigInt.h"
#include "BigInt_test.h"

// Run BigInt unit tests

int main() {

    BigInt_test_basic();
    BigInt_test_big_multiplication();
    printf("testing finished\n");

    return 0;
}



