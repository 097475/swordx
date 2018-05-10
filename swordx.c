#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include "swordx.h"


void printall(Trie *t)
{
	if(t!=NULL)
	{
		printf("value: %s\toccurrences: %d\n",t->value,t->occurrencies);
		for(int i = 0; i<26; i++)
		{
			printall(t->children[i]);		
		}
	}
	

	
}

void addRecord(char* str, Trie *t, int level){
Trie *n = (Trie*)malloc(sizeof(Trie));

        if(t->children[str[level-1]-'a'] == NULL)
        {           
            n->value = (char*)malloc(level+1);
            strncpy(n->value,str,level);  //bad function
           // snprintf(n->value,level+1,"%s",str); //space for null character
           // n->value=str;
           	 n->value[level] = '\0';
            //printf("%s %d\n",n->value,level);
            
            
            for(int j = 0; j<26; j++)
			{
				n->children[j] = NULL;
			}
			n->occurrencies=0;
			t->children[str[level-1]-'a'] = n;
        }
            if(level==strlen(str))
            {
                n->occurrencies++;
                free(str);
            }
            else{
                level++;
                addRecord(str,n,level);
            }
}

void add (char* str, Trie *t, int level){ //start with 1
int flag = 0;
Trie *next;

	next = t->children[str[level-1]-'a'];
    if(next != NULL && strncmp(str,next->value,level)==0)
    {
        //matched
        if(level == strlen(str))
        {
            // add count
            next->occurrencies++;
        }
        else{
            level++;
            add(str,next,level);
        }

        flag = 1; //insert successful
    }



if(flag==0)
{
	addRecord(str,t,level);
}
//unmatched, create record(s)
//printf("%s%d",str,level);


}

FILE* open_file(char* path){
	FILE* pf = fopen(path,"rb");

	if(pf==NULL)
	{
		perror("");
		exit(EXIT_FAILURE);
	}
	return pf;	
}

char* getWord(FILE *pf)
{
	char c;
	char buf[500];
	int pos = 0;
	while(!isalpha(c=getc(pf)) && c!=EOF);
	ungetc(c,pf);
	while(isalpha(c=getc(pf)))
	{
		//printf("%c ",
			//printf("%c\n",c);
			if(isupper(c))
			{
				c = tolower(c);
			}
			buf[pos] = c;
			pos++;
	
		if(pos==500)
		{
			fprintf(stderr,"Word is too long\n");
			exit(EXIT_FAILURE);
		}

	}
	//printf("%d ",pos);
	if(c==EOF)
	{
		return NULL;
	}
	//printf("buf: %s\n",buf);
	char *word = (char*)malloc((pos+1)*sizeof(char));
	//int tes = snprintf(word,pos+1,"%s",buf); //space for null character
	//word[pos]='\0';
	strncpy(word,buf,pos); //bad function
	word[pos] = '\0';
	//printf("%s %s pos:%d\n",buf,word,pos);
	
	// clear word
	
	return word; 
}

FILE* makeFile()
{
	FILE *pf = fopen("result.txt","wb");
	if(pf == NULL)
	{
		perror("");
		exit(EXIT_FAILURE);
	}
	
	return pf;
}

void writeWord(Trie *t,FILE *pf)
{
	char *word = t->value;
	int occurrencies = t->occurrencies;
	
	fwrite(word,sizeof(char),strlen(word),pf);
	putc(':',pf);
	putc(' ',pf);
	putc(occurrencies+'0',pf);
	putc('\r',pf);
	putc('\n',pf);
}


void visitTree(Trie *t, FILE *pf){
	
	if(t!=NULL){
		
		if(t->occurrencies>0)
		{
			writeWord(t,pf);
		}
		
		for(int i = 0; i<26; i++)
		{
			visitTree(t->children[i],pf);		
		}
		
	}
	
}

Trie* createTree()
{
	Trie *t = (Trie*)malloc(sizeof(Trie));
	t->value="";
	t->occurrencies=0;
	for(int i = 0; i<26; i++)
	{
		t->children[i] = NULL;
	}
	return t;
}

void execute(char *src)
{
	FILE *pfread = open_file(src);
	Trie *t = createTree();
	char *str;
	while((str=getWord(pfread))!=NULL)
	{
		add(str,t,1);
	}
	FILE *pfwrite = makeFile();
	visitTree(t,pfwrite);
	fclose(pfread);
	fclose(pfwrite);
	//printall(t);
}

int main(int argc, char *argv[])
{
	int option_index = 0;
	int opt;
    static struct option long_options[] = {
    {"add",     optional_argument, 0,  0 },
    {"append",  no_argument, 0,  0 },
    {"delete",  no_argument, 0,  0 },
    {"verbose", no_argument, 0,  0 },
    {"create",  no_argument, 0,  0 },
    {"file",    no_argument, 0,  0 },
    };
		
 while ((opt = getopt_long(argc, argv, "f:",long_options,&option_index)) != -1)
 {
 	switch(opt)
 	{
 		case 0:  printf("option %s", long_options[option_index].name); break;
 		case 'f':
            printf("from file(s)  %s\n",optarg);
            execute(optarg);
            break;
	}
 }

/*char* str = "test";
add(str,t,1);
add("tent",t,1);
add("test",t,1);
add("sternocleidomastoideo",t,1);
add("concupiscenza",t,1);*/




//printall(t);


/*printf("%s",getWord(pf));
printf("%s",getWord(pf));
printf("%s",getWord(pf));*/
}
