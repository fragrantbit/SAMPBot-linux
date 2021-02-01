#include "entries.h"
#include "network.h"


pthread_t *tickThread = 0;
pthread_t *blocksWrapperThread = 0;

static void *tickEntry(void *_this);
static void *blocksWrapperEntry(void *_this); 


void initializeEntries(void *_this) 
{
    tickThread = new pthread_t;
    blocksWrapperThread = new pthread_t;

    pthread_create(tickThread, 0, &tickEntry, _this);
    pthread_create(blocksWrapperThread, 0, &blocksWrapperEntry, _this);
}

static void *tickEntry(void *_this) 
{
    Network *tmp = (Network *)_this;
    return tmp->networkUpdateLoop();
}

static void *blocksWrapperEntry(void *_this)
{
    Network *tmp = (Network *)_this;
    return tmp->blocksWrapper();
}

void listener() 
{
    pthread_join(*blocksWrapperThread, 0);
    pthread_join(*tickThread, 0);
}
