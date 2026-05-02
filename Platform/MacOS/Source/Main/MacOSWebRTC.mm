#include "MacOSWebRTC.h"
#include "MacOSDebugLog.h"

extern "C" {

void GenWebRTC_SetTurnCredentials(const char* url, const char* username, const char* credential) {
    DLOG_NETWORK("GenWebRTC: SetTurnCredentials url=%s user=%s", url ? url : "", username ? username : "");
    // TODO: Implement TURN credentials setup
}

void GenWebRTC_SetLocalUserId(long long userId) {
    DLOG_NETWORK("GenWebRTC: SetLocalUserId %lld", userId);
    // TODO: Implement local user ID logic
}

void GenWebRTC_OnStartSignalling(long long lobbyId, long long targetUserId, int preferredPort) {
    DLOG_NETWORK("GenWebRTC: OnStartSignalling lobby=%lld target=%lld port=%d", lobbyId, targetUserId, preferredPort);
    // TODO: Implement WebRTC offer/answer signalling initiation
}

void GenWebRTC_OnSignalPayload(long long targetUserId, const unsigned char* payload, int payloadLen) {
    DLOG_NETWORK("GenWebRTC: OnSignalPayload target=%lld len=%d", targetUserId, payloadLen);
    // TODO: Implement payload parsing and WebRTC signalling
}

void GenWebRTC_RemovePeer(long long targetUserId) {
    DLOG_NETWORK("GenWebRTC: RemovePeer target=%lld", targetUserId);
    // TODO: Implement peer connection cleanup
}

void GenWebRTC_Poll(void) {
    // TODO: Implement WebRTC update/poll loop
}

} // extern "C"
