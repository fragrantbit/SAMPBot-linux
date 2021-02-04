#include "rpc.h"
#include "callbacks.h"
#include "main.h"
#include "common.h"


void RPC_InitGame(const RPCNode &rpcNode) {

    RakNet::BitStream initGameBS((unsigned char *)rpcNode.data, rpcNode.len, false);

    bool _zoneNames, _useCJWalk, _allowWeapons, _limitChat,
        _stuntBonus, _nameTagLOS, _manualVehiceEngineAndLight,
        _showPlayerTags, _lanMode, _instagib, _disableEnterExits;

    float globalChatRadius, nameTagDrawDistance,
        gravity;

    int spawnsAvailable, showPlayerMarkers, deathDropMoney,
        netModeNormalOnfootSendRate,
        netModeNormalIncarSendRate,
        netModeFiringSendRate,
        netModeSendMultiplier;

    unsigned short myPlayerID;
    unsigned char unk, strLen, weather, worldTime, lagCompensation;

    unsigned char vehicleModels[212];

    char hostname[220];

    initGameBS.ReadCompressed(_zoneNames);
    initGameBS.ReadCompressed(_useCJWalk);
    initGameBS.ReadCompressed(_allowWeapons);
    initGameBS.ReadCompressed(_limitChat);
        
    initGameBS.Read(globalChatRadius);

    initGameBS.ReadCompressed(_stuntBonus);
    initGameBS.Read(nameTagDrawDistance);

    initGameBS.ReadCompressed(_disableEnterExits);
    initGameBS.ReadCompressed(_nameTagLOS);

    initGameBS.ReadCompressed(_manualVehiceEngineAndLight);

    initGameBS.Read(spawnsAvailable);
    initGameBS.Read(myPlayerID);
    initGameBS.ReadCompressed(_showPlayerTags);
    initGameBS.Read(showPlayerMarkers);

    initGameBS.Read(worldTime);
    initGameBS.Read(weather);
    initGameBS.Read(gravity);
    initGameBS.ReadCompressed(_lanMode);
    initGameBS.Read(deathDropMoney);
    initGameBS.ReadCompressed(_instagib);

    initGameBS.Read(netModeNormalOnfootSendRate);
    initGameBS.Read(netModeNormalIncarSendRate);
    initGameBS.Read(netModeFiringSendRate);
    initGameBS.Read(netModeSendMultiplier);

    initGameBS.Read(lagCompensation);
    initGameBS.Read(unk);
    initGameBS.Read(unk);
    initGameBS.Read(unk);

    initGameBS.Read(strLen);
        

    if(strLen) {
        initGameBS.Read(hostname, strLen);
    }
    hostname[strLen] = 0;
    initGameBS.Read((char *)&vehicleModels[0],212);

    _log("Joined %s", hostname);
    printf("\n\n");
}