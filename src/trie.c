#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "trie.h"

int getIndex(char c);
void addRecord ( char* , Trie* , int );
void _add ( char* , Trie* , int );
int _search(char * , Trie * , int );

int getIndex(char c) { 
	if(isdigit(c))
		return c-'0';
	else
		return c - 'a' + 10;
}

int _search(char *str, Trie *t, int level) { 
		Trie *next = t->children[getIndex(str[level - 1])];
		if(next!=NULL)
			if(level == strlen(str)) 
				return 1;
			else { 
				level++;
				return _search(str,next,level);
			}
		else
			return 0;
}

int search(char *str, Trie *t) {
	return (str==NULL) ? 0 : _search(str,t,1);
}

void addRecord(char* str, Trie *t, int level) {
	Trie *n = (Trie*)malloc(sizeof(Trie));
	if(n == NULL) {
		perror("No more heap space");
		exit(EXIT_FAILURE);
	}

	n->value = (char*)malloc((level+1)*sizeof(char));
	if(n->value == NULL) {
		perror("No more heap space");
		exit(EXIT_FAILURE);
	}
	strncpy(n->value,str,level);
	n->value[level] = '\0';	
	
    for(int j = 0; j < CHARSET; j++)
		n->children[j] = NULL;
		
	n->occurrencies = 0;
	t->children[getIndex(str[level - 1])] = n;

    if(level == strlen(str)) {
		n->occurrencies++;
        free(str);
	}
    else {
		level++;
		addRecord(str,n,level);
	}
}

void _add (char* str, Trie *t, int level) {
	int flag = 0;
	Trie *next = t->children[getIndex(str[level - 1])]; // next is the next char of str (the root in a trie is empty)
	
	// if the childern is initialized and the next node has the same value of the substring... 
	if(next != NULL) {
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

void add (char* str, Trie *t) {
	_add(str,t,1);
}

void writeNodeInformation(Trie *t, FILE *pf) {
	fprintf(pf,"%s: %lu\r\n", t->value, t->occurrencies);
}

void writeTrie(Trie *t, FILE *pf) {
	if(t == NULL) return;
	if(t->occurrencies > 0) // if the word occurrs at least one time...
		writeNodeInformation(t,pf); // ... write the word in the file
	for(int i = 0; i < CHARSET; i++)
		writeTrie(t->children[i],pf); // check each children of the node
}

void printall(Trie *t) {
	if(t != NULL) {
		printf("value: %s\toccurrences: %lu\n", t->value, t->occurrencies);
		for(int i = 0; i < CHARSET; i++)
			printall(t->children[i]);
	}
}

Trie* createTree() {
	Trie *t = (Trie*)malloc(sizeof(Trie));
	if(t == NULL) {
		perror("No more heap space");
		exit(EXIT_FAILURE);
	}
	t->value = "";
	t->occurrencies = 0;
	for(int i = 0; i < CHARSET; i++)
		t->children[i] = NULL;
	return t;
}
