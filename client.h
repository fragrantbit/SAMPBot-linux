#ifndef CLIENT_H
#define CLIENT_H

#include "main.h"

class Client {

    char const *_botname;

public:
   
    Client(char const *botname) : _botname(botname)
    {

    }
    void join(char const *data, int len);
    void sendAuthKey(char const *data, int len);

    void onConnectionAccepted(char const *data, int len, 
        unsigned int externalBinaryAddress, unsigned short port);


    char const *getBotName() {
        return _botname;
    }
};

#endif




