all: test demo

test: BigInt.o BigInt_test.o test.o
	gcc -lm -o test BigInt.o BigInt_test.o test.o

demo: demo.o BigInt.o
	gcc -lm -o demo demo.c BigInt.o
	    
BigInt.o: BigInt.c BigInt.h
	gcc -c BigInt.c

test.o: test.c
	gcc -c test.c

demo.o: demo.c
	gcc -c test.c

BigInt_test.o: BigInt_test.c
	gcc -c BigInt_test.c

clean: 
	rm -f *.o test demo

