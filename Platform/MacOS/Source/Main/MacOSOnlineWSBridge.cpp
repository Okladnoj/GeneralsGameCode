#include "PreRTS.h"

#ifdef __APPLE__

#include "MacOSOnlineWSBridge.h"
#include "MacOSDebugLog.h"
#include "GameNetwork/GameSpy/PeerDefs.h"
#include "GameNetwork/GameSpy/PeerDefsImplementation.h"
#include "GameNetwork/GameSpy/PeerThread.h"
#include "GameNetwork/GameSpy/ThreadUtils.h"

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
    DLOG_NETWORK("WSBridge: injected member join '%s' (profileID=%d)", nick, profileID);
}

void GenOnlineWS_InjectMemberLeft(const char* nick) {
    if (!TheGameSpyPeerMessageQueue) return;

    PeerResponse resp;
    resp.peerResponseType = PeerResponse::PEERRESPONSE_PLAYERLEFT;
    resp.nick = nick;
    resp.player.roomType = GroupRoom;

    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected member left '%s'", nick);
}

void GenOnlineWS_InjectLobbyUpdate(int lobbyId, const char* name, int playerCount, int maxPlayers) {
    DLOG_NETWORK("WSBridge: lobby update id=%d name='%s' players=%d/%d",
                 lobbyId, name, playerCount, maxPlayers);
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
                                       int allowObservers, int useStats) {
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
    resp.stagingRoomMapName = mapPath ? mapPath : "";
    resp.nick = hostName;
    resp.stagingServerName = MultiByteToWideCharSingleLine(name);
    resp.stagingRoomPlayerNames[0] = hostName;

    TheGameSpyPeerMessageQueue->addResponse(resp);
    DLOG_NETWORK("WSBridge: injected lobby entry id=%d '%s' host='%s' map='%s' %d/%d crc=%u/%u",
                 lobbyId, name, hostName, mapPath ? mapPath : "", playerCount, maxPlayers, exeCRC, iniCRC);
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

} // extern "C"

#endif // __APPLE__
