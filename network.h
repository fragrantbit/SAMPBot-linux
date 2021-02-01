#ifndef NETWORK_H
#define NETWORK_H


#include <sys/socket.h>
#include <sys/types.h> 
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <map>
#include <string>
#include <iostream>
#include <vector>


#include "rakutils.h"
#include "main.h"

#define NETGAME_VERSION 4057
#define MAX_BLOCKS 50


class Network : public RakUtils {
private:
    int _sockfd;

    // Remote
    struct PeerInfo {
        char const *peerAddr;
        int peerPort;
        struct sockaddr_in peer;
    };

    struct PeerInfo *remote;
    struct sockaddr_in local;

    pthread_t tickThread;
    pthread_t blocksWrapperThread;

    bool _cookied;

    bool _connected;
    unsigned int _uiSvrChallenge;


    struct DataBlock {
        char            packetId; // The first byte.
        unsigned char   *content; // All the data after.

        int             len; // Count of bytes after the first one. 
        int             blockIdentifier;
        int             bytesCopied;


        DataBlock() {
            bytesCopied = 0;
        }

        void write(char const byte);
    };

    std::vector<struct DataBlock *> _blocks;

public:

    Network(char const *addr, int port) {
        remote = new PeerInfo;

        remote->peerAddr = addr;
        remote->peerPort = port;  

        _cookied = false;
        _connected = false;

        chargeKit();
        
    }

    Network() { }
    Network(void *&) {}
    struct PeerInfo *getRemote() {
        return this->remote;
    }
    int getSockFd() {
        return this->_sockfd;
    }
    bool isConnected() {
        return this->_connected;
    }


    void *getThis() { return this; }

    PlayerID externalID;
    PlayerIndex playerIndex;

    void requestCookie();
    void sendCookie(char const *&data, unsigned short cookie);
    void initRequest();
    void sendConnectionRequest();


    void setUISvrChallenge(unsigned int src) {
        _uiSvrChallenge = src;
    }

    unsigned int getUISvrChallenge() {
        return _uiSvrChallenge;
    }
    void chargeKit();

    int sendTo(char const *data, int length, bool handle = false);
    int recvFrom(char *data);

    void processBlock(struct DataBlock &block);

    void *networkUpdateLoop();

    void listener() const;

    void pingRemoteSystem();
    void CallsHandler();
    

    InternalPacket *makeIPacket(
        const RakNet::BitStream &bs, 
        PacketReliability reliability, 
        PacketPriority priority,
        long long creationTime);


    void makePacket(RakNet::BitStream &bs);

    void makePacket(
        RakNet::BitStream &bs, PacketReliability reliability,
        PacketPriority priority, RakNetTime timeMS);
    
    void unpackNetworkPacket(char const *data, int len);


    void onInternalPingLoop();


    void createBlock(char *data, int len);
    void insertBlock(struct DataBlock &block);

    void *blocksWrapper();

};


#endif