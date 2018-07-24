typedef struct ThreadIdStack {
	struct ThreadIdStack *next;
	pthread_t *tid;
} ThreadIdStack;

ThreadIdStack* createThreadIdStack();
int isThreadIdStackEmpty( ThreadIdStack * );
void threadIdPush ( ThreadIdStack * , pthread_t * );
pthread_t *threadIdPop ( ThreadIdStack * );
pthread_t* topT(ThreadIdStack * );
void visitThreadsStack( ThreadIdStack * );
