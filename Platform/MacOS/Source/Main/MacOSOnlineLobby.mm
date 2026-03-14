#import <Foundation/Foundation.h>
#include "MacOSOnlineLobby.h"
#include "MacOSOnlineLogin.h"
#include "MacOSOnlineWSBridge.h"
#include "MacOSOnlineP2P.h"
#include "MacOSDebugLog.h"

#ifdef __APPLE__

static const char* kLobbyApiURL = "https://api.playgenerals.online";
static const char* kLobbyEnv = "live";
static const char* kLobbyContract = "1";

static NSURLSession* s_lobbySession = nil;
static bool s_requestInFlight = false;
static int s_lastResult = 0;
static int s_createdLobbyId = 0;

static NSURLSession* getLobbySession() {
  if (!s_lobbySession) {
    NSURLSessionConfiguration* config =
        [NSURLSessionConfiguration ephemeralSessionConfiguration];
    config.timeoutIntervalForRequest = 10.0;
    s_lobbySession = [NSURLSession sessionWithConfiguration:config];
  }
  return s_lobbySession;
}

static NSMutableURLRequest* createAuthorizedRequest(NSString* urlStr,
                                                     NSString* method) {
  NSURL* url = [NSURL URLWithString:urlStr];
  NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:url];
  [request setHTTPMethod:method];
  [request setValue:@"application/json" forHTTPHeaderField:@"Content-Type"];

  const GenOnlineSession* session = GenOnline_GetSession();
  if (session && strlen(session->sessionToken) > 0) {
    NSString* bearer = [NSString
        stringWithFormat:@"Bearer %s", session->sessionToken];
    [request setValue:bearer forHTTPHeaderField:@"Authorization"];
  }

  return request;
}

void GenOnlineLobby_Create(const char* gameName, const char* mapName,
                            const char* mapPath, int mapOfficial,
                            int maxPlayers, const char* password,
                            int allowObservers, int useStats,
                            unsigned int startingCash,
                            unsigned short preferredPort,
                            unsigned short maxCamHeight,
                            unsigned int exeCRC, unsigned int iniCRC) {
  if (s_requestInFlight) {
    DLOG_NETWORK("GenOnlineLobby: create already in flight, ignoring");
    return;
  }

  s_requestInFlight = true;
  s_lastResult = 0;
  s_createdLobbyId = 0;

  NSString* urlStr = [NSString
      stringWithFormat:@"%s/env/%s/contract/%s/Lobbies",
                       kLobbyApiURL, kLobbyEnv, kLobbyContract];

  NSMutableURLRequest* request = createAuthorizedRequest(urlStr, @"PUT");

  bool hasPassword = (password && strlen(password) > 0);

  NSMutableDictionary* body = [NSMutableDictionary dictionary];
  body[@"name"] = [NSString stringWithUTF8String:gameName];
  body[@"map_name"] = [NSString stringWithUTF8String:(mapName ? mapName : "")];
  body[@"map_path"] = [NSString stringWithUTF8String:(mapPath ? mapPath : "")];
  body[@"map_official"] = @(mapOfficial ? true : false);
  body[@"max_players"] = @(maxPlayers > 0 ? maxPlayers : 8);
  body[@"preferred_port"] = @(preferredPort);
  body[@"vanilla_teams"] = @(false);
  body[@"track_stats"] = @(useStats ? true : false);
  body[@"starting_cash"] = @(startingCash);
  body[@"passworded"] = @(hasPassword);
  body[@"password"] = hasPassword
      ? [NSString stringWithUTF8String:password] : @"";
  body[@"allow_observers"] = @(allowObservers ? true : false);
  body[@"max_cam_height"] = @(maxCamHeight > 0 ? maxCamHeight : 310);
  body[@"exe_crc"] = @(exeCRC);
  body[@"ini_crc"] = @(iniCRC);

  NSError* jsonError = nil;
  NSData* jsonData = [NSJSONSerialization dataWithJSONObject:body
                                                    options:0
                                                      error:&jsonError];
  if (jsonError) {
    DLOG_NETWORK("GenOnlineLobby: JSON encode error");
    s_requestInFlight = false;
    s_lastResult = -1;
    return;
  }
  [request setHTTPBody:jsonData];

  DLOG_NETWORK("GenOnlineLobby: creating lobby '%s'", gameName);

  NSURLSessionDataTask* task = [getLobbySession()
      dataTaskWithRequest:request
        completionHandler:^(NSData* data, NSURLResponse* response,
                            NSError* error) {
          s_requestInFlight = false;

          if (error || !data) {
            DLOG_NETWORK("GenOnlineLobby: create network error: %s",
                         error ? [[error localizedDescription] UTF8String]
                               : "no data");
            s_lastResult = -1;
            return;
          }

          NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
          long status = [httpResp statusCode];
          DLOG_NETWORK("GenOnlineLobby: create HTTP status=%ld", status);

          if (status < 200 || status >= 300) {
            NSString* bodyStr =
                [[NSString alloc] initWithData:data
                                      encoding:NSUTF8StringEncoding];
            DLOG_NETWORK("GenOnlineLobby: create FAILED: %s",
                         bodyStr ? [bodyStr UTF8String] : "(nil)");
            s_lastResult = -1;
            return;
          }

          NSError* parseError = nil;
          NSDictionary* json =
              [NSJSONSerialization JSONObjectWithData:data
                                             options:0
                                               error:&parseError];
          if (parseError || !json) {
            DLOG_NETWORK("GenOnlineLobby: create JSON parse error");
            s_lastResult = -1;
            return;
          }

          NSNumber* lobbyId = json[@"lobby_id"];
          if (lobbyId) {
            s_createdLobbyId = [lobbyId intValue];
          }

          NSString* turnUser = json[@"turn_username"];
          NSString* turnToken = json[@"turn_token"];
          if (turnUser && turnToken) {
            GenOnlineP2P_SetTURNCredentials("",
                [turnUser UTF8String], [turnToken UTF8String]);
          }

          DLOG_NETWORK("GenOnlineLobby: lobby created! id=%d",
                       s_createdLobbyId);
          s_lastResult = 1;
        }];
  [task resume];
}

void GenOnlineLobby_Join(int lobbyId, const char* password) {
  if (s_requestInFlight) {
    DLOG_NETWORK("GenOnlineLobby: join already in flight, ignoring");
    return;
  }

  s_requestInFlight = true;
  s_lastResult = 0;

  NSString* urlStr = [NSString
      stringWithFormat:@"%s/env/%s/contract/%s/Lobbies/%d/join",
                       kLobbyApiURL, kLobbyEnv, kLobbyContract, lobbyId];

  NSMutableURLRequest* request = createAuthorizedRequest(urlStr, @"POST");

  NSMutableDictionary* body = [NSMutableDictionary dictionary];
  if (password && strlen(password) > 0) {
    body[@"password"] = [NSString stringWithUTF8String:password];
  }

  NSError* jsonError = nil;
  NSData* jsonData = [NSJSONSerialization dataWithJSONObject:body
                                                    options:0
                                                      error:&jsonError];
  if (jsonError) {
    s_requestInFlight = false;
    s_lastResult = -1;
    return;
  }
  [request setHTTPBody:jsonData];

  DLOG_NETWORK("GenOnlineLobby: joining lobby %d", lobbyId);

  NSURLSessionDataTask* task = [getLobbySession()
      dataTaskWithRequest:request
        completionHandler:^(NSData* data, NSURLResponse* response,
                            NSError* error) {
          s_requestInFlight = false;

          if (error || !data) {
            DLOG_NETWORK("GenOnlineLobby: join network error");
            s_lastResult = -1;
            return;
          }

          NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
          long status = [httpResp statusCode];
          DLOG_NETWORK("GenOnlineLobby: join HTTP status=%ld", status);

          if (status < 200 || status >= 300) {
            NSString* bodyStr =
                [[NSString alloc] initWithData:data
                                      encoding:NSUTF8StringEncoding];
            DLOG_NETWORK("GenOnlineLobby: join FAILED: %s",
                         bodyStr ? [bodyStr UTF8String] : "(nil)");
            s_lastResult = -1;
            return;
          }

          DLOG_NETWORK("GenOnlineLobby: joined lobby %d!", lobbyId);
          s_createdLobbyId = lobbyId;

          NSError* parseError = nil;
          NSDictionary* json =
              [NSJSONSerialization JSONObjectWithData:data
                                             options:0
                                               error:&parseError];
          if (!parseError && json) {
            NSString* turnUser = json[@"turn_username"];
            NSString* turnToken = json[@"turn_token"];
            if (turnUser && turnToken) {
              GenOnlineP2P_SetTURNCredentials("",
                  [turnUser UTF8String], [turnToken UTF8String]);
            }
          }

          s_lastResult = 1;
        }];
  [task resume];
}

void GenOnlineLobby_Leave(void) {
  DLOG_NETWORK("GenOnlineLobby: leave (no-op for now, server handles via WS disconnect)");
}

int GenOnlineLobby_IsRequestInFlight(void) { return s_requestInFlight ? 1 : 0; }

int GenOnlineLobby_GetLastResult(void) { return s_lastResult; }

int GenOnlineLobby_GetCreatedLobbyId(void) { return s_createdLobbyId; }

void GenOnlineLobby_ResetResult(void) {
  s_lastResult = 0;
  s_createdLobbyId = 0;
}

void GenOnline_FetchMOTD(void) {
  NSString* urlStr = [NSString
      stringWithFormat:@"%s/env/%s/contract/%s/MOTD",
                       kLobbyApiURL, kLobbyEnv, kLobbyContract];

  NSMutableURLRequest* request = createAuthorizedRequest(urlStr, @"GET");

  DLOG_NETWORK("GenOnline: fetching MOTD from %s", [urlStr UTF8String]);

  NSURLSessionDataTask* task = [getLobbySession()
      dataTaskWithRequest:request
        completionHandler:^(NSData* data, NSURLResponse* response,
                            NSError* error) {
          if (error || !data) {
            DLOG_NETWORK("GenOnline: MOTD fetch error: %s",
                         error ? [[error localizedDescription] UTF8String]
                               : "no data");
            return;
          }

          NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
          long status = [httpResp statusCode];
          if (status < 200 || status >= 300) {
            DLOG_NETWORK("GenOnline: MOTD HTTP %ld", status);
            return;
          }

          NSString* motdText =
              [[NSString alloc] initWithData:data
                                    encoding:NSUTF8StringEncoding];
          if (!motdText || [motdText length] == 0) {
            DLOG_NETWORK("GenOnline: MOTD empty");
            return;
          }

          DLOG_NETWORK("GenOnline: MOTD received (%lu chars)",
                       (unsigned long)[motdText length]);

          GenOnlineWS_InjectChatMessage("MOTD", [motdText UTF8String], 0);
        }];
  [task resume];
}

void GenOnlineLobby_FetchDetails(int lobbyId) {
  if (lobbyId <= 0) {
    return;
  }

  NSString* urlStr = [NSString
      stringWithFormat:@"%s/env/%s/contract/%s/Lobby/%d",
                       kLobbyApiURL, kLobbyEnv, kLobbyContract, lobbyId];

  NSMutableURLRequest* request = createAuthorizedRequest(urlStr, @"GET");

  DLOG_NETWORK("GenOnlineLobby: fetching details for lobby %d", lobbyId);

  NSURLSessionDataTask* task = [getLobbySession()
      dataTaskWithRequest:request
        completionHandler:^(NSData* data, NSURLResponse* response,
                            NSError* error) {
          if (error || !data) {
            DLOG_NETWORK("GenOnlineLobby: fetch details error");
            return;
          }

          NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
          long status = [httpResp statusCode];
          if (status < 200 || status >= 300) {
            DLOG_NETWORK("GenOnlineLobby: fetch details HTTP %ld", status);
            return;
          }

          NSError* parseError = nil;
          NSDictionary* json =
              [NSJSONSerialization JSONObjectWithData:data
                                             options:0
                                               error:&parseError];
          if (parseError || !json) {
            DLOG_NETWORK("GenOnlineLobby: fetch details parse error");
            return;
          }

          NSDictionary* lobby = json[@"lobby"];
          if (!lobby) {
            DLOG_NETWORK("GenOnlineLobby: fetch details no 'lobby' key");
            return;
          }

          NSString* name = lobby[@"Name"];
          NSArray* members = lobby[@"Members"];
          int playerCount = 0;
          int maxSlots = 8;

          if (members && [members isKindOfClass:[NSArray class]]) {
            for (NSDictionary* member in members) {
              NSNumber* slotState = member[@"SlotState"];
              if (slotState && [slotState intValue] == 1) {
                ++playerCount;
              }
            }
            maxSlots = (int)[members count];
          }

          DLOG_NETWORK("GenOnlineLobby: details id=%d name='%s' players=%d/%d",
                       lobbyId,
                       name ? [name UTF8String] : "(nil)",
                       playerCount, maxSlots);

          GenOnlineWS_InjectLobbyUpdate(
              lobbyId,
              name ? [name UTF8String] : "Unknown",
              playerCount,
              maxSlots);
        }];
  [task resume];
}

void GenOnlineLobby_FetchRooms(void) {
  NSString* urlStr = [NSString
      stringWithFormat:@"%s/env/%s/contract/%s/Rooms",
                       kLobbyApiURL, kLobbyEnv, kLobbyContract];

  NSMutableURLRequest* request = createAuthorizedRequest(urlStr, @"GET");

  DLOG_NETWORK("GenOnlineLobby: fetching rooms from %s", [urlStr UTF8String]);

  dispatch_semaphore_t sem = dispatch_semaphore_create(0);

  NSURLSessionDataTask* task = [getLobbySession()
      dataTaskWithRequest:request
        completionHandler:^(NSData* data, NSURLResponse* response,
                            NSError* error) {
          if (error || !data) {
            DLOG_NETWORK("GenOnlineLobby: fetch rooms error: %s",
                         error ? [[error localizedDescription] UTF8String]
                               : "no data");
            dispatch_semaphore_signal(sem);
            return;
          }

          NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
          long status = [httpResp statusCode];
          if (status < 200 || status >= 300) {
            DLOG_NETWORK("GenOnlineLobby: fetch rooms HTTP %ld", status);
            dispatch_semaphore_signal(sem);
            return;
          }

          NSError* parseError = nil;
          NSDictionary* json =
              [NSJSONSerialization JSONObjectWithData:data
                                             options:0
                                               error:&parseError];
          if (parseError || !json) {
            DLOG_NETWORK("GenOnlineLobby: fetch rooms parse error");
            dispatch_semaphore_signal(sem);
            return;
          }

          NSArray* rooms = json[@"rooms"];
          if (!rooms || ![rooms isKindOfClass:[NSArray class]]) {
            DLOG_NETWORK("GenOnlineLobby: fetch rooms no 'rooms' key");
            dispatch_semaphore_signal(sem);
            return;
          }

          DLOG_NETWORK("GenOnlineLobby: received %lu rooms",
                       (unsigned long)[rooms count]);

          NSDictionary* sentinelRoom = nil;
          for (NSDictionary* room in rooms) {
            NSNumber* roomId = room[@"id"];
            NSString* roomName = room[@"name"];

            if (!roomId || !roomName) continue;

            if ([roomId intValue] == 0) {
              sentinelRoom = room;
              continue;
            }

            GenOnlineWS_InjectGroupRoom(
                [roomId intValue],
                [roomName UTF8String],
                0, 100, 0, 0);
          }

          if (sentinelRoom) {
            GenOnlineWS_InjectGroupRoom(
                [sentinelRoom[@"id"] intValue],
                [sentinelRoom[@"name"] UTF8String],
                0, 100, 0, 0);
          }

          DLOG_NETWORK("GenOnlineLobby: injected %lu group rooms",
                       (unsigned long)[rooms count]);
          dispatch_semaphore_signal(sem);
        }];
  [task resume];

  dispatch_semaphore_wait(sem, dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC));
}

void GenOnlineLobby_FetchList(void) {
  NSString* urlStr = [NSString
      stringWithFormat:@"%s/env/%s/contract/%s/Lobbies",
                       kLobbyApiURL, kLobbyEnv, kLobbyContract];

  NSMutableURLRequest* request = createAuthorizedRequest(urlStr, @"GET");

  DLOG_NETWORK("GenOnlineLobby: fetching lobby list from %s",
               [urlStr UTF8String]);

  NSURLSessionDataTask* task = [getLobbySession()
      dataTaskWithRequest:request
        completionHandler:^(NSData* data, NSURLResponse* response,
                            NSError* error) {
          if (error || !data) {
            DLOG_NETWORK("GenOnlineLobby: fetch list error: %s",
                         error ? [[error localizedDescription] UTF8String]
                               : "no data");
            return;
          }

          NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
          long status = [httpResp statusCode];
          if (status < 200 || status >= 300) {
            DLOG_NETWORK("GenOnlineLobby: fetch list HTTP %ld", status);
            return;
          }

          NSString* rawBody = [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
          DLOG_NETWORK("GenOnlineLobby: HTTP %ld, body length=%lu, first 300: %.300s",
                       status, (unsigned long)[data length],
                       rawBody ? [rawBody UTF8String] : "(nil)");

          NSError* parseError = nil;
          NSDictionary* json =
              [NSJSONSerialization JSONObjectWithData:data
                                             options:0
                                               error:&parseError];
          if (parseError || !json) {
            DLOG_NETWORK("GenOnlineLobby: fetch list parse error");
            return;
          }

          NSArray* lobbies = json[@"lobbies"];
          if (!lobbies || ![lobbies isKindOfClass:[NSArray class]]) {
            DLOG_NETWORK("GenOnlineLobby: fetch list no 'lobbies' key");
            return;
          }

          DLOG_NETWORK("GenOnlineLobby: received %lu lobbies",
                       (unsigned long)[lobbies count]);

          GenOnlineWS_InjectLobbyListBegin();

          for (NSDictionary* lobby in lobbies) {
            NSNumber* lobbyId = lobby[@"LobbyID"];
            NSString* name = lobby[@"Name"];
            NSNumber* numPlayers = lobby[@"NumCurrentPlayers"];
            NSNumber* maxPlayers = lobby[@"MaxPlayers"];
            NSNumber* hasPassword = lobby[@"IsPassworded"];
            NSString* mapPath = lobby[@"MapPath"];
            NSNumber* exeCRC = lobby[@"ExeCRC"];
            NSNumber* iniCRC = lobby[@"IniCRC"];
            NSNumber* allowObs = lobby[@"AllowObservers"];
            NSNumber* useStats = lobby[@"IsTrackingStats"];

            if (!name) continue;

            NSString* hostName = @"";
            NSArray* members = lobby[@"Members"];
            LobbyMemberInfo memberInfos[MAX_LOBBY_MEMBERS] = {};
            int memberCount = 0;

            if (members && [members isKindOfClass:[NSArray class]]) {
              for (NSDictionary* member in members) {
                if (memberCount >= MAX_LOBBY_MEMBERS) break;

                NSString* dn = member[@"DisplayName"];
                NSNumber* uid = member[@"UserID"];
                NSNumber* slotTeam = member[@"Team"];
                NSNumber* slotColor = member[@"Color"];
                NSNumber* slotFaction = member[@"Faction"];

                if (!dn) continue;

                memberInfos[memberCount].displayName = [dn UTF8String];
                memberInfos[memberCount].userId = uid ? [uid intValue] : 0;
                memberInfos[memberCount].slotState = 1;
                memberInfos[memberCount].team = slotTeam ? [slotTeam intValue] : -1;
                memberInfos[memberCount].color = slotColor ? [slotColor intValue] : -1;
                memberInfos[memberCount].faction = slotFaction ? [slotFaction intValue] : -1;

                if (memberCount == 0 && dn) hostName = dn;
                ++memberCount;
              }
            }

            GenOnlineWS_InjectLobbyListEntry(
                lobbyId ? [lobbyId intValue] : 0,
                [name UTF8String],
                [hostName UTF8String],
                numPlayers ? [numPlayers intValue] : 0,
                maxPlayers ? [maxPlayers intValue] : 8,
                hasPassword ? [hasPassword boolValue] : false,
                mapPath ? [mapPath UTF8String] : "",
                exeCRC ? [exeCRC unsignedIntValue] : 1,
                iniCRC ? [iniCRC unsignedIntValue] : 1,
                allowObs ? [allowObs boolValue] : false,
                useStats ? [useStats boolValue] : false,
                memberInfos, memberCount);
          }

          DLOG_NETWORK("GenOnlineLobby: injected %lu lobby entries",
                       (unsigned long)[lobbies count]);
        }];
  [task resume];
}

#endif // __APPLE__
