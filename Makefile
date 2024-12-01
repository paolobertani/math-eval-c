CC=clang

CFLAGS=-Wall

# default build
all: main.c math-eval.c math-eval.h
	$(CC) $(CFLAGS) main.c math-eval.c -o math-eval
	$(CC) $(CFLAGS) math-eval-test.c math-eval.c -o math-eval-test
	@echo "Running tests:"
	./math-eval-test

# install math-eval into /usr/local/bin
install:
	$(CC) $(CFLAGS) main.c math-eval.c -o math-eval
    ifeq ($(wildcard math-eval-test), math-eval-test)
	    rm -f math-eval-test
    endif
	mv -i math-eval /usr/local/bin/math-eval

# removes math-eval from /usr/local/bin
clean:
	rm -f /usr/local/bin/math-eval

.PHONY: all install clean
