#pragma once

#ifdef __APPLE__

enum class GenOnlineLoginState {
  Idle,
  WaitingForBrowser,
  AutoLoggingIn,
  LoginSuccess,
  LoginFailed,
  Cancelled
};

struct GenOnlineSession {
  char sessionToken[2048];
  char refreshToken[2048];
  char displayName[128];
  char wsUri[512];
  long long userId;
};

#ifdef __cplusplus
extern "C" {
#endif

void GenOnline_StartLogin(void);

void GenOnline_CancelLogin(void);

void GenOnline_Update(void);

GenOnlineLoginState GenOnline_GetState(void);

const GenOnlineSession* GenOnline_GetSession(void);

bool GenOnline_HasSavedSession(void);

void GenOnline_TryAutoLogin(void);

void GenOnline_SaveSession(void);

void GenOnline_ClearSavedSession(void);

#ifdef __cplusplus
}
#endif

#endif // __APPLE__
