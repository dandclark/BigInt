
#include <stdio.h>

#include "BigInt.h"
#include "BigInt_test.h"

// Run BigInt unit tests

int main() {

    BigInt_test_basic();
    BigInt_test_big_multiplication();
    BigInt_test_print();
    printf("testing finished\n");

    return 0;
}



