CC     = cc
CFLAGS = -Wall -O3 -DSHAKE_TEST=1

test:
	$(CC) $(CFLAGS) -o shake shake.c

clean:
	rm -f shake

.PHONY: test clean
