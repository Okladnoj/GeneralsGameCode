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

// field=0 LOBBY_MAP (Owner only)
void GenOnlineLobby_UpdateMap(const char* map, const char* mapPath,
    int mapOfficial, int maxPlayers);

// field=1 MY_SIDE (Anyone)
void GenOnlineLobby_UpdateSide(int side, int startPos);

// field=2 MY_COLOR (Anyone)
void GenOnlineLobby_UpdateColor(int color);

// field=3 MY_START_POS (Anyone)
void GenOnlineLobby_UpdateStartPos(int startPos);

// field=4 MY_TEAM (Anyone)
void GenOnlineLobby_UpdateTeam(int team);

// field=5 LOBBY_STARTING_CASH (Owner only)
void GenOnlineLobby_UpdateStartingCash(unsigned int cash);

// field=6 LOBBY_LIMIT_SUPERWEAPONS (Owner only)
void GenOnlineLobby_UpdateLimitSuperweapons(int limit);

// field=7 HOST_ACTION_FORCE_START (Owner only)
void GenOnlineLobby_ForceStart(void);

// field=8 LOCAL_PLAYER_HAS_MAP (Anyone)
void GenOnlineLobby_UpdateHasMap(int hasMap);

// field=11 HOST_ACTION_KICK_USER (Owner only)
void GenOnlineLobby_KickUser(long long userId);

// field=12 HOST_ACTION_SET_SLOT_STATE (Owner only)
void GenOnlineLobby_SetSlotState(int slotIndex, int slotState);

// field=13 AI_SIDE (Owner only)
void GenOnlineLobby_UpdateAISide(int slot, int side, int startPos);

// field=14 AI_COLOR (Owner only)
void GenOnlineLobby_UpdateAIColor(int slot, int color);

// field=15 AI_TEAM (Owner only)
void GenOnlineLobby_UpdateAITeam(int slot, int team);

// field=16 AI_START_POS (Owner only)
void GenOnlineLobby_UpdateAIStartPos(int slot, int startPos);

// field=17 MAX_CAMERA_HEIGHT (Owner only)
void GenOnlineLobby_UpdateMaxCameraHeight(unsigned short maxCamHeight);

// field=18 JOINABILITY (Owner only)
void GenOnlineLobby_UpdateJoinability(int joinability);

// WS msg_id=5 NETWORK_ROOM_MARK_READY
void GenOnlineLobby_MarkReady(int ready);

unsigned int GenOnlineP2P_ResolveLocalIP(void);
void GenOnlineP2P_ApplyPeerAddresses(void* slotArray, int slotCount);
long long GenOnlineP2P_GetSlotUserId(int slotIndex);
int GenOnlineP2P_GetPeerAddressForUserId(long long userId, unsigned int* outIp, unsigned short* outPort);

#ifdef __cplusplus
}
#endif

#endif // __APPLE__
