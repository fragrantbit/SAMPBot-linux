#include "rpc.h"
#include "common.h"
#include <string.h>

#include "callbacks.h"


void RPC::sendRPC(
      int rpcID, 
      RakNet::BitStream *bs, 
      PacketPriority priority, 
      PacketReliability reliability)
{

    RakNet::BitStream input;
    DataStructures::List<PluginInterface *> messageHandlerList;
    int bitlen = bs->GetNumberOfBitsUsed();

    input.ResetWritePointer();
    
    input.Write((unsigned char)ID_RPC);
    input.Write((unsigned char)rpcID);
    input.WriteCompressed(bitlen);
    input.WriteBits((const unsigned char *)bs->GetData(), bitlen, false);

    bundle->networkManager()->makePacket(input);
    
    bundle->networkManager()->sendTo(
        (char *)input.GetData(), 
        input.GetNumberOfBitsUsed(), false);
}


void RPC::registerRPCCallback(int uniqueIdentifier, void (*callback)(const RPCNode &rpcNode)) {
    rpcMap[uniqueIdentifier] = callback;
}

void RPC::RPCCallback(const RPCNode &rpcNode)
{
    rpcMap[rpcNode.uniqueIdentifier](rpcNode);
}

void RPC::handleRPC(char const *data, int len)
{
    
    RakNet::BitStream incomingBitStream((unsigned char *)data, len, false);
    int uniqueIdentifier = 0;
    // incomingBitStream.IgnoreBits(8);

    if(data[0] == ID_TIMESTAMP) {
        printf("Ignore timestamp\n");
        incomingBitStream.IgnoreBits(8 * (sizeof(RakNetTime) + sizeof(unsigned char)));
    }

    incomingBitStream.Read((char *)&uniqueIdentifier, 1);


    if(incomingBitStream.ReadCompressed(len) == false) {
        printf("Bitstream is not long enough\n");

    }

    unsigned char *userData = new unsigned char[BITS_TO_BYTES(incomingBitStream.GetNumberOfBitsUsed())];

    if(incomingBitStream.ReadBits(
        (unsigned char *)userData, 
        len, 
        false) == false) {
             _datalog("Not enough data to read\n");
            return;
    }
    if(rpcMap[uniqueIdentifier]) {

        RPCNode node;
        node.data = (char const *)userData;
        node.uniqueIdentifier = uniqueIdentifier;
        node.len = len;

        RPCCallback(node);
    }

}


void RPC::registerRPCs()
{
    registerRPCCallback(139, RPC_InitGame);
}