#import <Foundation/Foundation.h>
#include "MacOSOnlineP2P.h"
#include "MacOSOnlineWebSocket.h"
#include "MacOSDebugLog.h"

#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <cstring>

#ifdef __APPLE__

static GenOnlineP2PPeer s_peers[kMaxP2PPeers];
static int s_peerCount = 0;
static uint32_t s_localIP = 0;

static char s_turnUrl[kMaxTURNUrlLen] = {};
static char s_turnUsername[kMaxTURNCredentialLen] = {};
static char s_turnCredential[kMaxTURNCredentialLen] = {};
static bool s_hasTurnCredentials = false;

void GenOnlineP2P_Reset() {
  s_peerCount = 0;
  memset(s_peers, 0, sizeof(s_peers));
  s_turnUrl[0] = '\0';
  s_turnUsername[0] = '\0';
  s_turnCredential[0] = '\0';
  s_hasTurnCredentials = false;
  DLOG_NETWORK("GenOnlineP2P: reset");
}

int GenOnlineP2P_GetPeerCount() {
  return s_peerCount;
}

GenOnlineP2PPeer* GenOnlineP2P_GetPeer(int index) {
  if (index < 0 || index >= s_peerCount) {
    return nullptr;
  }
  return &s_peers[index];
}

GenOnlineP2PPeer* GenOnlineP2P_FindPeer(int64_t userId) {
  for (int i = 0; i < s_peerCount; ++i) {
    if (s_peers[i].userId == userId) {
      return &s_peers[i];
    }
  }
  return nullptr;
}

static GenOnlineP2PPeer* addPeer(int64_t userId) {
  GenOnlineP2PPeer* existing = GenOnlineP2P_FindPeer(userId);
  if (existing) {
    return existing;
  }

  if (s_peerCount >= kMaxP2PPeers) {
    DLOG_NETWORK("GenOnlineP2P: max peers reached, cannot add user=%lld", userId);
    return nullptr;
  }

  GenOnlineP2PPeer* peer = &s_peers[s_peerCount++];
  peer->userId = userId;
  peer->ip = 0;
  peer->port = 0;
  peer->state = GenOnlineP2PState::WaitingForSignaling;
  return peer;
}

void GenOnlineP2P_OnStartSignalling(int64_t lobbyId, int64_t userId, int preferredPort) {
  GenOnlineP2PPeer* peer = addPeer(userId);
  if (!peer) {
    return;
  }

  peer->port = static_cast<uint16_t>(preferredPort);
  peer->state = GenOnlineP2PState::Signaling;

  DLOG_NETWORK("GenOnlineP2P: start signalling with user=%lld port=%d (peer #%d)",
               userId, preferredPort, s_peerCount);

  if (s_localIP != 0) {
    unsigned char payload[kP2PSignalPayloadSize];
    GenOnlineP2P_BuildSignalPayload(payload, s_localIP, 8088);
    GenOnlineWS_SendSignal(userId, payload, kP2PSignalPayloadSize);
    DLOG_NETWORK("GenOnlineP2P: sent our address to user=%lld", userId);
  }
}

void GenOnlineP2P_OnNetworkSignal(int64_t fromUserId, const unsigned char* payload, int payloadLen) {
  if (payloadLen < kP2PSignalPayloadSize) {
    DLOG_NETWORK("GenOnlineP2P: signal payload too short (%d bytes) from user=%lld",
                 payloadLen, fromUserId);
    return;
  }

  uint32_t peerIp = 0;
  uint16_t peerPort = 0;
  GenOnlineP2P_ParseSignalPayload(payload, &peerIp, &peerPort);

  GenOnlineP2PPeer* peer = addPeer(fromUserId);
  if (!peer) {
    return;
  }

  peer->ip = peerIp;
  peer->port = peerPort;
  peer->state = GenOnlineP2PState::Connected;

  struct in_addr addr;
  addr.s_addr = htonl(peerIp);
  DLOG_NETWORK("GenOnlineP2P: resolved user=%lld → %s:%d",
               fromUserId, inet_ntoa(addr), peerPort);
}

void GenOnlineP2P_OnDisconnectPlayer(int64_t lobbyId, int64_t userId) {
  DLOG_NETWORK("GenOnlineP2P: disconnect user=%lld (lobby=%lld)", userId, lobbyId);
  GenOnlineP2P_RemovePeer(userId);
}

void GenOnlineP2P_RemovePeer(int64_t userId) {
  for (int i = 0; i < s_peerCount; ++i) {
    if (s_peers[i].userId != userId) {
      continue;
    }

    DLOG_NETWORK("GenOnlineP2P: removing peer user=%lld (index=%d, count=%d→%d)",
                 userId, i, s_peerCount, s_peerCount - 1);

    for (int j = i; j < s_peerCount - 1; ++j) {
      s_peers[j] = s_peers[j + 1];
    }
    --s_peerCount;
    memset(&s_peers[s_peerCount], 0, sizeof(GenOnlineP2PPeer));
    return;
  }

  DLOG_NETWORK("GenOnlineP2P: remove peer user=%lld — not found", userId);
}

bool GenOnlineP2P_IsAllPeersConnected() {
  if (s_peerCount == 0) {
    return false;
  }

  for (int i = 0; i < s_peerCount; ++i) {
    if (s_peers[i].state != GenOnlineP2PState::Connected) {
      return false;
    }
  }
  return true;
}

void GenOnlineP2P_OnFullMeshCheckResponse() {
  long long connectedPeers[kMaxP2PPeers];
  int connectedCount = 0;

  for (int i = 0; i < s_peerCount; ++i) {
    if (s_peers[i].state == GenOnlineP2PState::Connected) {
      connectedPeers[connectedCount++] = s_peers[i].userId;
    }
  }

  GenOnlineWS_SendFullMeshCheckResponse(connectedPeers, connectedCount);
  DLOG_NETWORK("GenOnlineP2P: full mesh check response sent (%d/%d connected)",
               connectedCount, s_peerCount);
}

void GenOnlineP2P_SetLocalIP(uint32_t ip) {
  s_localIP = ip;
  struct in_addr addr;
  addr.s_addr = htonl(ip);
  DLOG_NETWORK("GenOnlineP2P: local IP set to %s", inet_ntoa(addr));
}

uint32_t GenOnlineP2P_GetLocalIP() {
  return s_localIP;
}

void GenOnlineP2P_BuildSignalPayload(unsigned char* outPayload, uint32_t ip, uint16_t port) {
  uint32_t netIp = htonl(ip);
  uint16_t netPort = htons(port);
  memcpy(outPayload, &netIp, 4);
  memcpy(outPayload + 4, &netPort, 2);
}

void GenOnlineP2P_ParseSignalPayload(const unsigned char* payload, uint32_t* outIp, uint16_t* outPort) {
  uint32_t netIp = 0;
  uint16_t netPort = 0;
  memcpy(&netIp, payload, 4);
  memcpy(&netPort, payload + 4, 2);
  *outIp = ntohl(netIp);
  *outPort = ntohs(netPort);
}

extern "C" unsigned int GenOnlineP2P_ResolveLocalIP(void) {
  if (s_localIP != 0) {
    return s_localIP;
  }

  struct ifaddrs* ifAddrList = nullptr;
  if (getifaddrs(&ifAddrList) != 0) {
    DLOG_NETWORK("GenOnlineP2P: getifaddrs failed");
    return 0;
  }

  uint32_t foundIP = 0;
  for (struct ifaddrs* ifa = ifAddrList; ifa != nullptr; ifa = ifa->ifa_next) {
    if (!ifa->ifa_addr) {
      continue;
    }
    if (ifa->ifa_addr->sa_family != AF_INET) {
      continue;
    }
    if (ifa->ifa_flags & IFF_LOOPBACK) {
      continue;
    }
    if (!(ifa->ifa_flags & IFF_UP)) {
      continue;
    }

    struct sockaddr_in* sa = (struct sockaddr_in*)ifa->ifa_addr;
    foundIP = ntohl(sa->sin_addr.s_addr);
    DLOG_NETWORK("GenOnlineP2P: found interface %s → %s",
                 ifa->ifa_name, inet_ntoa(sa->sin_addr));
    break;
  }

  freeifaddrs(ifAddrList);

  if (foundIP != 0) {
    GenOnlineP2P_SetLocalIP(foundIP);
  }

  return foundIP;
}

extern "C" void GenOnlineP2P_ApplyPeerAddresses(void* slotArray, int slotCount) {
  DLOG_NETWORK("GenOnlineP2P: applying %d peer addresses to %d slots",
               s_peerCount, slotCount);

  for (int i = 0; i < s_peerCount; ++i) {
    GenOnlineP2PPeer* peer = &s_peers[i];
    if (peer->state != GenOnlineP2PState::Connected) {
      DLOG_NETWORK("GenOnlineP2P: skipping peer user=%lld (state=%d)",
                   peer->userId, (int)peer->state);
      continue;
    }

    struct in_addr addr;
    addr.s_addr = htonl(peer->ip);
    DLOG_NETWORK("GenOnlineP2P: peer user=%lld → %s:%d (ready for slot matching)",
                 peer->userId, inet_ntoa(addr), peer->port);
  }
}

void GenOnlineP2P_SetTURNCredentials(const char* url, const char* username, const char* credential) {
  if (url) {
    strncpy(s_turnUrl, url, kMaxTURNUrlLen - 1);
    s_turnUrl[kMaxTURNUrlLen - 1] = '\0';
  }
  if (username) {
    strncpy(s_turnUsername, username, kMaxTURNCredentialLen - 1);
    s_turnUsername[kMaxTURNCredentialLen - 1] = '\0';
  }
  if (credential) {
    strncpy(s_turnCredential, credential, kMaxTURNCredentialLen - 1);
    s_turnCredential[kMaxTURNCredentialLen - 1] = '\0';
  }
  s_hasTurnCredentials = (username && username[0] != '\0');
  DLOG_NETWORK("GenOnlineP2P: TURN credentials set (user=%s, hasCreds=%d)",
               s_turnUsername, s_hasTurnCredentials ? 1 : 0);
}

bool GenOnlineP2P_HasTURNCredentials() {
  return s_hasTurnCredentials;
}

const char* GenOnlineP2P_GetTURNUrl() {
  return s_turnUrl;
}

const char* GenOnlineP2P_GetTURNUsername() {
  return s_turnUsername;
}

const char* GenOnlineP2P_GetTURNCredential() {
  return s_turnCredential;
}

#endif // __APPLE__
