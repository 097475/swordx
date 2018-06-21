#include <stdlib.h>
#include <stdio.h>
#include "BST.h"

BST** createBST()
{
	BST **b = (BST**)malloc(sizeof(BST*));
	if(b == NULL) {
		perror("No more heap space");
		exit(EXIT_FAILURE);
	}
	*b = NULL;
	return b;
}

void addBST(BST **b, Trie *t) {
	if(*b == NULL) {
		*b = (BST*) malloc(sizeof(BST));
		if(b == NULL) {
		perror("No more heap space");
		exit(EXIT_FAILURE);
	}
		(*b)->wordInfo	= t;
		(*b)->left	= NULL;
		(*b)->right	= NULL;
	}
	else
		if(t->occurrencies < ((*b)->wordInfo)->occurrencies)
			addBST(&(*b)->left,t);
		else
			addBST(&(*b)->right,t);
}

void writeTree(BST *b, FILE *pf) {
	if(b != NULL) {
		writeTree(b->right,pf);
		writeNodeInformation(b->wordInfo,pf);
		writeTree(b->left,pf);
	}
}

void printBST(BST *b) {
	if(b!=NULL) {
		printBST(b->left);
		printf("%s\n",b->wordInfo->value);
		printBST(b->right);
	}
}
