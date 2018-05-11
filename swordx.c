#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include "trie.h"

static int cmpstringp ( const void* , const void* );
int isInArray ( char** , long , char* );

FILE* open_file(char* path) {
	FILE* pf = fopen(path,"rb");
	if(pf == NULL) {
		perror("Error reading");
		exit(EXIT_FAILURE);
	}
	return pf;	
}

char* getWord(FILE *pf, char *blacklist[], int blc) {
	char c, buf[500]; // c is the character, buf is the word
	int pos = 0; // index
	
	while(!isalpha(c = getc(pf)) && c != EOF); // remove all not-alphabetic character before a string
	ungetc(c,pf); // put the last char back

	while(isalpha((c = getc(pf)))) {
		buf[ pos++ ] = tolower(c);
		if(pos == 500) {
			fprintf(stderr,"Word is too long\n");
			exit(EXIT_FAILURE);
		}
	}
	if(c == EOF) return NULL;
	char *word = (char*)malloc(pos*sizeof(char));
	strncpy(word,buf,pos);
	word[pos]='\0';
	
	if(!isInArray(blacklist,blc,word))
		return word;
	free(word);
	return getWord(pf,blacklist,blc);
}


FILE* makeFile() {
	FILE *pf = fopen("result.txt","wb");
	if(pf == NULL) {
		perror("Error writing");
		exit(EXIT_FAILURE);
	}
	return pf;
}

void writeWord(Trie *t,FILE *pf) {
	char *word = t->value;
	fwrite(word,sizeof(char),strlen(word),pf);
	fputs(": ",pf);
	putc(t->occurrencies+'0',pf);
	fputs("\r\n",pf);
}

void visitTree(Trie *t, FILE *pf) {
	if(t == NULL) return;
	if(t->occurrencies > 0) // if the word occurrs at least one time...
		writeWord(t,pf); // ... write the word in the file
	for(int i = 0; i < 26; i++)
		visitTree(t->children[i],pf); // check each children of the node
}

void execute(char *src, char *blacklist[], int blc) {
	FILE *pfread = open_file(src);
	Trie *t = createTree();
	char *str;
	if(blc > 1) qsort(blacklist, blc, sizeof(char*),cmpstringp);
	while((str = getWord(pfread,blacklist,blc)) != NULL)
		add(str,t); // add the word to the trie (starting from the 1st level)
	FILE *pfwrite = makeFile();
	visitTree(t,pfwrite); // write trie status
	fclose(pfread);
	fclose(pfwrite);
}

int main(int argc, char *argv[]) {
	char *arr[] = {"one","two","three","four","five","six","seven"};	
	char *src;
	int option_index = 0;
	int opt; // optaaantains the getopt_long return value (the index of long_options array
    static struct option long_options[] = { // "--comand" , argument required? , X , opt result
		{ "exclude",     required_argument, 0,  0 }
    };

	while ((opt = getopt_long(argc, argv, "f:",long_options,&option_index)) != -1) // read all the args, for each arg...
		switch(opt) { // ... switch it
			case 0:
				//printf("Option %s\n\tArgs: %s\n", long_options[option_index].name,optarg);
				printf("Blacklist\n\t%s\n", optarg);
				//arr = stringToArray(optarg);
				break;
			case 'f':
				printf("Reading from file(s)\t%s\n",optarg);
				src = optarg;
				break;
		}
	
	execute(src,arr,sizeof(arr)/sizeof(*arr)); // optarg contains the filename
	//execute(optarg,NULL,0);
}

int isInArray(char *array[], long length, char *str) {
	char **key = &str;
	return bsearch(key, array, length, sizeof(array[0]), cmpstringp) != NULL;
}

static int cmpstringp(const void *p1, const void *p2) { // from "man qsort": the qsort method accepts void pointers only
	return (strcmp(*(char **)p1, *(char **)p2));
}
