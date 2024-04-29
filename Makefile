CFLAGS=-Wall
NUM_TESTS=6

path-finder: src/*.c
	gcc $(CFLAGS) src/main.c -o path-finder 

.PHONY: test
test:
	make path-finder	
	bash tests/test.sh $(NUM_TESTS)
	make clean

.PHONY: clean
clean:
	rm path-finder