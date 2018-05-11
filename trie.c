#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

void addRecord ( char* , Trie* , int );
void _add ( char* , Trie* , int );

void add (char* str, Trie *t) {
	_add(str,t,1);
}

void _add (char* str, Trie *t, int level) {
	int flag = 0;
	Trie *next = t->children[str[level-1]-'a']; // next is the next char of str (the root in a trie is empty)
	
	// if the childern is initialized and the next node has the same value of the substring... 
	if(next != NULL && strncmp(str,next->value,level) == 0) {
		if(level == strlen(str)) // .. if it's the whole word, increase the next's occurrencies
			next->occurrencies++;
		else { // .. else check the next level
			level++;
			_add(str,next,level);
		}

		flag = 1;
	}
	if(flag == 0) // if there's no match, insert a new record
		addRecord(str,t,level);
}


void addRecord(char* str, Trie *t, int level) {
	Trie *n = (Trie*)malloc(sizeof(Trie));
	if(t->children[str[level - 1] - 'a'] == NULL) {
		n->value = (char*)malloc(level + 1);
		snprintf(n->value, level + 1, "%s", str); //space for null character
		
        for(int j = 0; j < 26; j++)
			n->children[j] = NULL;
		n->occurrencies = 0;
		t->children[str[level - 1] - 'a'] = n;
	}
    if(level == strlen(str)) {
		n->occurrencies++;
        free(str);
	}
    else {
		level++;
		addRecord(str,n,level);
	}
}


void printall(Trie *t) {
	if(t != NULL) {
		printf("value: %s\toccurrences: %d\n", t->value, t->occurrencies);
		for(int i = 0; i < 26; i++)
			printall(t->children[i]);
	}
}

Trie* createTree() {
	Trie *t = (Trie*)malloc(sizeof(Trie));
	t->value = "";
	t->occurrencies = 0;
	for(int i = 0; i < 26; i++)
		t->children[i] = NULL;
	return t;
}
