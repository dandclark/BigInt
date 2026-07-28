CC ?= gcc
# TODO add -Wall -Wextra to CFLAGS
CFLAGS ?= -g
LDFLAGS ?=
LDLIBS ?= -lm

.PHONY: all check clean asan-test ubsan-test sanitize-test

all: test demo

test: BigInt.o BigInt_test.o test.o
	$(CC) $(LDFLAGS) -o test BigInt.o BigInt_test.o test.o $(LDLIBS)

check : test
	./test

demo: demo.o BigInt.o
	$(CC) $(LDFLAGS) -o demo demo.o BigInt.o $(LDLIBS)

BigInt.o: BigInt.c BigInt.h
	$(CC) $(CFLAGS) -c BigInt.c

test.o: test.c
	$(CC) $(CFLAGS) -c test.c

demo.o: demo.c
	$(CC) $(CFLAGS) -c demo.c

BigInt_test.o: BigInt_test.c BigInt_test.h
	$(CC) $(CFLAGS) -c BigInt_test.c

# Rebuild the test suite from scratch under a sanitizer and run it.  A clean is
# required because the object files must be (re)compiled with sanitizer flags,
# which are passed to BOTH the compile and link steps via CFLAGS and LDFLAGS.
asan-test:
	$(MAKE) clean
	$(MAKE) test \
		CFLAGS="$(CFLAGS) -fsanitize=address -fno-omit-frame-pointer" \
		LDFLAGS="$(LDFLAGS) -fsanitize=address"
	./test

ubsan-test:
	$(MAKE) clean
	$(MAKE) test \
		CFLAGS="$(CFLAGS) -fsanitize=undefined -fno-omit-frame-pointer" \
		LDFLAGS="$(LDFLAGS) -fsanitize=undefined"
	UBSAN_OPTIONS=print_stacktrace=1 ./test

# Convenience target that runs ASan and UBSan together in a single build.
sanitize-test:
	$(MAKE) clean
	$(MAKE) test \
		CFLAGS="$(CFLAGS) -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer" \
		LDFLAGS="$(LDFLAGS) -fsanitize=address,undefined"
	UBSAN_OPTIONS=print_stacktrace=1 ./test

clean:
	rm -f *.o test demo

