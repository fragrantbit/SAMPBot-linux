# SAMPBot-linux

This is a basic implementation of bot for SA:MP, written for linux and using BitStream and a few other basic functions from RakNet.

The first purpose was to compile RakSAMP on linux. And after figuring out it'd be much simpler to start writing such thing from scratch I decided to start doing that.

As for now, this basic code joins a sa-mp server with chosen nickname and, shortly after, disconnects.

Todo:
  - thread pools
  - not all network packets sent by server are acknowledged - server doesn't receive some responses back from the client, and it might be the reason the server disconnects the client shortly after connection.

