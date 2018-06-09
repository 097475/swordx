typedef struct Stack {
	struct Stack *next;
    char *value;
} Stack;

int isStackEmpty ( Stack* );
void push ( Stack * , char * );
char *top ( Stack * );
char *pop ( Stack * );
int searchStack(Stack *s, char *needle);

void visitStack ( Stack * );
