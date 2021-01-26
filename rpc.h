#ifndef RPC_H
#define RPC_H

#include "rakutils.h"
#include <map>


struct RPCNode {
    int uniqueIdentifier;
    char const *data;
    int len;
};
class RPC {
    std::map<int, void (*)(const RPCNode &)> rpcMap;
    
public:

    void sendRPC(
        int rpcID, 
        RakNet::BitStream *bs, 
        PacketPriority priority, 
        PacketReliability reliability);


    void handleRPC(char const *data, int len);

    void registerRPCCallback(int uniqueIndentifier, void (*callback)(const RPCNode &));
    void RPCCallback(const RPCNode &rpcNode);

    void registerRPCs();
    
};



#endif