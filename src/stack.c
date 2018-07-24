#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "stack.h"


Stack* createStack() {
	Stack *s = (Stack*)malloc(sizeof(Stack));
	if(s == NULL) {
		perror("No more heap space");
		exit(EXIT_FAILURE);
	}
	s->value = NULL;
	s->next = NULL;
	return s;
}

int isStackEmpty(Stack *s) {
	return s->next == NULL;
}

void push(Stack *s, char *str) {
	// create a new stack piece
	Stack *ns = (Stack *)malloc(sizeof(Stack));
	if(ns == NULL) {
		perror("No more heap space");
		exit(EXIT_FAILURE);
	}
	// set the value
	ns->value = (char *)malloc((strlen(str) + 1)*sizeof(char));
	if(ns->value == NULL) {
		perror("No more heap space");
		exit(EXIT_FAILURE);
	}
	strcpy(ns->value, str);
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

