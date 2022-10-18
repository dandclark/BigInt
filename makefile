all: test demo

test: BigInt.o BigInt_test.o test.o
	gcc -lm -g -o test BigInt.o BigInt_test.o test.o

demo: demo.o BigInt.o
	gcc -lm -g -o demo demo.c BigInt.o
	    
BigInt.o: BigInt.c BigInt.h
	gcc -g -c BigInt.c

test.o: test.c
	gcc -g -c test.c

demo.o: demo.c
	gcc -g -c test.c

BigInt_test.o: BigInt_test.c BigInt_test.h
	gcc -g -c BigInt_test.c

clean: 
	rm -f *.o test demo

