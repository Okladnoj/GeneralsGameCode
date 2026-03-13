#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>
#include "MacOSOnlineLogin.h"
#include "MacOSDebugLog.h"

#ifdef __APPLE__

static const char* kGenOnlineApiURL = "https://api.playgenerals.online";
static const char* kGenOnlineBrowserURL = "https://www.playgenerals.online";
static const char* kGenOnlineClientID = "gen_online_60hz";
static const char* kGenOnlineEnv = "live";
static const char* kGenOnlineContract = "1";

static GenOnlineLoginState s_loginState = GenOnlineLoginState::Idle;
static GenOnlineSession s_session = {};
static char s_gameCode[64] = {};
static NSURLSession* s_urlSession = nil;
static bool s_pollInFlight = false;
static CFAbsoluteTime s_lastPollTime = 0;
static const CFTimeInterval kPollInterval = 2.0;

static void generateGameCode() {
  static const char charset[] =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  for (int i = 0; i < 30; i++) {
    s_gameCode[i] = charset[arc4random_uniform(sizeof(charset) - 1)];
  }
  s_gameCode[30] = '\0';
}

static void openLoginInBrowser() {
  NSString* urlStr = [NSString
      stringWithFormat:@"%s/login?gamecode=%s", kGenOnlineBrowserURL, s_gameCode];
  NSURL* url = [NSURL URLWithString:urlStr];
  if (url) {
    [[NSWorkspace sharedWorkspace] openURL:url];
    DLOG_NETWORK("GenOnline: opened browser: %s", [urlStr UTF8String]);
  }
}

static void pollCheckLogin() {
  if (s_pollInFlight) return;
  if (s_loginState != GenOnlineLoginState::WaitingForBrowser) return;

  CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
  if (now - s_lastPollTime < kPollInterval) return;
  s_lastPollTime = now;

  s_pollInFlight = true;

  NSString* urlStr = [NSString
      stringWithFormat:@"%s/env/%s/contract/%s/CheckLogin",
                       kGenOnlineApiURL, kGenOnlineEnv, kGenOnlineContract];
  NSURL* url = [NSURL URLWithString:urlStr];
  NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:url];
  [request setHTTPMethod:@"POST"];
  [request setValue:@"application/json" forHTTPHeaderField:@"Content-Type"];

  NSDictionary* body =
      @{@"code" : @(s_gameCode), @"client_id" : @(kGenOnlineClientID)};
  NSError* jsonError = nil;
  NSData* jsonData = [NSJSONSerialization dataWithJSONObject:body
                                                    options:0
                                                      error:&jsonError];
  if (jsonError) {
    DLOG_NETWORK("GenOnline: JSON encode error: %s",
                 [[jsonError localizedDescription] UTF8String]);
    s_pollInFlight = false;
    return;
  }
  [request setHTTPBody:jsonData];

  DLOG_NETWORK("GenOnline: polling CheckLogin...");

  NSURLSessionDataTask* task = [s_urlSession
      dataTaskWithRequest:request
        completionHandler:^(NSData* data, NSURLResponse* response,
                            NSError* error) {
          s_pollInFlight = false;

          if (error) {
            DLOG_NETWORK("GenOnline: poll error: %s",
                         [[error localizedDescription] UTF8String]);
            return;
          }

          NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
          DLOG_NETWORK("GenOnline: HTTP status=%ld", (long)httpResp.statusCode);

          if (httpResp.statusCode == 423) {
            DLOG_NETWORK("GenOnline: user is banned");
            s_loginState = GenOnlineLoginState::LoginFailed;
            return;
          }

          if (!data) {
            DLOG_NETWORK("GenOnline: no response data");
            return;
          }

          NSString* rawResponse =
              [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
          DLOG_NETWORK("GenOnline: response body: %s",
                       rawResponse ? [rawResponse UTF8String] : "(nil)");

          NSError* parseError = nil;
          NSDictionary* json =
              [NSJSONSerialization JSONObjectWithData:data
                                             options:0
                                               error:&parseError];
          if (parseError || !json) {
            DLOG_NETWORK("GenOnline: JSON parse error");
            return;
          }

          NSNumber* resultNum = json[@"result"];
          if (!resultNum) {
            DLOG_NETWORK("GenOnline: no 'result' field in response");
            return;
          }

          int result = [resultNum intValue];
          NSNumber* userId = json[@"user_id"];
          long long uid = userId ? [userId longLongValue] : -1;
          DLOG_NETWORK("GenOnline: poll result=%d user_id=%lld", result, uid);

          if (result == 0) {
            return;
          }

          if (result == 1 || result == 2) {
            NSString* displayName = json[@"display_name"];
            NSString* sessionToken = json[@"session_token"];
            NSString* refreshToken = json[@"refresh_token"];
            NSString* wsUri = json[@"ws_uri"];

            if (displayName && ![displayName isKindOfClass:[NSNull class]])
              strlcpy(s_session.displayName, [displayName UTF8String],
                      sizeof(s_session.displayName));
            if (sessionToken && ![sessionToken isKindOfClass:[NSNull class]])
              strlcpy(s_session.sessionToken, [sessionToken UTF8String],
                      sizeof(s_session.sessionToken));
            if (refreshToken && ![refreshToken isKindOfClass:[NSNull class]])
              strlcpy(s_session.refreshToken, [refreshToken UTF8String],
                      sizeof(s_session.refreshToken));
            if (wsUri && ![wsUri isKindOfClass:[NSNull class]])
              strlcpy(s_session.wsUri, [wsUri UTF8String],
                      sizeof(s_session.wsUri));
            s_session.userId = uid;

            DLOG_NETWORK(
                "GenOnline: LOGIN SUCCESS! user=%s id=%lld",
                s_session.displayName, s_session.userId);

            s_loginState = GenOnlineLoginState::LoginSuccess;
            return;
          }

          if (result == -1 || result == 0) {
            return;
          }

          if (result == 3) {
            DLOG_NETWORK("GenOnline: login explicitly failed (result=3)");
            s_loginState = GenOnlineLoginState::LoginFailed;
            return;
          }
        }];
  [task resume];
}

void GenOnline_StartLogin() {
  memset(&s_session, 0, sizeof(s_session));
  s_pollInFlight = false;
  s_lastPollTime = 0;

  generateGameCode();
  DLOG_NETWORK("GenOnline: starting login with gamecode=%s", s_gameCode);

  if (!s_urlSession) {
    NSURLSessionConfiguration* config =
        [NSURLSessionConfiguration ephemeralSessionConfiguration];
    config.timeoutIntervalForRequest = 10.0;
    s_urlSession = [NSURLSession sessionWithConfiguration:config];
  }

  openLoginInBrowser();
  s_loginState = GenOnlineLoginState::WaitingForBrowser;
}

void GenOnline_CancelLogin() {
  DLOG_NETWORK("GenOnline: login cancelled/reset");
  s_loginState = GenOnlineLoginState::Idle;
  s_pollInFlight = false;
}

void GenOnline_Update() {
  if (s_loginState == GenOnlineLoginState::WaitingForBrowser) {
    pollCheckLogin();
  }
}

GenOnlineLoginState GenOnline_GetState() { return s_loginState; }

const GenOnlineSession* GenOnline_GetSession() { return &s_session; }

static NSString* const kSavedRefreshToken = @"GenOnline_RefreshToken";
static NSString* const kSavedDisplayName = @"GenOnline_DisplayName";
static NSString* const kSavedUserId = @"GenOnline_UserId";

bool GenOnline_HasSavedSession() {
  NSString* token =
      [[NSUserDefaults standardUserDefaults] stringForKey:kSavedRefreshToken];
  return token != nil && [token length] > 0;
}

void GenOnline_SaveSession() {
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  [defaults setObject:[NSString stringWithUTF8String:s_session.refreshToken]
               forKey:kSavedRefreshToken];
  [defaults setObject:[NSString stringWithUTF8String:s_session.displayName]
               forKey:kSavedDisplayName];
  [defaults setInteger:(NSInteger)s_session.userId forKey:kSavedUserId];
  [defaults synchronize];
  DLOG_NETWORK("GenOnline: session saved for user=%s", s_session.displayName);
}

void GenOnline_ClearSavedSession() {
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  [defaults removeObjectForKey:kSavedRefreshToken];
  [defaults removeObjectForKey:kSavedDisplayName];
  [defaults removeObjectForKey:kSavedUserId];
  [defaults synchronize];
  DLOG_NETWORK("GenOnline: saved session cleared");
}

void GenOnline_TryAutoLogin() {
  NSUserDefaults* defaults = [NSUserDefaults standardUserDefaults];
  NSString* savedToken = [defaults stringForKey:kSavedRefreshToken];
  if (!savedToken || [savedToken length] == 0) {
    DLOG_NETWORK("GenOnline: no saved session, need browser login");
    s_loginState = GenOnlineLoginState::LoginFailed;
    return;
  }

  DLOG_NETWORK("GenOnline: attempting auto-login with saved token");
  s_loginState = GenOnlineLoginState::AutoLoggingIn;
  memset(&s_session, 0, sizeof(s_session));

  if (!s_urlSession) {
    NSURLSessionConfiguration* config =
        [NSURLSessionConfiguration ephemeralSessionConfiguration];
    config.timeoutIntervalForRequest = 10.0;
    s_urlSession = [NSURLSession sessionWithConfiguration:config];
  }

  NSString* urlStr = [NSString
      stringWithFormat:@"%s/env/%s/contract/%s/LoginWithToken",
                       kGenOnlineApiURL, kGenOnlineEnv, kGenOnlineContract];
  NSURL* url = [NSURL URLWithString:urlStr];
  NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:url];
  [request setHTTPMethod:@"POST"];
  [request setValue:@"application/json"
      forHTTPHeaderField:@"Content-Type"];
  NSString* bearer =
      [NSString stringWithFormat:@"Bearer %@", savedToken];
  [request setValue:bearer forHTTPHeaderField:@"Authorization"];
  NSString* bodyJson =
      [NSString stringWithFormat:@"{\"client_id\":\"%s\"}", kGenOnlineClientID];
  [request setHTTPBody:[bodyJson dataUsingEncoding:NSUTF8StringEncoding]];

  NSURLSessionDataTask* task = [s_urlSession
      dataTaskWithRequest:request
        completionHandler:^(NSData* data, NSURLResponse* response,
                            NSError* error) {
          if (error || !data) {
            DLOG_NETWORK("GenOnline: auto-login network error");
            GenOnline_ClearSavedSession();
            s_loginState = GenOnlineLoginState::LoginFailed;
            return;
          }

          NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
          long status = [httpResp statusCode];
          DLOG_NETWORK("GenOnline: auto-login HTTP status=%ld", status);

          if (status != 200) {
            DLOG_NETWORK("GenOnline: auto-login token expired/invalid");
            GenOnline_ClearSavedSession();
            s_loginState = GenOnlineLoginState::LoginFailed;
            return;
          }

          NSError* parseError = nil;
          NSDictionary* json =
              [NSJSONSerialization JSONObjectWithData:data
                                             options:0
                                               error:&parseError];
          if (parseError || !json) {
            DLOG_NETWORK("GenOnline: auto-login JSON parse error");
            GenOnline_ClearSavedSession();
            s_loginState = GenOnlineLoginState::LoginFailed;
            return;
          }

          NSString* rawResp =
              [[NSString alloc] initWithData:data
                                    encoding:NSUTF8StringEncoding];
          DLOG_NETWORK("GenOnline: auto-login response: %s",
                       [rawResp UTF8String]);

          NSString* sessionToken = json[@"session_token"];
          NSString* refreshToken = json[@"refresh_token"];
          NSString* displayName = json[@"display_name"];
          NSString* wsUri = json[@"ws_uri"];
          NSNumber* userId = json[@"user_id"];

          if (sessionToken && ![sessionToken isKindOfClass:[NSNull class]])
            strlcpy(s_session.sessionToken, [sessionToken UTF8String],
                    sizeof(s_session.sessionToken));
          if (refreshToken && ![refreshToken isKindOfClass:[NSNull class]])
            strlcpy(s_session.refreshToken, [refreshToken UTF8String],
                    sizeof(s_session.refreshToken));
          if (displayName && ![displayName isKindOfClass:[NSNull class]])
            strlcpy(s_session.displayName, [displayName UTF8String],
                    sizeof(s_session.displayName));
          if (wsUri && ![wsUri isKindOfClass:[NSNull class]])
            strlcpy(s_session.wsUri, [wsUri UTF8String],
                    sizeof(s_session.wsUri));
          if (userId)
            s_session.userId = [userId longLongValue];

          DLOG_NETWORK("GenOnline: AUTO-LOGIN SUCCESS! user=%s id=%lld",
                       s_session.displayName, s_session.userId);

          GenOnline_SaveSession();
          s_loginState = GenOnlineLoginState::LoginSuccess;
        }];
  [task resume];
}

#endif // __APPLE__
