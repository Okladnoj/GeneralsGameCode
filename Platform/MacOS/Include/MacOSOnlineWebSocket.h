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

#endif // __APPLE__
