#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include "swordx.h"

static int cmpstringp ( const void* , const void* );
int isInArray ( char** , long , char* );

char* _getWord(FILE *pf) {
	char c, buf[500]; // c is the character, buf is the word
	int pos = 0; // index
	
	while(!isalnum(c = getc(pf)) && strchr("אטילעש",c)==NULL && c != EOF); // remove all not-alphabetic character before a string
	ungetc(c,pf); // put the last char back

	while(isalnum(c = getc(pf)) || strchr("אטילעש",c)!=NULL) {
		buf[ pos++ ] = tolower(c);
		if(pos == 500) {
			fprintf(stderr,"Word is too long\n");
			exit(EXIT_FAILURE);
		}
	}
	if(c == EOF) return NULL;
	char *word = (char*)malloc((pos+1)*sizeof(char));
	strncpy(word,buf,pos);
	word[pos]='\0';
	return word;
}

char* getWord(FILE *pf, char *blacklist[], int blc) {
	char *ret = NULL;
	do {
		if(ret != NULL) free(ret);
		ret = _getWord(pf);
	} while(ret != NULL && isInArray(blacklist,blc,ret));
	return ret;
}

FILE* open_file(char* path) {
	FILE* pf = fopen(path,"rb");
	if(pf == NULL) {
		perror("Error reading");
		exit(EXIT_FAILURE);
	}
	return pf;	
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
	fprintf(pf,"%s: %lu\r\n",t->value,t->occurrencies);
}

void sbo(Trie *t, FILE *pf)
{
	BST **b = (BST**)malloc(sizeof(BST*));
	sortTrie(t,b);
	orderedPrint(*b,pf);
}

void sortTrie(Trie *t, BST **b){	
	if(t == NULL) return;
	if(t->occurrencies > 0) 
	{
		//printf("%s\n",t->value);
		addBST(b,t); 
	}

	for(int i = 0; i < 42; i++)
		sortTrie(t->children[i],b);	
}

void orderedPrint(BST *b, FILE *pf) {
	if(b != NULL){
		orderedPrint(b->left,pf);
		writeWord(b->word,pf);
		orderedPrint(b->right,pf);
	}

}
void visitTree(Trie *t, FILE *pf) {
	if(t == NULL) return;
	if(t->occurrencies > 0) // if the word occurrs at least one time...
		writeWord(t,pf); // ... write the word in the file
	for(int i = 0; i < CHARSET; i++)
		visitTree(t->children[i],pf); // check each children of the node
}

/*void execute(stack* s, char** args, unsigned char flags) {
	
	Trie *t = createTree();
	if(blc > 1) qsort(blacklist, blc, sizeof(char*),cmpstringp);
			
	char *str;
	for(int i=0;i<nfiles;i++) {
		FILE *pfread = open_file(files[i]);	
		while((str = getWord(pfread,blacklist,blc)) != NULL)
			add(str,t); // add the word to the trie (starting from the 1st level)
		fclose(pfread);		
	}

	FILE *pfwrite = makeFile();
	visitTree(t,pfwrite); // write trie status
	fclose(pfwrite);
}*/

void writeHelp() {
	printf("swordx [options] [inputs]\n");
	printf("   swordx counts the occurrencies of each words (alphanumeric characters by default) and print them into an output file.\n");
	printf("\n");
	printf("   [inputs]\n");
	printf("      the file and/or directory to process\n");
	printf("   [options]\n");
	printf("      -h | --help : display this message\n");
	printf("\n");
	printf("      --output <filename> : write the result in a spacific file (<filename>)\n");
	printf("         by default the file is named \"sword.out\"\n");
	printf("\n");
	printf("      -r | --recursive : go through all the passed directories recursivly\n");
	printf("      -f | --follow : follow links\n");
	printf("      -e | --explude : exclude a file (if -r is enable)\n");
	printf("\n");
	printf("      -a | --alpha : consider alphabetics letters only\n");
	printf("      -m <num> | --min <num> : consider words with at least <num> letters\n");
	printf("      -i <file> | --ignore <file> : all words contained in <file> are ignored (one per line)\n");
	printf("\n");
	printf("      --sordbyoccurrency | -sbo : sort words by occurrencies in the output file\n");
}

int main(int argc, char *argv[]) {	
	int opt = 0, nparams = 0;
	char *explude = NULL,*ignore = NULL,*output = NULL, *min = NULL, **args, **params;
	unsigned char flags = 0; //1 byte integer field
	
    struct option long_options[] = {
		{"help",    no_argument, 0,  0 },
		{"recursive",  no_argument, 0,  1},
		{"follow",  no_argument, 0,  2 },
		{"explude", required_argument, 0,  3 },
		{"alpha",  no_argument, 0,  4 },
		{"min",    required_argument, 0,  5 },
		{"ignore",  required_argument, 0,  6 },
		{"sortbyoccurrency",  no_argument, 0,  7 },
		{"sbo",  no_argument, 0,  7 },
		{"output",  required_argument, 0,  8 },
		{ NULL, 0, NULL, 0 }  //required
	};
	
	while ((opt = getopt_long_only(argc, argv, "hrfe:am:i:",long_options,NULL)) != -1)
	{
		switch(opt)
		{
			case 0:
			case 'h': writeHelp(); break;
			case 1:
			case 'r': 
					flags |= RECURSE_FLAG;
					break;
			case 2:
			case 'f':					
					flags |= FOLLOW_FLAG;
					break;
			case 3:
			case 'e':
					explude = (char*)malloc((strlen(optarg)+1)*sizeof(char));
					strcpy(explude,optarg);
					break;
			case 4:
			case 'a': 					
					flags |= ALPHA_FLAG;
					break;
			case 5:
			case 'm': 
					min = (char*)malloc((strlen(optarg)+1)*sizeof(char));
					strcpy(min,optarg);
					break;
			case 6:
			case 'i': 
					ignore = (char*)malloc((strlen(optarg)+1)*sizeof(char));
					strcpy(ignore,optarg);
					break;
			case 7: 					
					flags |= SBO_FLAG;
					break;
			case 8: 
					output = (char*)malloc((strlen(optarg)+1)*sizeof(char));
					strcpy(output,optarg);
					break;
			default: printf("default case");break;
		}
	}
	
	args = (char**)malloc(4*sizeof(char*));
	args[0] = explude;
	args[1] = min;
	args[2] = ignore;
	args[3] = output;
	
	
	nparams = argc-optind;
	params = (char**)malloc(nparams*sizeof(char*));
	for(int i = optind; i<argc; i++)
	{
		params[i] = (char*)malloc((strlen(argv[i])+1) * sizeof(char));
		strcpy(params[i],argv[i]);
		printf("%s ",params[i]);
		Stack *s = (Stack *)malloc(sizeof(Stack));
		//create stack with params and nparams
	}

	//execute(s,args,flags);
	exit(EXIT_SUCCESS);
}

int isInArray(char *array[], long length, char *str) {
	char **key = &str;
	return bsearch(key, array, length, sizeof(array[0]), cmpstringp) != NULL;
}

static int cmpstringp(const void *p1, const void *p2) { // from "man qsort": the qsort method accepts void pointers only
	return (strcmp(*(char **)p1, *(char **)p2));
}
