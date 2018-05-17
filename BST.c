#include <stdlib.h>
#include <stdio.h>
#include "trie.h"


void addBST(BST **b, Trie *t){
	
	if(*b==NULL)
	{
		*b=(BST*) malloc(sizeof(BST));
		(*b)->word = t;
		(*b)->left = NULL;
		(*b)->right = NULL;
	}
	else
	{
		if(t->occurrencies < ((*b)->word)->occurrencies)
		{
			addBST(&(*b)->left,t);
		}
		else
		{
			addBST(&(*b)->right,t);
		}
	}
}

void printBST(BST *b)
{
	if(b!=NULL)
	{
		printBST(b->left);
		printf("%s\n",b->word->value);
		printBST(b->right);
	}
}


