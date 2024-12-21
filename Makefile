CC=clang

CFLAGS=-Wall

# default build
all: main.c matheval.c matheval.h
	$(CC) $(CFLAGS) main.c matheval.c -o matheval
	$(CC) $(CFLAGS) matheval-test.c matheval.c -o matheval-test
	@echo "Running tests:"
	./matheval-test

# install matheval into /usr/local/bin
install:
	$(CC) $(CFLAGS) main.c matheval.c -o matheval
    ifeq ($(wildcard matheval-test), matheval-test)
	    rm -f matheval-test
    endif
	mv -i math-eval /usr/local/bin/matheval

# removes matheval from /usr/local/bin
clean:
	rm -f /usr/local/bin/matheval

.PHONY: all install clean
