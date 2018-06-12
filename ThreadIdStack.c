#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include "ThreadIdStack.h"

int isThreadIdStackEmpty(ThreadIdStack *s) {
	return s->next == NULL;
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
	//~ printf("\tinserted %lu\n", *(s->next->tid));
//	free(intid);
}

pthread_t *topT(ThreadIdStack *s) {
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
	//~ printf("\treturning %lu\n", *res);
	ThreadIdStack* toFree = s->next;
	s->next = s->next->next;
	printf("makeing free %p (tid = %lu, next = %p) (actual root next = %p)\n", toFree, *toFree->tid, toFree->next, s->next);
	//~ free(toFree);
	printf("\t\t\t\tdone\n\n");
	return res;
}
