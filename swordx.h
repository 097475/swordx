typedef struct Trie {
char *value;
int occurrencies;
struct Trie *children[26];
}Trie;
