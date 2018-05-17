
typedef struct BST{
Trie *word;
struct BST *left;
struct BST *right;
}BST;

BST* createNode();
void addBST(BST *b, Trie *t);
void printBST(BST *b);
