#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "ThreadIdStack.h"

int isThreadIdStackEmpty(ThreadIdStack *s) {
	return s->next == NULL;
}

ThreadIdStack* createThreadIdStack() {
	ThreadIdStack* ret = malloc(sizeof(ThreadIdStack*));
	if(ret == NULL) {
		perror("No more heap space");
		exit(EXIT_FAILURE);
	}
	ret->tid = NULL;
	ret->next = NULL;
	return ret;
}

void threadIdPush(ThreadIdStack *s, pthread_t *intid) {
	// create a new stack piece
	ThreadIdStack *ns = (ThreadIdStack *)malloc(sizeof(ThreadIdStack));
	if(ns == NULL) {
		perror("No more heap space");
		exit(EXIT_FAILURE);
	}
	// set the value
	ns->tid = intid;
	// next is null
	ns->next = s->next;
	s->next = ns;
}

pthread_t* topT(ThreadIdStack *s) {
	if(s->next == NULL) return NULL;
	return s->next->tid;
}

void visitThreadsStack(ThreadIdStack *s) {
	if(topT(s) == NULL) return;
	printf("%p\n",topT(s));
	return visitThreadsStack(s->next);
}

pthread_t* threadIdPop(ThreadIdStack *s) {
	if(isThreadIdStackEmpty(s)) return NULL;
	pthread_t* res = s->next->tid;
	ThreadIdStack* toFree = s->next;
	s->next = s->next->next;
	free(toFree);
	return res;
}
