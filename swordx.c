#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <getopt.h>
#include <dirent.h>
#include "swordx.h"

static int cmpstringp ( const void* , const void* );
int isInArray ( char** , long , char* );
void writeHelp();
extern char *canonicalize_file_name(const char*);
char* _getWord(FILE *pf); 
void scan(char *path, Stack *files,unsigned char flags, char *explude);

void getBlacklist(Trie *t, char *file)
{
	char *str;
	FILE *pf = fopen(file,"r");
	if(pf==NULL) {perror("--ignore"); exit(EXIT_FAILURE);}
	
	while((str = _getWord(pf)) != NULL)
			add(str,t); 
	
	fclose(pf);
}

char* _getWord(FILE *pf) {
	char c, buf[500]; // c is the character, buf is the word
	int pos = 0; // index

	while(!isalnum(c = getc(pf)) && c != EOF); // remove all not-alphabetic character before a string
	if(c == EOF) return NULL;
	else ungetc(c,pf); // put the last char back


	while(isalnum(c = getc(pf))) {
		buf[ pos++ ] = tolower(c);
		if(pos == 500) {
			fprintf(stderr,"Word is too long\n");
			exit(EXIT_FAILURE);
		}
	}
	char *word = (char*)malloc((pos+1)*sizeof(char));
	strncpy(word,buf,pos);
	word[pos]='\0';
	return word;
}

int _isalphanum(char * str) {
	for(int i = 0; i < strlen(str); i++)
		if(isdigit(str[i])) return 1;
	return 0;
}

char* getWord(FILE *pf, int min, Trie *ignoreTrie, unsigned char flags) {
	char *ret = NULL;
	do {
		if(ret != NULL) free(ret);
		ret = _getWord(pf);
	} while(ret != NULL && (strlen(ret) < min || search(ret,ignoreTrie) || ((flags & ALPHA_FLAG) && _isalphanum(ret))));

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

FILE* makeFile(char *output) {
	FILE *pf = fopen(output,"wb");
	if(pf == NULL) {
		perror("Error writing");
		exit(EXIT_FAILURE);
	}
	return pf;
}

void writeWord(Trie *t,FILE *pf) {
	fprintf(pf,"%s: %lu\r\n",t->value,t->occurrencies);
}

void orderedPrint(BST *b, FILE *pf) {
	if(b != NULL){
		orderedPrint(b->left,pf);
		writeWord(b->word,pf);
		orderedPrint(b->right,pf);
	}

}

void sortTrie(Trie *t, BST **b){
	if(t == NULL) return;
	if(t->occurrencies > 0)
	{
		//printf("%s\n",t->value);
		addBST(b,t);
	}

	for(int i = 0; i < CHARSET; i++)
		sortTrie(t->children[i],b);
}

void sbo(Trie *t, FILE *pf) {
	BST **b = (BST**)malloc(sizeof(BST*));
	sortTrie(t,b);
	orderedPrint(*b,pf);
}

void visitTree(Trie *t, FILE *pf) {
	if(t == NULL) return;
	if(t->occurrencies > 0) // if the word occurrs at least one time...
		writeWord(t,pf); // ... write the word in the file
	for(int i = 0; i < CHARSET; i++)
		visitTree(t->children[i],pf); // check each children of the node
}

Stack *arrayToStack(char **par, int np, char *explude, unsigned char flags) {
	Stack *s = (Stack *)malloc(sizeof(Stack));
	s->value = NULL;
	s->next = NULL;

	struct stat filestats;
	
	for(int i = 0; i < np; i++) {
		 if (lstat(par[i], &filestats) == -1) {
        	perror("stat");
        	exit(EXIT_FAILURE);
   		 }
		else
		{
			 printf("File type:                ");

   			switch (filestats.st_mode & S_IFMT) {
    		case S_IFDIR:  printf("directory\n");scan(par[i],s,flags,explude);break;
    		case S_IFREG:  printf("regular file\n"); push(s,par[i]);  break;
    		default:fprintf(stderr,"Unknown file type \"%s\"\n",par[i]); exit(EXIT_FAILURE);break;
    		}
		}

		//if(explude!=NULL && !strcmp(par[i],explude)) continue; // to explude a file
	}
    free(par);
    free(explude);
	return s;
}

char *absPath(char *path) {
	path = canonicalize_file_name(path); // return the link location also, following link2link
	if(path == NULL) {
		perror("getting absolute path");
		exit(EXIT_FAILURE);
	}
	return path;
}

void scan(char *path, Stack *files,unsigned char flags, char *explude) {
	Stack *folders = (Stack *)malloc(sizeof(Stack));
	DIR *dirp;
	struct dirent *dp;
	
	if(explude != NULL)
		explude = absPath(explude);
	else
		explude = "";
	
	path = absPath(path);
	push(folders,path);
	
    while(!isStackEmpty(folders)) {
		path = pop(folders);
		if (( dirp = opendir(path) ) == NULL) {
			perror("Error opening dir");
			exit(EXIT_FAILURE);
		}
		//~ printf("analayzing %s\n",path);
		while( (dp = readdir(dirp)) != NULL)
			if(strcmp(dp->d_name,"..") != 0 && strcmp(dp->d_name,".") != 0) {
				
				char *toInsert = malloc((strlen(dp->d_name) + strlen(path))*sizeof(char) + 2);
				strcpy(toInsert,path);
				strcat(toInsert,"/");
				strcat(toInsert,dp->d_name);
				
				if(strcmp(toInsert,explude) != 0) {
					if(dp->d_type == DT_DIR && ((flags & RECURSE_FLAG) == RECURSE_FLAG))
						push(folders,toInsert);
						//~ printf("\tnew dir: %s\n", dp->d_name);
					else if(dp->d_type == DT_REG)
						push(files,toInsert);
						//~ printf("\tnew file: %s\n", dp->d_name);
					else if(dp->d_type == DT_LNK && (flags & FOLLOW_FLAG) == FOLLOW_FLAG) {
						toInsert = absPath(toInsert);
						struct stat *sb = malloc(sizeof(struct stat));
						if (lstat(toInsert, sb) == -1) {
							perror("stat");
							exit(EXIT_FAILURE);
						}
						switch (sb->st_mode & S_IFMT) {
							case S_IFDIR: // points to a directory
								if((flags & RECURSE_FLAG) == RECURSE_FLAG && strcmp(toInsert,explude) != 0)
									push(folders,toInsert);
								break;
							case S_IFREG: // points to a regular file
								if(strcmp(toInsert,explude) != 0)
									push(files,toInsert);
								break;
							default:
								printf("\tI don't care about %s\n", dp->d_name);
								break;
						}
						//~ printf("\tnew link: %s\n", dp->d_name);
					}
					else
						printf("Don't care about %s", dp->d_name);
				}
			}
	}
    closedir(dirp);
    free(folders);
}

void execute(Stack* s, char** args, unsigned char flags) {
	int min = (args[0] == NULL) ? 1 : atoi(args[0]);
	char *ignore = args[1];
	char *output = (args[2] == NULL) ? "swordx.out" : args[2];

	if(min<=0) {
		fprintf(stderr,"Error: Insert a valid value for --min | -m option. <num> must be > 0");
		exit(EXIT_FAILURE);
	}


	Trie *t = createTree();
	Trie *ignoreTrie = createTree();
	
	if(ignore!=NULL)
		getBlacklist(ignoreTrie,ignore);

	char *src,*str;
	FILE *pfread;
	while(!isStackEmpty(s))
	{
		src = pop(s);
		pfread = open_file(src);
		while((str = getWord(pfread,min,ignoreTrie, flags)) != NULL)
			add(str,t); // add the word to the trie (starting from the 1st level)
		fclose(pfread);
		free(src);
	}
	FILE *pfwrite = makeFile(output);

	if(flags & SBO_FLAG)
		sbo(t,pfwrite);
	else
		visitTree(t,pfwrite);

	fclose(pfwrite);
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

	args = (char**)malloc(3*sizeof(char*));
	args[0] = min;
	args[1] = ignore;
	args[2] = output;


	nparams = argc-optind;
	params = (char**)malloc(nparams*sizeof(char*));
	for(int i = optind; i<argc; i++) { // make params
		params[i-optind] = (char*)malloc((strlen(argv[i])+1) * sizeof(char));
		strcpy(params[i-optind],argv[i]);
	}
	//create stack with params and nparams
	Stack *s = arrayToStack(params,nparams,explude,flags);
	//~ visitStack(s);
	execute(s,args,flags);
	exit(EXIT_SUCCESS);
}

int isInArray(char *array[], long length, char *str) {
	char **key = &str;
	return bsearch(key, array, length, sizeof(array[0]), cmpstringp) != NULL;
}
static int cmpstringp(const void *p1, const void *p2) { // from "man qsort": the qsort method accepts void pointers only
	return (strcmp(*(char **)p1, *(char **)p2));
}
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

