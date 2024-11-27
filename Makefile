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
	ifndef SUDO_UID
		$(error `install` must be run with sudo)
	endif
	$(CC) $(CFLAGS) main.c math-eval.c -o math-eval
	mv -i math-eval /usr/local/bin/math-eval

# removes math-eval from /usr/local/bin
clean:
	rm -f /usr/local/bin/math-eval

.PHONY: all install clean
