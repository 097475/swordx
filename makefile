all: swordx
swordx: swordx.o trie.o stack.o BST.o ThreadIdStack.o
	gcc -pthread -o swordx swordx.o trie.o stack.o BST.o ThreadIdStack.o
swordx.o: swordx.c trie.c stack.c BST.c ThreadIdStack.c
	gcc -c swordx.c trie.c stack.c BST.c ThreadIdStack.c
clean:
	rm swordx swordx.o trie.o stack.o BST.o ThreadInStack.o

