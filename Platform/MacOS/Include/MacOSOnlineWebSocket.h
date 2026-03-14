#pragma once

#ifdef __APPLE__

enum class GenOnlineWSState {
  Disconnected,
  Connecting,
  Connected,
  Error
};

void GenOnlineWS_Connect(const char* wsUri, const char* sessionToken);
void GenOnlineWS_Disconnect();
void GenOnlineWS_Update();
GenOnlineWSState GenOnlineWS_GetState();

void GenOnlineWS_SendPing();
void GenOnlineWS_SendChangeRoom(int roomId);
void GenOnlineWS_SendChat(const char* message);
void GenOnlineWS_SendLobbyChat(const char* message);
void GenOnlineWS_SendReady(int ready);
void GenOnlineWS_SendStartGame();
void GenOnlineWS_SendStartCountdown();
void GenOnlineWS_SendRemovePassword();
void GenOnlineWS_SendChangePassword(const char* password);

void GenOnlineWS_SendFullMeshCheckBegin();
void GenOnlineWS_SendFullMeshCheckResponse(const long long* userIds, int count);
void GenOnlineWS_SendRequestSignalling(long long targetUserId);
void GenOnlineWS_SendSignal(long long targetUserId, const unsigned char* payload, int payloadLen);

#endif // __APPLE__
