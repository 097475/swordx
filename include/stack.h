typedef struct Stack {
	struct Stack *next;
    char *value;
} Stack;

Stack* createStack();
int isStackEmpty ( Stack * );
void push ( Stack * , char * );
char* pop ( Stack * );
char* top ( Stack * );
void visitStack ( Stack * );
int searchStack(Stack * , char * );
