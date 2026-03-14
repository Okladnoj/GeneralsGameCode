#pragma once

#ifdef __APPLE__

#ifdef __cplusplus
extern "C" {
#endif

void GenOnlineLobby_Create(const char* gameName, const char* mapName,
    const char* mapPath, int mapOfficial, int maxPlayers, const char* password,
    int allowObservers, int useStats, unsigned int startingCash,
    unsigned short preferredPort, unsigned short maxCamHeight,
    unsigned int exeCRC, unsigned int iniCRC);

void GenOnlineLobby_Join(int lobbyId, const char* password);

void GenOnlineLobby_Leave(void);

int GenOnlineLobby_IsRequestInFlight(void);

int GenOnlineLobby_GetLastResult(void);

int GenOnlineLobby_GetCreatedLobbyId(void);

void GenOnlineLobby_ResetResult(void);

void GenOnline_FetchMOTD(void);

void GenOnlineLobby_FetchDetails(int lobbyId);
void GenOnlineLobby_FetchList(void);
void GenOnlineLobby_FetchRooms(void);

unsigned int GenOnlineP2P_ResolveLocalIP(void);
void GenOnlineP2P_ApplyPeerAddresses(void* slotArray, int slotCount);

#ifdef __cplusplus
}
#endif

#endif // __APPLE__
