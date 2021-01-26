#include "network.h"
#include "client.h"
#include "rpc.h"

class API {
private:
    Network *_network;
    Client *_client;
    RPC *_rpc;
public:
    API(char const *addr, int port, char const *name) {
        _network = new Network(addr, port);
        _client = new Client(name);
        _rpc = new RPC;
    }

    Network *getNetworkInterface() {
        return this->_network;
    }

    Client *getClientInterface() {
        return this->_client;
    }

    RPC *getRPCInterface() {
        return this->_rpc;
    }
};