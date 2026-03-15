#include "PreRTS.h"

#ifdef __APPLE__

#include "MacOSOnlineWSBridge.h"
#include "MacOSDebugLog.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerDefsImplementation.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/ThreadUtils.h"
#include "GameNetwork/GameSpy/PersistentStorageThread.h"

extern "C" {

void GenOnlineWS_InjectChatMessage(const char* nick, const char* text, int isAction) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_MESSAGE;
    resp.nick = nick;
    resp.text = MultiByteToWideCharSingleLine(text);
    resp.message.isPrivate = FALSE;
    resp.message.isAction = isAction ? TRUE : FALSE;
    resp.message.profileID = 0;

    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected chat from '%s': '%s'", nick, text);
}

void GenOnlineWS_InjectMemberListBegin(void) {
    DLOG_NETWORK("WSBridge: member list update received");
}

void GenOnlineWS_InjectMemberJoin(const char* nick, int profileID) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERJOIN;
    resp.nick = nick;
    resp.player.roomType = GroupRoom;
    resp.player.profileID = profileID;
    resp.player.flags = 0;
    resp.player.wins = 0;
    resp.player.losses = 0;
    resp.player.rankPoints = 0;
    resp.player.side = 0;
    resp.player.preorder = 0;
    resp.player.IP = 0;

    TheGameSpyPeerMessageQueue->addResponse(resp);
    
    resp.player.roomType = StagingRoom;
    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected member join '%s' (profileID=%d)", nick, profileID);

    if (profileID > 0 && TheGameSpyPSMessageQueue) {
        PSRequest psReq;
        psReq.requestType = PSRequest::PSREQUEST_READPLAYERSTATS;
        psReq.player.id = profileID;
        TheGameSpyPSMessageQueue->addRequest(psReq);
    }
}

void GenOnlineWS_InjectMemberLeft(const char* nick) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERLEFT;
    resp.nick = nick;
    resp.player.roomType = GroupRoom;
    TheGameSpyPeerMessageQueue->addResponse(resp);

    resp.player.roomType = StagingRoom;
    TheGameSpyPeerMessageQueue->addResponse(resp);

    DLOG_NETWORK("WSBridge: injected member left '%s'", nick);
}

void GenOnlineWS_InjectLobbyUpdate(int lobbyId, const char* name, int playerCount, int maxPlayers) {
    DLOG_NETWORK("WSBridge: lobby update id=%d name='%s' players=%d/%d",
                 lobbyId, name, playerCount, maxPlayers);
}

void GenOnlineWS_InjectStagingRoomUTM(const char* hostName, const char* options) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_ROOMUTM;
    resp.nick = hostName;
    resp.command = "SL";
    resp.commandOptions = options;

    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected StagingRoom UTM for '%s'", hostName);
}

void GenOnlineWS_InjectPlayerUTM(const char* nick, const char* command, const char* options) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERUTM;
    resp.nick = nick;
    resp.command = command;
    resp.commandOptions = options;

    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected PlayerUTM from '%s' cmd='%s' opts='%s'", nick, command, options);
}

void GenOnlineWS_InjectSlotListUTM(const char* hostName, const char* mapPath,
                                    int mapContentsMask, unsigned int mapCRC, unsigned int mapSize,
                                    int seed, int crcInterval, int useStats, unsigned int startingCash,
                                    unsigned short superweaponRestriction, int oldFactionsOnly,
                                    const LobbySlotInfo* slots, int slotCount) {
    if (!TheGameSpyPeerMessageQueue) return;

    char slOptions[2048];
    int pos = 0;

    // SL M= field expects directory-only path (e.g. "invisible boundaries").
    // ParseAsciiStringToGameInfo duplicates last segment and adds .map extension.
    // API returns full path like "invisible boundaries\invisible boundaries.map"
    // so we strip the filename portion (everything after last backslash).
    char cleanMapPath[512] = {0};
    if (mapPath) {
        strncpy(cleanMapPath, mapPath, sizeof(cleanMapPath) - 1);
        char* lastSlash = strrchr(cleanMapPath, '\\');
        if (lastSlash) {
            *lastSlash = '\0';
        }
    }

    pos += snprintf(slOptions + pos, sizeof(slOptions) - pos,
        "US=%d;M=%02x%s;MC=%X;MS=%d;SD=%d;C=%d;SR=%u;SC=%u;O=%c;S=",
        useStats, mapContentsMask, cleanMapPath,
        mapCRC, mapSize, seed, crcInterval,
        superweaponRestriction, startingCash,
        oldFactionsOnly ? 'Y' : 'N');

    for (int i = 0; i < MAX_SLOTS && i < slotCount; ++i) {
        const LobbySlotInfo& s = slots[i];

        if (s.slotState == 5) {
            pos += snprintf(slOptions + pos, sizeof(slOptions) - pos,
                "H%s,0,0,%c%c,%d,%d,%d,%d,0:",
                s.displayName ? s.displayName : "",
                s.isAccepted ? 'T' : 'F',
                s.hasMap ? 'T' : 'F',
                s.color, s.side, s.startPos, s.team);
        } else if (s.slotState == 2) {
            pos += snprintf(slOptions + pos, sizeof(slOptions) - pos, "CE,%d,%d,%d,%d:",
                s.color, s.side, s.startPos, s.team);
        } else if (s.slotState == 3) {
            pos += snprintf(slOptions + pos, sizeof(slOptions) - pos, "CM,%d,%d,%d,%d:",
                s.color, s.side, s.startPos, s.team);
        } else if (s.slotState == 4) {
            pos += snprintf(slOptions + pos, sizeof(slOptions) - pos, "CH,%d,%d,%d,%d:",
                s.color, s.side, s.startPos, s.team);
        } else if (s.slotState == 1) {
            pos += snprintf(slOptions + pos, sizeof(slOptions) - pos, "X:");
        } else {
            pos += snprintf(slOptions + pos, sizeof(slOptions) - pos, "O:");
        }
    }

    for (int i = slotCount; i < MAX_SLOTS; ++i) {
        pos += snprintf(slOptions + pos, sizeof(slOptions) - pos, "X:");
    }

    pos += snprintf(slOptions + pos, sizeof(slOptions) - pos, ";");

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_ROOMUTM;
    resp.nick = hostName ? hostName : "";
    resp.command = "SL";
    resp.commandOptions = slOptions;

    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected SL UTM from host='%s' mapPath='%s' slots=%d",
                 hostName ? hostName : "", mapPath ? mapPath : "", slotCount);
}

void GenOnlineWS_InjectLobbyListBegin(void) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_STAGINGROOMLISTCOMPLETE;
    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: lobby list begin (staging room list complete sentinel)");
}

void GenOnlineWS_InjectLobbyListEntry(int lobbyId, const char* name, const char* hostName,
                                       int playerCount, int maxPlayers, int hasPassword,
                                       const char* mapPath, unsigned int exeCRC, unsigned int iniCRC,
                                       int allowObservers, int useStats,
                                       const LobbyMemberInfo* members, int memberCount) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_STAGINGROOM;
    resp.stagingRoom.id = lobbyId;
    resp.stagingRoom.action = 0; // PEER_ADD
    resp.stagingRoom.isStaging = TRUE;
    resp.stagingRoom.percentComplete = 100;
    resp.stagingRoom.numPlayers = playerCount;
    resp.stagingRoom.maxPlayers = maxPlayers;
    resp.stagingRoom.requiresPassword = hasPassword ? TRUE : FALSE;
    resp.stagingRoom.allowObservers = allowObservers ? TRUE : FALSE;
    resp.stagingRoom.useStats = useStats ? TRUE : FALSE;
    resp.stagingRoom.exeCRC = exeCRC;
    resp.stagingRoom.iniCRC = iniCRC;
    resp.stagingRoom.version = 65536;
    resp.stagingRoom.numObservers = 0;
    resp.stagingRoom.ladderPort = 0;
    std::string qr2MapName;
    if (mapPath && mapPath[0] != '\0') {
        qr2MapName = "Maps/";
        for (const char* p = mapPath; *p; ++p) {
            qr2MapName += (*p == '\\') ? '/' : *p;
        }
    }
    resp.stagingRoomMapName = qr2MapName;
    resp.stagingServerPingString = TheGameSpyInfo ? TheGameSpyInfo->getPingString().str() : "";
    resp.nick = hostName;
    std::string displayName = name ? name : "";
    if (hostName && hostName[0] != '\0') {
        displayName += " (";
        displayName += hostName;
        displayName += ")";
    }
    resp.stagingServerName = MultiByteToWideCharSingleLine(displayName.c_str());

    for (int i = 0; i < MAX_SLOTS; ++i) {
        resp.stagingRoom.profileID[i] = 0;
        resp.stagingRoom.faction[i] = -1;
        resp.stagingRoom.color[i] = -1;
        resp.stagingRoom.wins[i] = 0;
        resp.stagingRoom.losses[i] = 0;
        resp.stagingRoomPlayerNames[i] = "";
    }

    int slotsToFill = (memberCount < MAX_SLOTS) ? memberCount : MAX_SLOTS;
    for (int i = 0; i < slotsToFill; ++i) {
        if (members[i].displayName) {
            resp.stagingRoomPlayerNames[i] = members[i].displayName;
            resp.stagingRoom.profileID[i] = members[i].userId;
            resp.stagingRoom.color[i] = members[i].color;
            resp.stagingRoom.faction[i] = members[i].faction;
        }
    }

    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected lobby entry id=%d display='%s' map='%s' %d/%d crc=%u/%u members=%d",
                 lobbyId, displayName.c_str(), mapPath ? mapPath : "", playerCount, maxPlayers, exeCRC, iniCRC, memberCount);
}

void GenOnlineWS_InjectGroupRoom(int groupId, const char* name,
                                  int numWaiting, int maxWaiting, int numGames, int numPlaying) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_GROUPROOM;
    resp.groupRoom.id = groupId;
    resp.groupRoom.numWaiting = numWaiting;
    resp.groupRoom.maxWaiting = maxWaiting;
    resp.groupRoom.numGames = numGames;
    resp.groupRoom.numPlaying = numPlaying;
    resp.groupRoomName = name ? name : "";

    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected group room id=%d '%s' waiting=%d games=%d",
                 groupId, name ? name : "", numWaiting, numGames);
}

void GenOnlineWS_InjectJoinStagingRoomSuccess(int lobbyId) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_JOINSTAGINGROOM;
    resp.joinStagingRoom.id = lobbyId;
    resp.joinStagingRoom.ok = TRUE;
    resp.joinStagingRoom.isHostPresent = TRUE;
    resp.joinStagingRoom.result = PEERJoinSuccess;

    GameSpyStagingRoom *room = TheGameSpyInfo ? TheGameSpyInfo->getCurrentStagingRoom() : nullptr;
    if (room) {
        for (int i = 0; i < MAX_SLOTS; ++i) {
            const GameSlot *slot = room->getConstSlot(i);
            if (slot && slot->isHuman()) {
                AsciiString name;
                name.translate(slot->getName());
                resp.stagingRoomPlayerNames[i] = name.str();
            }
        }
    }

    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected join staging room SUCCESS id=%d", lobbyId);
}

void GenOnlineWS_InjectJoinStagingRoomSuccessWithPlayers(int lobbyId, const char** playerNames, int playerCount) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_JOINSTAGINGROOM;
    resp.joinStagingRoom.id = lobbyId;
    resp.joinStagingRoom.ok = TRUE;
    resp.joinStagingRoom.isHostPresent = (playerCount > 0) ? TRUE : FALSE;
    resp.joinStagingRoom.result = PEERJoinSuccess;

    for (int i = 0; i < MAX_SLOTS && i < playerCount; ++i) {
        if (playerNames[i]) {
            resp.stagingRoomPlayerNames[i] = playerNames[i];
        }
    }

    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected join staging room SUCCESS id=%d with %d players", lobbyId, playerCount);
}

void GenOnlineWS_InjectJoinStagingRoomFailure(int lobbyId, int reason) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_JOINSTAGINGROOM;
    resp.joinStagingRoom.id = lobbyId;
    resp.joinStagingRoom.ok = FALSE;
    resp.joinStagingRoom.isHostPresent = FALSE;
    resp.joinStagingRoom.result = reason;
    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected join staging room FAILURE id=%d reason=%d", lobbyId, reason);
}

void GenOnlineWS_InjectQMStatus(int qmStatus) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_QUICKMATCHSTATUS;
    resp.qmStatus.status = (QMStatus)qmStatus;
    resp.qmStatus.poolSize = 0;
    resp.qmStatus.mapIdx = 0;
    resp.qmStatus.seed = 0;
    for (int i = 0; i < MAX_SLOTS; ++i) {
        resp.qmStatus.IP[i] = 0;
        resp.qmStatus.side[i] = 0;
        resp.qmStatus.color[i] = 0;
        resp.qmStatus.nat[i] = 0;
    }

    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected QM status=%d", qmStatus);
}

void GenOnlineWS_InjectGameStart() {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_GAMESTART;

    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected GAMESTART (from QM server)");
}

void GenOnlineWS_InjectQMJoinLobby(long long lobbyId) {
    DLOG_NETWORK("WSBridge: QM server assigned lobby_id=%lld — requesting join", lobbyId);
    extern void GenOnlineLobby_Join(int lobbyId, const char* password);
    GenOnlineLobby_Join((int)lobbyId, "");
}

} // extern "C"

#include "GameNetwork/GameSpy/BuddyThread.h"

void GenOnlineWS_InjectBuddyMessage(int profileID, const char* nick, const char* text) {
    if (!TheGameSpyBuddyMessageQueue) return;

    BuddyResponse resp;
    resp.buddyResponseType = BuddyResponse::BUDDYRESPONSE_MESSAGE;
    resp.profile = profileID;
    strlcpy(resp.arg.message.nick, nick, GP_NICK_LEN);
    std::wstring ws = MultiByteToWideCharSingleLine(text);
    wcslcpy(resp.arg.message.text, ws.c_str(), MAX_BUDDY_CHAT_LEN);
    resp.arg.message.date = 0;

    TheGameSpyBuddyMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected buddy message from '%s' (profileID=%d)", nick, profileID);
}

void GenOnlineWS_InjectBuddyRequest(int profileID, const char* nick, const char* reason) {
    if (!TheGameSpyBuddyMessageQueue) return;

    BuddyResponse resp;
    resp.buddyResponseType = BuddyResponse::BUDDYRESPONSE_REQUEST;
    resp.profile = profileID;
    strlcpy(resp.arg.request.nick, nick, GP_NICK_LEN);
    resp.arg.request.email[0] = '\0';
    resp.arg.request.countrycode[0] = '\0';
    std::wstring ws = MultiByteToWideCharSingleLine(reason);
    wcslcpy(resp.arg.request.text, ws.c_str(), GP_REASON_LEN);

    TheGameSpyBuddyMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected buddy request from '%s' (profileID=%d)", nick, profileID);
}

void GenOnlineWS_InjectBuddyStatus(int profileID, const char* nick, int gpStatus, const char* statusString, const char* location) {
    if (!TheGameSpyBuddyMessageQueue) return;

    BuddyResponse resp;
    resp.buddyResponseType = BuddyResponse::BUDDYRESPONSE_STATUS;
    resp.profile = profileID;
    strlcpy(resp.arg.status.nick, nick, GP_NICK_LEN);
    resp.arg.status.email[0] = '\0';
    resp.arg.status.countrycode[0] = '\0';
    resp.arg.status.status = (GPEnum)gpStatus;
    strlcpy(resp.arg.status.statusString, statusString ? statusString : "", GP_STATUS_STRING_LEN);
    strlcpy(resp.arg.status.location, location ? location : "", GP_LOCATION_STRING_LEN);

    TheGameSpyBuddyMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected buddy status for '%s' (profileID=%d) status=%d", nick, profileID, gpStatus);
}

#endif // __APPLE__
