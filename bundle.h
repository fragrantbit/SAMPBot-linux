#include "network.h"
#include "client.h"
#include "rpc.h"

class Bundle {
private:
    Network *_network;
    Client *_client;
    RPC *_rpc;
public:
    Bundle(char const *addr, int port, char const *name) {
        _network = new Network(addr, port);
        _client = new Client(name);
        _rpc = new RPC;
    }

    Network *networkManager() {
        return this->_network;
    }

    Client *clientManager() {
        return this->_client;
    }

    RPC *rpcManager() {
        return this->_rpc;
    }
};