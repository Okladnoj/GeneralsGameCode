#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void GenWebRTC_SetTurnCredentials(const char* url, const char* username, const char* credential);
void GenWebRTC_SetLocalUserId(long long userId);
void GenWebRTC_OnStartSignalling(long long lobbyId, long long targetUserId, int preferredPort);
void GenWebRTC_OnSignalPayload(long long targetUserId, const unsigned char* payload, int payloadLen);
void GenWebRTC_RemovePeer(long long targetUserId);
void GenWebRTC_Poll(void);

#ifdef __cplusplus
}
#endif
