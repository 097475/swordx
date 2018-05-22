#define CHARSET 42

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

typedef struct BST{
Trie *word;
struct BST *left;
struct BST *right;
}BST;


void addBST(BST **b, Trie *t);
void printBST(BST *b);
