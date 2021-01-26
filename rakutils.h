#ifndef RAKUTILS_H
#define RAKUTILS_H



#include "RakNet/BitStream.h"
#include "RakNet/StringCompressor.h"
#include "RakNet/PluginInterface.h"

#include "RakNet/DS_HuffmanEncodingTree.h"
#include "RakNet/DS_RangeList.h"
#include "RakNet/DS_BPlusTree.h"
#include "RakNet/InternalPacket.h"
#include "RakNet/InternalPacketPool.h"

#include <assert.h>

#define RakAssert(x) 

extern unsigned histogramPlossCount, histogramAckCount;
extern long long unreliableTimeout;

// Cut
struct RakNetStatisticsStruct
{
   
    ///  Number of messages sent (high, medium, low priority)
    unsigned messagesSent[ NUMBER_OF_PRIORITIES ];
    ///  Number of data bits used for user messages
    unsigned messageDataBitsSent[ NUMBER_OF_PRIORITIES ];
    ///  Number of total bits used for user messages, including headers
    unsigned messageTotalBitsSent[ NUMBER_OF_PRIORITIES ];
    
    ///  Number of packets sent containing only acknowledgements
    unsigned acknowlegementsSent;
    ///  Number of acknowledgements waiting to be sent
  
    ///  Number of acknowledgements bits sent
    unsigned acknowlegementBitsSent;
    
};

#define MAXIMUM_MTU_SIZE 576

extern InternalPacketPool internalPacketPool;

extern DataStructures::RangeList<unsigned short> acknowlegements;
extern RakNetStatisticsStruct statistics;



enum PacketEnumeration
{
    ID_INTERNAL_PING = 6,
    ID_PING,
    ID_PING_OPEN_CONNECTIONS,
    ID_CONNECTED_PONG,
    ID_REQUEST_STATIC_DATA,
    ID_CONNECTION_REQUEST,
    ID_AUTH_KEY,
    ID_BROADCAST_PINGS = 14,
    ID_SECURED_CONNECTION_RESPONSE,
    ID_SECURED_CONNECTION_CONFIRMATION,
    ID_RPC_MAPPING,
    ID_SET_RANDOM_NUMBER_SEED = 19,
    ID_RPC,
    ID_RPC_REPLY,
    ID_DETECT_LOST_CONNECTIONS = 23,
    ID_OPEN_CONNECTION_REQUEST,
    ID_OPEN_CONNECTION_REPLY,
    ID_OPEN_CONNECTION_COOKIE,
    ID_RSA_PUBLIC_KEY_MISMATCH = 28,
    ID_CONNECTION_ATTEMPT_FAILED,
    ID_NEW_INCOMING_CONNECTION = 30,
    ID_NO_FREE_INCOMING_CONNECTIONS = 31,
    ID_DISCONNECTION_NOTIFICATION,	
    ID_CONNECTION_LOST,
    ID_CONNECTION_REQUEST_ACCEPTED,
    ID_CONNECTION_BANNED = 36,
    ID_INVALID_PASSWORD,
    ID_MODIFIED_PACKET,
    ID_PONG,
    ID_TIMESTAMP,
    ID_RECEIVED_STATIC_DATA,
    ID_REMOTE_DISCONNECTION_NOTIFICATION,
    ID_REMOTE_CONNECTION_LOST,
    ID_REMOTE_NEW_INCOMING_CONNECTION,
    ID_REMOTE_EXISTING_CONNECTION,
    ID_REMOTE_STATIC_DATA,
    ID_ADVERTISE_SYSTEM = 55,

    ID_PLAYER_SYNC = 207,
    ID_MARKERS_SYNC = 208,
    ID_UNOCCUPIED_SYNC = 209,
    ID_TRAILER_SYNC = 210,
    ID_PASSENGER_SYNC = 211,
    ID_SPECTATOR_SYNC = 212,
    ID_AIM_SYNC = 203,
    ID_VEHICLE_SYNC = 200,
    ID_RCON_COMMAND = 201,
    ID_RCON_RESPONCE = 202,
    ID_WEAPONS_UPDATE = 204,
    ID_STATS_UPDATE = 205,
    ID_BULLET_SYNC = 206,
};

class RakUtils {
private:
    long long nextAckTime;
public:
    unsigned GenerateDatagram(
        RakNet::BitStream *output, 
        const InternalPacket &IPs, 
        int MTUSize, 
        RakNetTimeNS time);

    void insertPacket(
        InternalPacket *internalPacket, 
        RakNetTimeNS time, 
        bool makeCopyOfInternalPacket, 
        bool firstResend);

    int signBSFromIP(
        RakNet::BitStream *bs, 
        const InternalPacket *const internalPacket);



    InternalPacket *getIPFromBS(RakNet::BitStream *bs, RakNetTimeNS time);
    int getBSHeaderLen(const InternalPacket *const internalPacket);
};


#endif