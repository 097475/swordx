#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"

int isStackEmpty(Stack *s) {
	return s->next == NULL;
}

void push(Stack *s, char *str) {
	// create a new stack piece
	Stack *ns = (Stack *)malloc(sizeof(Stack));
	// set the value
	ns->value = (char *)malloc((strlen(str) + 1)*sizeof(char));
	strcpy(ns->value, str);
	//ns->value[strlen(str)] = '\0';
	// next is null
	ns->next = s->next;
	s->next = ns;
	free(str);
}

char *top(Stack *s) {
	if(s->next == NULL) return NULL;
	return s->next->value;
}

char *pop(Stack *s) {
	if(isStackEmpty(s)) return NULL;
	char *res = s->next->value;
	Stack *toFree = s->next;
	s->next = s->next->next;
	free(toFree);
	return res;
}

void visitStack(Stack *s) {
	if(top(s) == NULL) return;
	printf("%s\n",top(s));
	return visitStack(s->next);
}

int searchStack(Stack *s, char *needle){
	if(top(s) == NULL) return 0;
	else return strcmp(top(s),needle) ? searchStack(s->next,needle) : 1;
	
}

/*int main(int argc, char **argv) {
	Stack *s = (Stack *)malloc(sizeof(Stack));
	s->value = NULL;
	s->next = NULL;
	push(s,"o");
	push(s,"a");
	push(s,"i");
	push(s,"c");
	while(!isStackEmpty(s))
		printf("%s\n",pop(s));
	return 0;
}*/

