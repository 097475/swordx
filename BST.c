#include <stdlib.h>
#include <stdio.h>
#include "BST.h"


void addBST(BST **b, Trie *t) {
	if(*b == NULL) {
		*b = (BST*) malloc(sizeof(BST));
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
		writeTree(b->left,pf);
		writeNodeInformation(b->wordInfo,pf);
		writeTree(b->right,pf);
	}
}

void printBST(BST *b) {
	if(b!=NULL) {
		printBST(b->left);
		printf("%s\n",b->wordInfo->value);
		printBST(b->right);
	}
}
