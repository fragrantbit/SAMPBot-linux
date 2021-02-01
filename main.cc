#include "network.h"
#include "common.h"
#include "main.h"



API *api = 0;
bool logdata = false; // log what's being sent and received.


int main()
{


    api = new API("0.0.0.0", 0000, "Gandalf");

    api->getRPCInterface()->registerRPCs();

    api->getNetworkInterface()->initRequest();
    initializeEntries(api->getNetworkInterface()->getThis());

    listener();

    return 0;
}

