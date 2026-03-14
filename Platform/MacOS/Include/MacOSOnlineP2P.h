#pragma once

#ifdef __APPLE__

#include <cstdint>

static const int kMaxP2PPeers = 8;
static const int kP2PSignalPayloadSize = 6;
static const int kMaxTURNUrlLen = 256;
static const int kMaxTURNCredentialLen = 128;

enum class GenOnlineP2PState : uint8_t {
  Idle,
  WaitingForSignaling,
  Signaling,
  Connected,
  Failed
};

struct GenOnlineP2PPeer {
  int64_t userId;
  uint32_t ip;
  uint16_t port;
  GenOnlineP2PState state;
};

void GenOnlineP2P_Reset();
int GenOnlineP2P_GetPeerCount();
GenOnlineP2PPeer* GenOnlineP2P_GetPeer(int index);
GenOnlineP2PPeer* GenOnlineP2P_FindPeer(int64_t userId);

void GenOnlineP2P_OnStartSignalling(int64_t lobbyId, int64_t userId, int preferredPort);
void GenOnlineP2P_OnNetworkSignal(int64_t fromUserId, const unsigned char* payload, int payloadLen);
void GenOnlineP2P_OnDisconnectPlayer(int64_t lobbyId, int64_t userId);
void GenOnlineP2P_OnFullMeshCheckResponse();

void GenOnlineP2P_RemovePeer(int64_t userId);
bool GenOnlineP2P_IsAllPeersConnected();

void GenOnlineP2P_SetLocalIP(uint32_t ip);
uint32_t GenOnlineP2P_GetLocalIP();

void GenOnlineP2P_SetTURNCredentials(const char* url, const char* username, const char* credential);
bool GenOnlineP2P_HasTURNCredentials();
const char* GenOnlineP2P_GetTURNUrl();
const char* GenOnlineP2P_GetTURNUsername();
const char* GenOnlineP2P_GetTURNCredential();

void GenOnlineP2P_BuildSignalPayload(unsigned char* outPayload, uint32_t ip, uint16_t port);
void GenOnlineP2P_ParseSignalPayload(const unsigned char* payload, uint32_t* outIp, uint16_t* outPort);

#endif // __APPLE__
