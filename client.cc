#include "client.h"
#include <cstdio>

#include "common.h"
#include "samp_encr.h"
#include "RakNet/GetTime.h"

#include "tables.h"

void Client::join(char const *data, int len)
{
    printf("\n");
    char const *botname = getBotName();
    _log("Connected. Joining the game as %s ...", botname);

    RakNet::BitStream outcomingBS;


    int iVersion = NETGAME_VERSION;
    unsigned int challengeResponse = bundle->networkManager()->getUISvrChallenge()
                                            ^ iVersion;


    

    char auth_bs[43] = { 0 };
    gen_gpci(auth_bs, 0x3e9);

    unsigned char authBSLen;
    authBSLen = (unsigned char)strlen(auth_bs);

    unsigned char botNameLen = (unsigned char)strlen(botname);
    unsigned char mod = 1;

    outcomingBS.Write(iVersion);
    outcomingBS.Write(mod);
    outcomingBS.Write(botNameLen);
    outcomingBS.Write(botname, botNameLen);
    outcomingBS.Write(challengeResponse);
    outcomingBS.Write(authBSLen);
    outcomingBS.Write(auth_bs, authBSLen);

    bundle->rpcManager()->sendRPC(
        25,
        &outcomingBS,
        HIGH_PRIORITY,
        RELIABLE);
}

void Client::sendAuthKey(char const *data, int len)
{
    RakNet::BitStream bsAuth((unsigned char *)data, len, false);
    RakNet::BitStream bs;

    unsigned char authLen;
    unsigned char authKeyLen;

    char auth[260];
    char authKey[260];

    // bsAuth.IgnoreBits(8);
    bsAuth.Read(authLen);
    bsAuth.Read(auth, authLen);

    auth[authLen] = '\0';

    gen_auth_key(authKey, auth);

    authKeyLen = (unsigned char)strlen(authKey);

    bs.Write((unsigned char)ID_AUTH_KEY);
    bs.Write((unsigned char)authKeyLen);
    bs.Write(authKey, authKeyLen);

  
    bundle->networkManager()->makePacket(bs);

    _datalog("Sending auth key :: %s -> %s", auth, authKey);

    bundle->networkManager()->sendTo(
        (char *)bs.GetData(),
        bs.GetNumberOfBytesUsed(),
        false);

    _log("Auth key sent");
    _datalog(true);
}

void Client::onConnectionAccepted(char const *data, int len,
        unsigned int externalBinaryAddress, unsigned short port)
{
    _log("Connection request accepted");



    join(data, len);
    // _isConnected = true;


}