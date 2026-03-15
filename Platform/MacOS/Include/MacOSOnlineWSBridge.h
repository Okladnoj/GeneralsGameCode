#pragma once

#ifdef __APPLE__

#define MAX_LOBBY_MEMBERS 8

struct LobbyMemberInfo {
    const char* displayName;
    int userId;
    int slotState;
    int team;
    int color;
    int faction;
};

struct LobbySlotInfo {
    const char* displayName;
    int slotState;
    int side;
    int color;
    int team;
    int startPos;
    int hasMap;
    int isAccepted;
};

#ifdef __cplusplus
extern "C" {
#endif

void GenOnlineWS_InjectChatMessage(const char* nick, const char* text, int isAction);
void GenOnlineWS_InjectStagingRoomUTM(const char* hostName, const char* options);
void GenOnlineWS_InjectMemberListBegin(void);
void GenOnlineWS_InjectMemberJoin(const char* nick, int profileID);
void GenOnlineWS_InjectMemberLeft(const char* nick);
void GenOnlineWS_InjectLobbyUpdate(int lobbyId, const char* name, int playerCount, int maxPlayers);
void GenOnlineWS_InjectSlotListUTM(const char* hostName, const char* mapPath,
    int mapContentsMask, unsigned int mapCRC, unsigned int mapSize,
    int seed, int crcInterval, int useStats, unsigned int startingCash,
    unsigned short superweaponRestriction, int oldFactionsOnly,
    const LobbySlotInfo* slots, int slotCount);
void GenOnlineWS_InjectLobbyListBegin(void);
void GenOnlineWS_InjectLobbyListEntry(int lobbyId, const char* name, const char* hostName,
    int playerCount, int maxPlayers, int hasPassword,
    const char* mapPath, unsigned int exeCRC, unsigned int iniCRC,
    int allowObservers, int useStats,
    const LobbyMemberInfo* members, int memberCount);
void GenOnlineWS_InjectGroupRoom(int groupId, const char* name,
    int numWaiting, int maxWaiting, int numGames, int numPlaying);

void GenOnlineWS_InjectJoinStagingRoomSuccess(int lobbyId);
void GenOnlineWS_InjectJoinStagingRoomSuccessWithPlayers(int lobbyId, const char** playerNames, int playerCount);
void GenOnlineWS_InjectJoinStagingRoomFailure(int lobbyId, int reason);

#ifdef __cplusplus
}
#endif

#endif // __APPLE__
