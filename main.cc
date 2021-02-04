#include "network.h"
#include "common.h"
#include "main.h"



Bundle *bundle = 0;
bool logdata = false; // log what's being sent and received.


int main()
{


    bundle = new Bundle("0.0.0.0", 7777, "Gandalf");

    bundle->rpcManager()->registerRPCs();

    bundle->networkManager()->connect();
    initializeEntries(bundle->networkManager()->getThis());

    listener();

    return 0;
}

