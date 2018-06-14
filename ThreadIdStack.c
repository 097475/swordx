#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "ThreadIdStack.h"

int isThreadIdStackEmpty(ThreadIdStack *s) {
	return s->next == NULL;
}

ThreadIdStack* createThreadIdStack() {
	ThreadIdStack* ret = malloc(sizeof(ThreadIdStack*));
	ret->tid = NULL;
	ret->next = NULL;
	return ret;
}

void threadIdPush(ThreadIdStack *s, pthread_t *intid) {
	// create a new stack piece
	ThreadIdStack *ns = (ThreadIdStack *)malloc(sizeof(ThreadIdStack));
	printf("new threadidstack pointer = %p\n\n",ns);
	// set the value
	ns->tid = (pthread_t *)malloc(sizeof(pthread_t*));
	*ns->tid = *intid;
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
