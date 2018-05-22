all: swordx
swordx: swordx.o trie.o stack.o BST.o
	gcc -o swordx swordx.o trie.o stack.o BST.o
swordx.o: swordx.c trie.c stack.c BST.c
	gcc -c swordx.c trie.c stack.c BST.c
clean:
	rm swordx swordx.o trie.o stack.o BST.o
