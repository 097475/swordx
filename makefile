all: swordx
swordx: swordx.o trie.o stack.o BST.o ThreadInStack.o
	gcc -o swordx swordx.o trie.o stack.o BST.o ThreadInStack.o
swordx.o: swordx.c trie.c stack.c BST.c ThreadInStack.c
	gcc -c swordx.c trie.c stack.c BST.c ThreadInStack.c
clean:
	rm swordx swordx.o trie.o stack.o BST.o ThreadInStack.o
