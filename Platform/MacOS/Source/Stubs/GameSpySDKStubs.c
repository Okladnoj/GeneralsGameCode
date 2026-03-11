// GameSpySDKStubs.c — Stub implementations for GameSpy SDK C API on macOS
// Same approach as DX8 → Metal: headers from SDK, implementations are stubs.
// SDK itself does not compile on macOS (gsdebug.c va_start issue, etc.)

#include <string.h>
#include <time.h>

#include "gscommon.h"
#include "gp/gp.h"
#include "peer/peer.h"
#include "serverbrowsing/sb_serverbrowsing.h"
#include "gstats/gstats.h"
#include "gstats/gpersist.h"
#include "chat/chat.h"
#include "qr2/qr2.h"
#include "qr2/qr2regkeys.h"

// ══════════════════════════════════════════════════════
//  GLOBALS
// ══════════════════════════════════════════════════════

char gcd_gamename[256] = {0};
char gcd_secret_key[256] = {0};
char GPConnectionManagerHostname[64] = {0};
char GPSearchManagerHostname[64] = {0};

// ══════════════════════════════════════════════════════
//  GP (Presence & Messaging)
// ══════════════════════════════════════════════════════

GPResult gpInitialize(GPConnection* connection, int productID, int namespaceID, int partnerID) {
    (void)connection; (void)productID; (void)namespaceID; (void)partnerID;
    return GP_NO_ERROR;
}

void gpDestroy(GPConnection* connection) {
    (void)connection;
}

GPResult gpProcess(GPConnection* connection) {
    (void)connection;
    return GP_NO_ERROR;
}

GPResult gpSetCallback(GPConnection* connection, GPEnum func, GPCallback callback, void* param) {
    (void)connection; (void)func; (void)callback; (void)param;
    return GP_NO_ERROR;
}

GPResult gpConnectA(GPConnection* connection,
                    const gsi_char nick[GP_NICK_LEN],
                    const gsi_char email[GP_EMAIL_LEN],
                    const gsi_char password[GP_PASSWORD_LEN],
                    GPEnum firewall, GPEnum blocking,
                    GPCallback callback, void* param) {
    (void)connection; (void)nick; (void)email; (void)password;
    (void)firewall; (void)blocking; (void)callback; (void)param;
    return GP_NO_ERROR;
}

GPResult gpConnectNewUserA(GPConnection* connection,
                           const gsi_char nick[GP_NICK_LEN],
                           const gsi_char uniquenick[GP_UNIQUENICK_LEN],
                           const gsi_char email[GP_EMAIL_LEN],
                           const gsi_char password[GP_PASSWORD_LEN],
                           const gsi_char cdkey[GP_CDKEY_LEN],
                           GPEnum firewall, GPEnum blocking,
                           GPCallback callback, void* param) {
    (void)connection; (void)nick; (void)uniquenick; (void)email;
    (void)password; (void)cdkey; (void)firewall; (void)blocking;
    (void)callback; (void)param;
    return GP_NO_ERROR;
}

void gpDisconnect(GPConnection* connection) {
    (void)connection;
}

GPResult gpIsConnected(GPConnection* connection, GPEnum* connected) {
    (void)connection;
    if (connected) *connected = GP_NOT_CONNECTED;
    return GP_NO_ERROR;
}

GPResult gpGetInfo(GPConnection* connection, GPProfile profile,
                   GPEnum checkCache, GPEnum blocking,
                   GPCallback callback, void* param) {
    (void)connection; (void)profile; (void)checkCache;
    (void)blocking; (void)callback; (void)param;
    return GP_NO_ERROR;
}

GPResult gpGetBuddyStatus(GPConnection* connection, int index, GPBuddyStatus* status) {
    (void)connection; (void)index;
    if (status) memset(status, 0, sizeof(*status));
    return GP_NO_ERROR;
}

GPResult gpSetInfoMask(GPConnection* connection, GPEnum mask) {
    (void)connection; (void)mask;
    return GP_NO_ERROR;
}

GPResult gpSetStatusA(GPConnection* connection, GPEnum status,
                      const gsi_char statusString[GP_STATUS_STRING_LEN],
                      const gsi_char locationString[GP_LOCATION_STRING_LEN]) {
    (void)connection; (void)status; (void)statusString; (void)locationString;
    return GP_NO_ERROR;
}

GPResult gpSendBuddyMessageA(GPConnection* connection, GPProfile profile, const gsi_char* message) {
    (void)connection; (void)profile; (void)message;
    return GP_NO_ERROR;
}

GPResult gpSendBuddyRequestA(GPConnection* connection, GPProfile profile, const gsi_char reason[GP_REASON_LEN]) {
    (void)connection; (void)profile; (void)reason;
    return GP_NO_ERROR;
}

GPResult gpAuthBuddyRequest(GPConnection* connection, GPProfile profile) {
    (void)connection; (void)profile;
    return GP_NO_ERROR;
}

GPResult gpDenyBuddyRequest(GPConnection* connection, GPProfile profile) {
    (void)connection; (void)profile;
    return GP_NO_ERROR;
}

GPResult gpDeleteBuddy(GPConnection* connection, GPProfile profile) {
    (void)connection; (void)profile;
    return GP_NO_ERROR;
}

GPResult gpDeleteProfile(GPConnection* connection, GPCallback callback, void* param) {
    (void)connection; (void)callback; (void)param;
    return GP_NO_ERROR;
}

// ══════════════════════════════════════════════════════
//  PEER
// ══════════════════════════════════════════════════════

PEER peerInitialize(PEERCallbacks* callbacks) {
    (void)callbacks;
    return NULL;
}

void peerShutdown(PEER peer) {
    (void)peer;
}

void peerDisconnect(PEER peer) {
    (void)peer;
}

void peerThink(PEER peer) {
    (void)peer;
}

PEERBool peerIsConnected(PEER peer) {
    (void)peer;
    return PEERFalse;
}

void peerConnectA(PEER peer, const gsi_char* nick, int profileID,
                  peerNickErrorCallback nickErrorCallback,
                  peerConnectCallback connectCallback,
                  void* param, PEERBool blocking) {
    (void)peer; (void)nick; (void)profileID;
    (void)nickErrorCallback; (void)connectCallback;
    (void)param; (void)blocking;
}

PEERBool peerSetTitleA(PEER peer, const gsi_char* title,
                       const gsi_char* qrSecretKey,
                       const gsi_char* sbName,
                       const gsi_char* sbSecretKey,
                       int sbGameVersion, int sbMaxUpdates,
                       PEERBool natNegotiate,
                       PEERBool pingRooms[NumRooms],
                       PEERBool crossPingRooms[NumRooms]) {
    (void)peer; (void)title; (void)qrSecretKey; (void)sbName;
    (void)sbSecretKey; (void)sbGameVersion; (void)sbMaxUpdates;
    (void)natNegotiate; (void)pingRooms; (void)crossPingRooms;
    return PEERFalse;
}

void peerRetryWithNickA(PEER peer, const gsi_char* nick) {
    (void)peer; (void)nick;
}

void peerJoinGroupRoom(PEER peer, int groupID,
                       peerJoinRoomCallback callback,
                       void* param, PEERBool blocking) {
    (void)peer; (void)groupID; (void)callback; (void)param; (void)blocking;
}

void peerJoinStagingRoomA(PEER peer, SBServer server,
                          const gsi_char password[PEER_PASSWORD_LEN],
                          peerJoinRoomCallback callback,
                          void* param, PEERBool blocking) {
    (void)peer; (void)server; (void)password;
    (void)callback; (void)param; (void)blocking;
}

void peerLeaveRoomA(PEER peer, RoomType roomType, const gsi_char* reason) {
    (void)peer; (void)roomType; (void)reason;
}

void peerCreateStagingRoomWithSocketA(PEER peer, const gsi_char* name,
                                      int maxPlayers,
                                      const gsi_char password[PEER_PASSWORD_LEN],
                                      SOCKET socket, unsigned short port,
                                      peerJoinRoomCallback callback,
                                      void* param, PEERBool blocking) {
    (void)peer; (void)name; (void)maxPlayers; (void)password;
    (void)socket; (void)port; (void)callback; (void)param; (void)blocking;
}

void peerListGroupRoomsA(PEER peer, const gsi_char* fields,
                         peerListGroupRoomsCallback callback,
                         void* param, PEERBool blocking) {
    (void)peer; (void)fields; (void)callback; (void)param; (void)blocking;
}

void peerStartListingGamesA(PEER peer, const unsigned char* fields,
                            int numFields, const gsi_char* filter,
                            peerListingGamesCallback callback,
                            void* param) {
    (void)peer; (void)fields; (void)numFields;
    (void)filter; (void)callback; (void)param;
}

void peerStopListingGames(PEER peer) {
    (void)peer;
}

void peerMessageRoomA(PEER peer, RoomType roomType,
                      const gsi_char* message, MessageType messageType) {
    (void)peer; (void)roomType; (void)message; (void)messageType;
}

void peerMessagePlayerA(PEER peer, const gsi_char* nick,
                        const gsi_char* message, MessageType messageType) {
    (void)peer; (void)nick; (void)message; (void)messageType;
}

void peerUTMRoomA(PEER peer, RoomType roomType,
                  const gsi_char* command, const gsi_char* parameters,
                  PEERBool authenticate) {
    (void)peer; (void)roomType; (void)command;
    (void)parameters; (void)authenticate;
}

void peerUTMPlayerA(PEER peer, const gsi_char* nick,
                    const gsi_char* command, const gsi_char* parameters,
                    PEERBool authenticate) {
    (void)peer; (void)nick; (void)command;
    (void)parameters; (void)authenticate;
}

void peerEnumPlayers(PEER peer, RoomType roomType,
                     peerEnumPlayersCallback callback, void* param) {
    (void)peer; (void)roomType; (void)callback; (void)param;
}

PEERBool peerGetPlayerFlagsA(PEER peer, const gsi_char* nick,
                             RoomType roomType, int* flags) {
    (void)peer; (void)nick; (void)roomType;
    if (flags) *flags = 0;
    return PEERFalse;
}

PEERBool peerGetPlayerInfoNoWaitA(PEER peer, const gsi_char* nick,
                                  unsigned int* IP, int* profileID) {
    (void)peer; (void)nick;
    if (IP) *IP = 0;
    if (profileID) *profileID = 0;
    return PEERFalse;
}

void peerGetPlayerProfileIDA(PEER peer, const gsi_char* nick,
                             peerGetPlayerProfileIDCallback callback,
                             void* param, PEERBool blocking) {
    (void)peer; (void)nick; (void)callback; (void)param; (void)blocking;
}

unsigned int peerGetPublicIP(PEER peer) {
    (void)peer;
    return 0;
}

void peerGetRoomKeysA(PEER peer, RoomType roomType,
                      const gsi_char* nick, int num,
                      const gsi_char** keys,
                      peerGetRoomKeysCallback callback,
                      void* param, PEERBool blocking) {
    (void)peer; (void)roomType; (void)nick; (void)num;
    (void)keys; (void)callback; (void)param; (void)blocking;
}

void peerSetRoomKeysA(PEER peer, RoomType roomType,
                      const gsi_char* nick, int num,
                      const gsi_char** keys, const gsi_char** values) {
    (void)peer; (void)roomType; (void)nick;
    (void)num; (void)keys; (void)values;
}

void peerSetRoomWatchKeysA(PEER peer, RoomType roomType,
                           int num, const gsi_char** keys,
                           PEERBool addKeys) {
    (void)peer; (void)roomType; (void)num; (void)keys; (void)addKeys;
}

void peerStartGameA(PEER peer, const gsi_char* message, int reportingOptions) {
    (void)peer; (void)message; (void)reportingOptions;
}

void peerStopGame(PEER peer) {
    (void)peer;
}

void peerUpdateGame(PEER peer, SBServer server, PEERBool fullUpdate) {
    (void)peer; (void)server; (void)fullUpdate;
}

void peerStateChanged(PEER peer) {
    (void)peer;
}

void peerParseQueryA(PEER peer, char* query, int len, struct sockaddr* sender) {
    (void)peer; (void)query; (void)len; (void)sender;
}

void peerAuthenticateCDKeyA(PEER peer, const gsi_char* cdkey,
                            peerAuthenticateCDKeyCallback callback,
                            void* param, PEERBool blocking) {
    (void)peer; (void)cdkey; (void)callback; (void)param; (void)blocking;
}

// ══════════════════════════════════════════════════════
//  SERVER BROWSING
// ══════════════════════════════════════════════════════

int SBServerGetIntValueA(SBServer server, const gsi_char* key, int idefault) {
    (void)server; (void)key;
    return idefault;
}

const gsi_char* SBServerGetStringValueA(SBServer server, const gsi_char* keyname, const gsi_char* def) {
    (void)server; (void)keyname;
    return def;
}

int SBServerGetPlayerIntValueA(SBServer server, int playernum, const gsi_char* key, int idefault) {
    (void)server; (void)playernum; (void)key;
    return idefault;
}

const gsi_char* SBServerGetPlayerStringValueA(SBServer server, int playernum,
                                              const gsi_char* key, const gsi_char* sdefault) {
    (void)server; (void)playernum; (void)key;
    return sdefault;
}

unsigned int SBServerGetPublicInetAddress(SBServer server) {
    (void)server;
    return 0;
}

unsigned int SBServerGetPrivateInetAddress(SBServer server) {
    (void)server;
    return 0;
}

unsigned short SBServerGetPrivateQueryPort(SBServer server) {
    (void)server;
    return 0;
}

SBBool SBServerHasBasicKeys(SBServer server) {
    (void)server;
    return SBFalse;
}

// ══════════════════════════════════════════════════════
//  GSTATS / PERSISTENT STORAGE
// ══════════════════════════════════════════════════════

int InitStatsConnection(int gameport) {
    (void)gameport;
    return GE_NOSOCKET;
}

int IsStatsConnected(void) {
    return 0;
}

void CloseStatsConnection(void) {
}

statsgame_t NewGame(int usebuckets) {
    (void)usebuckets;
    return NULL;
}

void FreeGame(statsgame_t game) {
    (void)game;
}

char* GetChallenge(statsgame_t game) {
    (void)game;
    return "NULLGAME";
}

char* GenerateAuthA(const char* challenge, const gsi_char* password, char response[33]) {
    (void)challenge; (void)password;
    if (response) memset(response, 0, 33);
    return response;
}

int SendGameSnapShotA(statsgame_t game, const gsi_char* snapshot, int isFinal) {
    (void)game; (void)snapshot; (void)isFinal;
    return GE_NOCONNECT;
}

void PreAuthenticatePlayerPM(int localid, int profileid,
                             const char* challengeresponse,
                             PersAuthCallbackFn callback, void* instance) {
    (void)localid; (void)profileid; (void)challengeresponse;
    (void)callback; (void)instance;
}

void PreAuthenticatePlayerCDA(int localid, const gsi_char* nick,
                              const char* keyhash,
                              const char* challengeresponse,
                              PersAuthCallbackFn callback, void* instance) {
    (void)localid; (void)nick; (void)keyhash;
    (void)challengeresponse; (void)callback; (void)instance;
}

void GetPersistDataValuesA(int localid, int profileid,
                           persisttype_t type, int index,
                           gsi_char* keys,
                           PersDataCallbackFn callback, void* instance) {
    (void)localid; (void)profileid; (void)type;
    (void)index; (void)keys; (void)callback; (void)instance;
}

void SetPersistDataValuesA(int localid, int profileid,
                           persisttype_t type, int index,
                           const gsi_char* keyvalues,
                           PersDataSaveCallbackFn callback, void* instance) {
    (void)localid; (void)profileid; (void)type;
    (void)index; (void)keyvalues; (void)callback; (void)instance;
}

int PersistThink(void) {
    return 1;
}

// ══════════════════════════════════════════════════════
//  CHAT
// ══════════════════════════════════════════════════════

void chatSetLocalIP(unsigned long preferredIP) {
    (void)preferredIP;
}

// ══════════════════════════════════════════════════════
//  QR2 (Query & Reporting)
// ══════════════════════════════════════════════════════

gsi_bool qr2_buffer_addA(qr2_buffer_t outbuf, const char* value) {
    (void)outbuf; (void)value;
    return gsi_false;
}

gsi_bool qr2_buffer_add_int(qr2_buffer_t outbuf, int value) {
    (void)outbuf; (void)value;
    return gsi_false;
}

gsi_bool qr2_keybuffer_add(qr2_keybuffer_t keybuffer, int keyid) {
    (void)keybuffer; (void)keyid;
    return gsi_false;
}

void qr2_register_keyA(int keyid, const char* key) {
    (void)keyid; (void)key;
}

// ══════════════════════════════════════════════════════
//  GHTTP (GameSpy HTTP SDK)
// ══════════════════════════════════════════════════════

#include "ghttp/ghttp.h"

void ghttpStartup(void) {
}

void ghttpCleanup(void) {
}

GHTTPRequest ghttpGetA(const char* URL, GHTTPBool blocking,
                       ghttpCompletedCallback completedCallback, void* param) {
    (void)URL; (void)blocking; (void)completedCallback; (void)param;
    if (completedCallback) {
        completedCallback(-1, GHTTPHostLookupFailed, "", 0, param);
    }
    return (GHTTPRequest)-1;
}

GHTTPRequest ghttpHeadA(const char* URL, GHTTPBool blocking,
                        ghttpCompletedCallback completedCallback, void* param) {
    (void)URL; (void)blocking; (void)completedCallback; (void)param;
    if (completedCallback) {
        completedCallback(-1, GHTTPHostLookupFailed, "", 0, param);
    }
    return (GHTTPRequest)-1;
}

GHTTPRequest ghttpHeadExA(const char* URL, const char* headers,
                          GHTTPBool throttle, GHTTPBool blocking,
                          ghttpProgressCallback progressCallback,
                          ghttpCompletedCallback completedCallback, void* param) {
    (void)URL; (void)headers; (void)throttle; (void)blocking;
    (void)progressCallback; (void)completedCallback; (void)param;
    return (GHTTPRequest)-1;
}

void ghttpThink(void) {
}

const char* ghttpGetHeaders(GHTTPRequest request) {
    (void)request;
    return "";
}

GHTTPBool ghttpSetProxy(const char* server) {
    (void)server;
    return GHTTPFalse;
}

void ghttpCancelRequest(GHTTPRequest request) {
    (void)request;
}

// ══════════════════════════════════════════════════════
//  MISC
// ══════════════════════════════════════════════════════

int getQR2HostingStatus(void) {
    return 0;
}

