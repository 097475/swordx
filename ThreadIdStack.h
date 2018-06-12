typedef struct ThreadIdStack {
	struct ThreadIdStack *next;
    pthread_t *tid;
} ThreadIdStack;

void threadIdPush ( ThreadIdStack* , pthread_t* );
pthread_t *threadIdPop ( ThreadIdStack* );

int isThreadIdStackEmpty( ThreadIdStack* );

void visitThreadsStack( ThreadIdStack* );
