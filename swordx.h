#define RECURSE_FLAG (1<<0) //00000001
#define FOLLOW_FLAG (1<<1) //00000010
#define ALPHA_FLAG (1<<2) //00000100
#define SBO_FLAG (1<<3) //00001000
#include "trie.h"
#include "stack.h"

char* getWord ( FILE* , int , Trie* , unsigned char );
FILE* open_file ( char* );
FILE* makeFile ( char * );
void orderedWrite ( BST *b, FILE *pf );
void sortTrie ( Trie* , BST** );
void scan ( char* , Stack* , unsigned char , char* );
