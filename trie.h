typedef struct Trie {
	char *value;
	long occurrencies;
	struct Trie *children[26];
} Trie;

void add ( char* , Trie* );
void printall ( Trie* );
Trie* createTree ( void );
