bin/nfcycler: src/nfcycler.c
	gcc -std=gnu99 -o $@ $^ -Wall -Wextra

examples/sequence: examples/sequence.c
	gcc -std=gnu99 -o $@ $^ -Wall -Wextra
