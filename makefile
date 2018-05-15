all: swordx
swordx: swordx.o trie.o
	gcc -o swordx swordx.o trie.o
swordx.o: swordx.c trie.c
	gcc -c swordx.c
	gcc -c trie.c
clean:
	rm swordx swordx.o trie.o