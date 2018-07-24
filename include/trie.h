#ifndef TRIE_H
#define TRIE_H

#define CHARSET 36

typedef struct Trie {
	char *value;
	long occurrencies;
	struct Trie *children[CHARSET];
} Trie;


void add ( char* , Trie* );
void printall ( Trie* );
Trie* createTree ( void );
int search(char*,Trie*);
void writeTrie ( Trie* , FILE* );
void writeNodeInformation ( Trie* , FILE* );

#endif
