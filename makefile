all: swordx
swordx: swordx.o trie.o stack.o
	gcc -o swordx swordx.o trie.o stack.o
swordx.o: swordx.c trie.c stack.c
	gcc -c swordx.c trie.c stack.c
clean:
	rm swordx swordx.o trie.o stack.o
