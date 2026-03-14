#pragma once

#ifdef __APPLE__

#ifdef __cplusplus
extern "C" {
#endif

void GenOnlineWS_InjectChatMessage(const char* nick, const char* text, int isAction);
void GenOnlineWS_InjectMemberListBegin(void);
void GenOnlineWS_InjectMemberJoin(const char* nick, int profileID);
void GenOnlineWS_InjectMemberLeft(const char* nick);
void GenOnlineWS_InjectLobbyUpdate(int lobbyId, const char* name, int playerCount, int maxPlayers);
void GenOnlineWS_InjectLobbyListBegin(void);
void GenOnlineWS_InjectLobbyListEntry(int lobbyId, const char* name, const char* hostName,
    int playerCount, int maxPlayers, int hasPassword,
    const char* mapPath, unsigned int exeCRC, unsigned int iniCRC,
    int allowObservers, int useStats);
void GenOnlineWS_InjectGroupRoom(int groupId, const char* name,
    int numWaiting, int maxWaiting, int numGames, int numPlaying);

#ifdef __cplusplus
}
#endif

#endif // __APPLE__
