#include "trie.h"
typedef struct BST{
	Trie *wordInfo;
	struct BST *left;
	struct BST *right;
}BST;


void addBST(BST **b, Trie *t);
void printBST(BST *b);
void writeTree(BST *b, FILE *pf);
BST** createBST();

