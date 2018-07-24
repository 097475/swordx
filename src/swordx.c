#include <ctype.h>
#include <dirent.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <glob.h>
#include <pthread.h>
#include "ThreadIdStack.h"
#include "trie.h"
#include "stack.h"
#include "BST.h"

#define RECURSE_FLAG (1<<0) //00000001
#define FOLLOW_FLAG (1<<1) //00000010
#define ALPHA_FLAG (1<<2) //00000100
#define SBO_FLAG (1<<3) //00001000

void exitWithError(char * );
void getBlacklist(Trie * , Stack * );
char* _getWord(FILE * ); 
int _isalphanum(char * );
char* getWord ( FILE * , int , Trie* , unsigned char );
FILE* open_file ( char* );
FILE* makeFile ( char * );
void sortTrie ( Trie* , BST** );
void sbo(Trie * , FILE * );
Stack* expand(Stack * );
Stack* arrayToStack(char ** , int , Stack * , unsigned char);
char* absPath(char * );
void scan(char * , Stack * , unsigned char, Stack * );
void* threadFun(void * );
void execute(Stack * , char ** , Trie * , unsigned char);
void writeHelp( char * );
extern char *canonicalize_file_name(const char*); 


typedef struct threadArgs {
	char* src;
	int min;
	Trie* ignoreTrie;
	unsigned char flags;
}ThreadArgs;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Trie* t;


void exitWithError(char *error) {
	perror(error);
	exit(EXIT_FAILURE);
}

void getBlacklist(Trie *t, Stack *ignore) {
	char *str,*src;
	FILE *pf;
	while(!isStackEmpty(ignore))
	{
		src = pop(ignore);
		pf = fopen(src,"r");
		if(pf == NULL)
			exitWithError("opening file");
		while((str = _getWord(pf)) != NULL)
			add(str,t); 
		fclose(pf);
	}
	free(ignore);
}

char* _getWord(FILE *pf) {
	char c, buf[500];
	int pos = 0; 
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
	if(word == NULL) exitWithError("No more heap space");
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



void sortTrie(Trie *t, BST **b) {
	if(t == NULL) return;
	if(t->occurrencies > 0)
		addBST(b,t);

	for(int i = 0; i < CHARSET; i++)
		sortTrie(t->children[i],b);
}

void sbo(Trie *t, FILE *pf) {
	BST **b = createBST();
	sortTrie(t,b);
	writeTree(*b,pf);
}

Stack* expand(Stack *toExpand)
{
	char* tmp;
	struct stat filestats;
	Stack *result = createStack();
	glob_t globbuf;
	while(!isStackEmpty(toExpand))	{
		tmp = pop(toExpand);
		if(glob(tmp, 0,NULL, &globbuf)!= GLOB_NOMATCH)
		{

			for(int i = 0; i < globbuf.gl_pathc; i++)
			{	
				if (lstat(globbuf.gl_pathv[i], &filestats) == -1)
					exitWithError("stat");
				else
				{
					if((filestats.st_mode & S_IFMT) == S_IFDIR)
						scan(globbuf.gl_pathv[i],result,0,createStack());
					else 
						push(result,absPath(globbuf.gl_pathv[i]));	
				}

			}
		}

	}
	free(toExpand);
	return result;
}



Stack *arrayToStack(char **par, int np, Stack *_explude, unsigned char flags) {
	Stack *s = createStack();
	Stack *explude = expand(_explude);
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
	free(par);
	return s;
}

char *absPath(char *path) {
	path = canonicalize_file_name(path); // return the link location also, following link2link
	if(path == NULL)
		exitWithError("Getting absolute path");
	return path;
}

void scan(char *path, Stack *files, unsigned char flags, Stack *explude) {
	Stack *folders = createStack();
	DIR *dirp;
	struct dirent *dp;
	
	path = absPath(path);
	push(folders,path);
	
    while(!isStackEmpty(folders)) {
		path = pop(folders);
		if (( dirp = opendir(path) ) == NULL)
			exitWithError("Error opening dir");
			
		while( (dp = readdir(dirp)) != NULL)
			if(strcmp(dp->d_name,"..") != 0 && strcmp(dp->d_name,".") != 0) {
				char *toInsert = malloc((strlen(dp->d_name) + strlen(path))*sizeof(char) + 2);
				if(toInsert == NULL) exitWithError("No more heap space");
				strcpy(toInsert,path);
				strcat(toInsert,"/");
				strcat(toInsert,dp->d_name);
				if(searchStack(explude,toInsert)==0) {
					if(dp->d_type == DT_DIR && ((flags & RECURSE_FLAG) == RECURSE_FLAG))
						push(folders,toInsert);
					else if(dp->d_type == DT_REG)
						push(files,toInsert);
					else if(dp->d_type == DT_LNK && (flags & FOLLOW_FLAG) == FOLLOW_FLAG) {
						toInsert = absPath(toInsert);
						struct stat *sb = malloc(sizeof(struct stat));
						if(sb == NULL) exitWithError("No more heap space");
						if (lstat(toInsert, sb) == -1)
							exitWithError("stat");
						switch (sb->st_mode & S_IFMT) {
							case S_IFDIR: // points to a directory
								if((flags & RECURSE_FLAG) == RECURSE_FLAG && searchStack(explude,toInsert)==0)
									push(folders,toInsert);
								break;
							case S_IFREG: // points to a regular file
								if(searchStack(explude,toInsert)==0)
									push(files,toInsert);
								break;
							default:
								printf("\tUnkown file type: \"%s\"\n", dp->d_name);
								break;
						}
					}
				}
			}
	}
    closedir(dirp);
    free(folders);
}

void* threadFun(void* arg) {	
	ThreadArgs* a = (ThreadArgs*)arg;
	char* str;	
	FILE *pfread = open_file(a->src);
	while((str = getWord(pfread,a->min,a->ignoreTrie,a->flags)) != NULL) {
		pthread_mutex_lock(&mutex);
			add(str,t); 
		pthread_mutex_unlock(&mutex);
	}	
	fclose(pfread);
	return NULL;
}

void execute(Stack* s, char** args, Trie *ignoreTrie, unsigned char flags) {
	int min = (args[0] == NULL) ? 1 : atoi(args[0]);
	char *output = (args[1] == NULL) ? "swordx.out" : args[1];

	if(min <= 0) {
		fprintf(stderr,"Error: Insert a valid value for --min <num> | -m <num> option.\n\t<num> must be > 0");
		exit(EXIT_FAILURE);
	}
	
	t = createTree();
	
	// --- Threads Part ---
	char* src;
	ThreadIdStack *ts = createThreadIdStack();	
	pthread_t *tid;
	while(!isStackEmpty(s))	{
		src = pop(s);
		tid = (pthread_t*)malloc(sizeof(pthread_t));
		if(tid == NULL) exitWithError("No more heap space");
		ThreadArgs *a = malloc(sizeof(ThreadArgs));
		if(a == NULL) exitWithError("No more heap space");
		a->src = src;
		a->min = min;
		a->ignoreTrie = ignoreTrie;
		a->flags = flags;
		int err = pthread_create(tid, NULL, &threadFun, a);
		if (err != 0)
			exitWithError("Can't create thread");
		else 
			threadIdPush(ts,tid);
	}
	while(!isThreadIdStackEmpty(ts)) {
		tid = threadIdPop(ts);
		pthread_join(*tid,NULL);
		free(tid);
	}
	// --- END Threads Part ---
	FILE *pfwrite = makeFile(output);

	if(flags & SBO_FLAG)
		sbo(t,pfwrite);
	else
		writeTrie(t,pfwrite);

	fclose(pfwrite);
}

int main(int argc, char *argv[]) {
	if(argc == 1){
		writeHelp(argv[0]);
		exit(EXIT_SUCCESS);
	}
	int opt = 0, nparams = 0;
	char *str = NULL,*output = NULL, *min = NULL, **args, **params;
	Stack *_explude = createStack();
	Stack *_ignore = createStack();
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
		{	NULL,				0,				NULL,		0	}  //required
	};

	while ((opt = getopt_long_only(argc, argv, "hrfe:am:i:o:",long_options,NULL)) != -1) {
		switch(opt) {
			case 0:	case 'h':
				writeHelp(argv[0]); exit(EXIT_SUCCESS); break;
			case 1: case 'r':
				flags |= RECURSE_FLAG; break;
			case 2: case 'f':
				flags |= FOLLOW_FLAG; break;
			case 3: case 'e':
				str = (char*)malloc((strlen(optarg)+1)*sizeof(char));
				if(str == NULL) exitWithError("No more heap space");
				strcpy(str,optarg);
				push(_explude,str);  ;break;
			case 4: case 'a':
				flags |= ALPHA_FLAG; break;
			case 5: case 'm':
				min = (char*)malloc((strlen(optarg)+1)*sizeof(char));
				if(min == NULL) exitWithError("No more heap space");
				strcpy(min,optarg); break;
			case 6: case 'i':
				str = (char*)malloc((strlen(optarg)+1)*sizeof(char));
				if(str == NULL) exitWithError("No more heap space");
				strcpy(str,optarg);
				push(_ignore,str); break;
			case 7:
				flags |= SBO_FLAG; break;
			case 8: case 'o': //verify
				output = (char*)malloc((strlen(optarg)+1)*sizeof(char));
				if(output == NULL) exitWithError("No more heap space");
				strcpy(output,optarg); break;
			default:
				printf("Unknown option");break; //verify
		}
	}

	args	= (char**)malloc(2 * sizeof(char*));
	if(args == NULL) exitWithError("No more heap space");
	args[0]	= min;		// min word lenght
	args[1]	= output;	// output filename

	nparams = argc-optind; // number of input files
	params = (char**)malloc(nparams * sizeof(char*)); // list of filenames
	if(params == NULL) exitWithError("No more heap space");

	for(int i = optind; i<argc; i++) { // make params
		params[i-optind] = (char*)malloc((strlen(argv[i])+1) * sizeof(char));
		if(params[i-optind] == NULL) exitWithError("No more heap space");
		strcpy(params[i-optind],argv[i]);
	}
	//create stack with params and nparams
	Stack *s = arrayToStack(params,nparams,_explude,flags);

	//create ignore trie
	Stack *ignore = expand(_ignore);
	Trie *ignoreTrie = createTree();
	getBlacklist(ignoreTrie,ignore);

	execute(s,args,ignoreTrie,flags);
	exit(EXIT_SUCCESS);
}

void writeHelp(char *appname) {
	printf("%s [options] [inputs]\n", appname);
	printf("\tswordx counts the occurrencies of each word (alphanumeric characters by default) in a file or a range of files and print them into a new file.\n");
	printf("\n");
	printf("\t[inputs]\n");
	printf("\t\tfiles and/or directories to process\n");
	printf("\t[options]\n");
	printf("\t\t-h | --help : display this message\n");
	printf("\n");
	printf("\t\t-o <filename> | --output <filename> : write the result in a specific output file (<filename>) (NOTE: if the file already exists, it will be overwrited!)\n");
	printf("\t\t\tby default the file is named \"sword.out\"\n");
	printf("\t\t-r | --recursive : go through all the passed directories recursively\n");
	printf("\t\t-f | --follow : follow links\n");
	printf("\t\t-e <file> | --explude <file> : exclude specific files in folders (regular expressions can be used using double quote)\n");
	printf("\n");
	printf("\t\t-a | --alpha : consider alphabetic letters only\n");
	printf("\t\t-m <num> | --min <num> : consider words with at least <num> letters\n");
	printf("\t\t-i <file> | --ignore <file> : all words contained in <file> are ignored (provide one word per line) (regular expressions can be used using double quote)\n");
	printf("\n");
	printf("\t\t--sortbyoccurrency | -sbo : sort words by occurrencies (descending order) in the output file\n");
}

