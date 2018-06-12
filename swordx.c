#include <ctype.h>
#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "swordx.h"
#include <pthread.h>
#include "ThreadIdStack.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int cmpstringp ( const void* , const void* );
int isInArray ( char** , long , char* );
void writeHelp( char* );
extern char *canonicalize_file_name(const char*);
char* _getWord(FILE *pf); 
void scan(char *path, Stack *files,unsigned char flags, char *explude);

Trie* t;
struct ThreadArgs {
	char* src;
	int min;
	Trie* ignoreTrie;
	unsigned char flags;
};

void exitWithError(char *error) {
	perror(error);
	exit(EXIT_FAILURE);
}

void getBlacklist(Trie *t, char *file) {
	char *str;
	FILE *pf = fopen(file,"r");
	if(pf == NULL)
		exitWithError("opening file");
	
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
		if(ret != NULL) { printf("qui1\n"); free(ret); }
		ret = _getWord(pf);
	} while(ret != NULL && (strlen(ret) < min || search(ret,ignoreTrie) || ((flags & ALPHA_FLAG) && _isalphanum(ret))));

	return ret;
}

FILE* open_file(char* path) {
	FILE* pf = fopen(path,"rb");
	if(pf == NULL)
		exitWithError("Error reading");
	return pf;
}

FILE* makeFile(char *output) {
	FILE *pf = fopen(output,"wb");
	if(pf == NULL)
		exitWithError("Error writing");
	return pf;
}

void orderedWrite(BST *b, FILE *pf) {
	if(b != NULL) {
		orderedWrite(b->left,pf);
		writeNodeInformation(b->wordInfo,pf);
		orderedWrite(b->right,pf);
	}
}

void sortTrie(Trie *t, BST **b) {
	if(t == NULL) return;
	if(t->occurrencies > 0)
		addBST(b,t);

	for(int i = 0; i < CHARSET; i++)
		sortTrie(t->children[i],b);
}

void sbo(Trie *t, FILE *pf) {
	BST **b = (BST**)malloc(sizeof(BST*));
	sortTrie(t,b);
	orderedWrite(*b,pf);
}

Stack *arrayToStack(char **par, int np, char *explude, unsigned char flags) {
	Stack *s = (Stack *)malloc(sizeof(Stack));
	s->value = NULL;
	s->next = NULL;

	struct stat filestats;
	
	for(int i = 0; i < np; i++) {
		if (lstat(par[i], &filestats) == -1)
			exitWithError("stat");
		else
   			switch (filestats.st_mode & S_IFMT) {
				case S_IFDIR:
					scan(par[i],s,flags,explude);
					break;
				case S_IFREG:
					push(s,par[i]);
					break;
				default:
					fprintf(stderr,"Unknown file type: \"%s\"\n",par[i]);
					exit(EXIT_FAILURE);
					break;
    		}
	}
	printf("qui2\n");
    free(par);
    printf("qui3\n");
    free(explude);
	return s;
}

char *absPath(char *path) {
	path = canonicalize_file_name(path); // return the link location also, following link2link
	if(path == NULL)
		exitWithError("Getting absolute path");
	return path;
}

void scan(char *path, Stack *files, unsigned char flags, char *explude) {
	Stack *folders = (Stack *)malloc(sizeof(Stack));
	DIR *dirp;
	struct dirent *dp;
	
	if(explude != NULL)	explude = absPath(explude);
	else				explude = "";
	
	path = absPath(path);
	push(folders,path);
	
    while(!isStackEmpty(folders)) {
		path = pop(folders);
		if (( dirp = opendir(path) ) == NULL)
			exitWithError("Error opening dir");
			
		while( (dp = readdir(dirp)) != NULL)
			if(strcmp(dp->d_name,"..") != 0 && strcmp(dp->d_name,".") != 0) {
				char *toInsert = malloc((strlen(dp->d_name) + strlen(path))*sizeof(char) + 2);
				strcpy(toInsert,path);
				strcat(toInsert,"/");
				strcat(toInsert,dp->d_name);
				
				if(strcmp(toInsert,explude) != 0) {
					if(dp->d_type == DT_DIR && ((flags & RECURSE_FLAG) == RECURSE_FLAG))
						push(folders,toInsert);
					else if(dp->d_type == DT_REG)
						push(files,toInsert);
					else if(dp->d_type == DT_LNK && (flags & FOLLOW_FLAG) == FOLLOW_FLAG) {
						toInsert = absPath(toInsert);
						struct stat *sb = malloc(sizeof(struct stat));
						if (lstat(toInsert, sb) == -1)
							exitWithError("stat");
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
								printf("\tUnkown file type: \"%s\"\n", dp->d_name);
								break;
						}
					}
					else
						printf("\tUnkown file type: \"%s\"\n", dp->d_name);
				}
			}
	}
    closedir(dirp);
    printf("qui4\n");
    free(folders);
}

void* threadFun(void* arg) {
	
	printf("\tgetting args\n\n");
	
	struct ThreadArgs* a = (struct ThreadArgs*)arg;
	char* str;
	
	printf("\topening file %s\n\n",a->src);
	
	FILE *pfread = open_file(a->src);
	while((str = getWord(pfread,a->min,a->ignoreTrie,a->flags)) != NULL) {
		pthread_mutex_lock(&mutex);
			add(str,t); // add the word to the trie (starting from the 1st level)
		pthread_mutex_unlock(&mutex);
	}
	
	fclose(pfread);
	
	//~ printf("\tfreeing src\n\n");
	
	//~ free(a->src);
	
	printf("\tthread done\n\n");
	
	return NULL;
}

void execute(Stack* s, char** args, unsigned char flags) {
	int min = (args[0] == NULL) ? 1 : atoi(args[0]);
	char *ignore = args[1];
	char *output = (args[2] == NULL) ? "swordx.out" : args[2];

	if(min <= 0) {
		fprintf(stderr,"Error: Insert a valid value for --min <num> | -m <num> option.\n\t<num> must be > 0");
		exit(EXIT_FAILURE);
	}
	
	t = createTree();
	Trie *ignoreTrie = createTree();
	
	if(ignore!=NULL)
		getBlacklist(ignoreTrie,ignore);
	
	// --- Threads Part ---
	char* src;
	ThreadIdStack *ts = malloc(sizeof(ThreadIdStack*));
	ts->tid = NULL;
	ts->next = NULL;

	pthread_t* tid;
	while(!isStackEmpty(s))	{
			printf("popping src\n\n");
		src = pop(s);
			printf("allocing threadArgs\n\n");
		struct ThreadArgs* a = malloc(sizeof(struct ThreadArgs*));
			printf("inserting args into threadargs\n\n");
		a->src = src;
		a->min = min;
		a->ignoreTrie = ignoreTrie;
		a->flags = flags;
			printf("done\n\n");
			printf("creating thread\n\n");
		int err = pthread_create(tid, NULL, &threadFun, a);
			printf("done\n\n");
		if (err != 0)
			exitWithError("Can't create thread");
		else {
				printf("pushing the thread into the stack (tid = %lu)\n\n",*tid);
			threadIdPush(ts,tid);
		}
	}
		printf("waiting for threads\n\n");
	while(!isThreadIdStackEmpty(ts)) {
			printf("\t\tgetting tid\n\n");
		tid = threadIdPop(ts);
			printf("\t\twaiting %lu\n\n",*tid);
		pthread_join(*tid,NULL);
			printf("\t\t%lu done\n\n",*tid);
		free(tid);
	}
		printf("bye bye bye bye\n");
	//~ printall(t);
	// --- END Threads Part ---
	FILE *pfwrite = makeFile(output);

	if(flags & SBO_FLAG)
		sbo(t,pfwrite);
	else
		writeTrie(t,pfwrite);

	fclose(pfwrite);
}

int main(int argc, char *argv[]) {
	int opt = 0, nparams = 0;
	char *explude = NULL,*ignore = NULL,*output = NULL, *min = NULL, **args, **params;
	unsigned char flags = 0; //1 byte integer field

    struct option long_options[] = {
		{	"help",				no_argument,		0,		0	},
		{	"recursive",		no_argument,		0,		1	},
		{	"follow",			no_argument,		0,		2	},
		{	"explude",			required_argument,	0,		3	},
		{	"alpha",			no_argument,		0,		4	},
		{	"min",				required_argument,	0,		5	},
		{	"ignore",			required_argument,	0,		6	},
		{	"sortbyoccurrency",	no_argument,		0,		7	},
		{	"sbo",				no_argument,		0,		7	},
		{	"output",			required_argument,	0,		8	},
		{	NULL,				0,					NULL,	0	}  //required
	};

	while ((opt = getopt_long_only(argc, argv, "hrfe:am:i:",long_options,NULL)) != -1) {
		switch(opt) {
			case 0:	case 'h':
				writeHelp(argv[0]); break;
			case 1: case 'r':
				flags |= RECURSE_FLAG; break;
			case 2: case 'f':
				flags |= FOLLOW_FLAG; break;
			case 3: case 'e':
				explude = (char*)malloc((strlen(optarg)+1)*sizeof(char));
				strcpy(explude,optarg); break;
			case 4: case 'a':
				flags |= ALPHA_FLAG; break;
			case 5: case 'm':
				min = (char*)malloc((strlen(optarg)+1)*sizeof(char));
				strcpy(min,optarg); break;
			case 6: case 'i':
				ignore = (char*)malloc((strlen(optarg)+1)*sizeof(char));
				strcpy(ignore,optarg); break;
			case 7:
				flags |= SBO_FLAG; break;
			case 8:
				output = (char*)malloc((strlen(optarg)+1)*sizeof(char));
				strcpy(output,optarg); break;
			default:
				printf("default case");break;
		}
	}

	args	= (char**)malloc(3 * sizeof(char*));
	args[0]	= min;		// min word lenght
	args[1]	= ignore;	// ignore file list
	args[2]	= output;	// output filename

	nparams = argc-optind; // number of input files
	params = (char**)malloc(nparams * sizeof(char*)); // list of filenames

	for(int i = optind; i<argc; i++) { // make params
		params[i-optind] = (char*)malloc((strlen(argv[i])+1) * sizeof(char));
		strcpy(params[i-optind],argv[i]);
	}
	//create stack with params and nparams
	Stack *s = arrayToStack(params,nparams,explude,flags);
	
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

void writeHelp(char *appname) {
	printf("%s [options] [inputs]\n", appname);
	printf("   swordx counts the occurrencies of each words (alphanumeric characters by default) in a file or a range of files and print them into a new file.\n");
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

