#include <pthread.h>


extern pthread_t *tickThread;
extern pthread_t *blocksWrapperThread;

void initializeEntries(void *_this);
void listener();
