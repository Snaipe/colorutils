CC=gcc
CFLAGS=-std=c99 -Wall -Wextra -Werror

colorset: colorset.o
	$(LINK.o) -o $@ $< -lm

clean:
	$(RM) colorset colorset.o

.PHONY: clean
