#import <Foundation/Foundation.h>
#include "MacOSOnlineLobby.h"
#include "MacOSOnlineLogin.h"
#include "MacOSOnlineWSBridge.h"
#include "MacOSOnlineWebSocket.h"
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
  body[@"preferred_port"] = @(0); // WebRTC ICE requires vport 0
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
      stringWithFormat:@"%s/env/%s/contract/%s/Lobby/%d",
                       kLobbyApiURL, kLobbyEnv, kLobbyContract, lobbyId];

  NSMutableURLRequest* request = createAuthorizedRequest(urlStr, @"PUT");

  NSMutableDictionary* body = [NSMutableDictionary dictionary];
  body[@"preferred_port"] = @(0); // WebRTC ICE needs 0
  body[@"has_map"] = @(YES);
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

  s_createdLobbyId = lobbyId;
  DLOG_NETWORK("GenOnlineLobby: joining lobby %d", lobbyId);

  NSURLSessionDataTask* task = [getLobbySession()
      dataTaskWithRequest:request
        completionHandler:^(NSData* data, NSURLResponse* response,
                            NSError* error) {
          s_requestInFlight = false;

          if (error || !data) {
            DLOG_NETWORK("GenOnlineLobby: join network error");
            s_createdLobbyId = 0;
            s_lastResult = -1;
            GenOnlineWS_InjectJoinStagingRoomFailure(lobbyId, 4);
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
            s_createdLobbyId = 0;
            s_lastResult = -1;
            int reason = (status == 406) ? 3 : 4;
            GenOnlineWS_InjectJoinStagingRoomFailure(lobbyId, reason);
            return;
          }

          DLOG_NETWORK("GenOnlineLobby: joined lobby %d!", lobbyId);

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

          NSString* detailsUrl = [NSString
              stringWithFormat:@"%s/env/%s/contract/%s/Lobby/%d",
                               kLobbyApiURL, kLobbyEnv, kLobbyContract, lobbyId];
          NSMutableURLRequest* detailsReq = createAuthorizedRequest(detailsUrl, @"GET");

          dispatch_semaphore_t detailsSem = dispatch_semaphore_create(0);
          static const char* s_fetchedNames[8] = {};
          static char s_nameBufs[8][64] = {};
          static int s_fetchedCount = 0;
          for (int i = 0; i < 8; ++i) { s_fetchedNames[i] = nullptr; s_nameBufs[i][0] = '\0'; }
          s_fetchedCount = 0;
          NSURLSessionConfiguration* detCfg =
              [NSURLSessionConfiguration ephemeralSessionConfiguration];
          detCfg.timeoutIntervalForRequest = 5.0;
          NSURLSession* detSession = [NSURLSession sessionWithConfiguration:detCfg];

          NSURLSessionDataTask* detailsTask = [detSession
              dataTaskWithRequest:detailsReq
                completionHandler:^(NSData* detData, NSURLResponse* detResp,
                                    NSError* detErr) {
                  if (detErr || !detData) {
                    dispatch_semaphore_signal(detailsSem);
                    return;
                  }
                  NSHTTPURLResponse* detHttp = (NSHTTPURLResponse*)detResp;
                  if ([detHttp statusCode] < 200 || [detHttp statusCode] >= 300) {
                    dispatch_semaphore_signal(detailsSem);
                    return;
                  }
                  NSError* detParseErr = nil;
                  NSDictionary* detJson =
                      [NSJSONSerialization JSONObjectWithData:detData
                                                      options:0
                                                        error:&detParseErr];
                  if (detParseErr || !detJson || !detJson[@"lobby"]) {
                    dispatch_semaphore_signal(detailsSem);
                    return;
                  }
                  NSDictionary* lobby = detJson[@"lobby"];
                  NSArray* members = lobby[@"Members"];
                  NSString* mapPath = lobby[@"MapPath"];
                  NSNumber* rngSeed = lobby[@"RNGSeed"];
                  NSNumber* startCash = lobby[@"StartingCash"];
                  NSNumber* limSW = lobby[@"IsLimitSuperweapons"];
                  NSNumber* trackStats = lobby[@"TrackStats"];
                  NSString* hostName = @"";

                  if (members && [members isKindOfClass:[NSArray class]]) {
                    int cnt = (int)[members count];
                    if (cnt > 8) cnt = 8;
                    s_fetchedCount = cnt;

                    LobbySlotInfo slots[8] = {};
                    static char slotNameBufs[8][64];

                    for (int i = 0; i < cnt; ++i) {
                      NSDictionary* m = members[i];
                      NSString* dn = m[@"DisplayName"];
                      NSNumber* ss = m[@"SlotState"];
                      NSNumber* side = m[@"Side"];
                      NSNumber* col = m[@"Color"];
                      NSNumber* team = m[@"Team"];
                      NSNumber* sp = m[@"StartingPosition"];
                      NSNumber* hm = m[@"HasMap"];
                      NSNumber* rdy = m[@"IsReady"];

                      if (dn) {
                        strncpy(slotNameBufs[i], [dn UTF8String], 63);
                        slotNameBufs[i][63] = '\0';
                        strncpy(s_nameBufs[i], [dn UTF8String], 63);
                        s_nameBufs[i][63] = '\0';
                      }
                      s_fetchedNames[i] = s_nameBufs[i];

                      if (i == 0 && dn) hostName = dn;

                      slots[i].displayName = slotNameBufs[i];
                      slots[i].slotState = ss ? [ss intValue] : 0;
                      slots[i].side = side ? [side intValue] : 0;
                      slots[i].color = col ? [col intValue] : -1;
                      slots[i].team = team ? [team intValue] : -1;
                      slots[i].startPos = sp ? [sp intValue] : -1;
                      slots[i].hasMap = hm ? [hm boolValue] : 0;
                      slots[i].isAccepted = rdy ? [rdy boolValue] : 0;
                    }

                    const char* mapPathStr = mapPath ? [mapPath UTF8String] : "";
                    int seedVal = rngSeed ? [rngSeed intValue] : 0;
                    unsigned int cashVal = startCash ? [startCash unsignedIntValue] : 10000;
                    int useStatsVal = trackStats ? [trackStats boolValue] : 1;
                    unsigned short swRestriction = limSW ? ([limSW boolValue] ? 1 : 0) : 0;

                    GenOnlineWS_InjectSlotListUTM(
                        [hostName UTF8String], mapPathStr,
                        0x3F, 0, 0,
                        seedVal, 100, useStatsVal, cashVal,
                        swRestriction, 0,
                        slots, cnt);
                    DLOG_NETWORK("GenOnlineLobby: join FetchDetails injected SL with %d slots", cnt);
                  }
                  dispatch_semaphore_signal(detailsSem);
                }];
          [detailsTask resume];
          dispatch_semaphore_wait(detailsSem,
              dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC));

          GenOnlineWS_InjectJoinStagingRoomSuccessWithPlayers(
              lobbyId, s_fetchedNames, s_fetchedCount);
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

          NSString* mapPath = lobby[@"MapPath"];
          NSNumber* rngSeed = lobby[@"RNGSeed"];
          NSNumber* startingCash = lobby[@"StartingCash"];
          NSNumber* limitSuperweapons = lobby[@"IsLimitSuperweapons"];
          NSNumber* useStats = lobby[@"TrackStats"];
          NSArray* members = lobby[@"Members"];

          NSString* hostName = @"";
          if (members && [members isKindOfClass:[NSArray class]] &&
              [members count] > 0) {
            NSDictionary* firstMember = members[0];
            NSString* name = firstMember[@"DisplayName"];
            if (name) hostName = name;
          }

          int slotCount = 0;
          LobbySlotInfo slots[8] = {};
          static char slotNames[8][64];

          if (members && [members isKindOfClass:[NSArray class]]) {
            slotCount = (int)[members count];
            if (slotCount > 8) slotCount = 8;

            for (int i = 0; i < slotCount; ++i) {
              NSDictionary* member = members[i];

              NSString* name = member[@"DisplayName"];
              NSNumber* memberId = member[@"UserID"];
              NSNumber* slotState = member[@"SlotState"];
              NSNumber* side = member[@"Side"];
              NSNumber* color = member[@"Color"];
              NSNumber* team = member[@"Team"];
              NSNumber* startPos = member[@"StartingPosition"];
              NSNumber* hasMap = member[@"HasMap"];
              NSNumber* isReady = member[@"IsReady"];

              if (name) {
                strncpy(slotNames[i], [name UTF8String], 63);
                slotNames[i][63] = '\0';
              } else {
                slotNames[i][0] = '\0';
              }

              slots[i].displayName = slotNames[i];
              slots[i].userId = memberId ? [memberId longLongValue] : -1;
              slots[i].slotState = slotState ? [slotState intValue] : 0;
              slots[i].side = side ? [side intValue] : 0;
              slots[i].color = color ? [color intValue] : -1;
              slots[i].team = team ? [team intValue] : -1;
              slots[i].startPos = startPos ? [startPos intValue] : -1;
              slots[i].hasMap = hasMap ? [hasMap boolValue] : 0;
              slots[i].isAccepted = isReady ? [isReady boolValue] : 0;

              GenOnlineP2P_SetSlotUserId(i, slots[i].userId);
            }
          }

          const char* mapPathStr = mapPath ? [mapPath UTF8String] : "";
          int seedVal = rngSeed ? [rngSeed intValue] : 0;
          unsigned int cashVal = startingCash ? [startingCash unsignedIntValue] : 10000;
          int useStatsVal = useStats ? [useStats boolValue] : 1;
          unsigned short swRestriction = limitSuperweapons ? ([limitSuperweapons boolValue] ? 1 : 0) : 0;

          NSNumber* ownerUserId = lobby[@"Owner"];
          long long ownerId = ownerUserId ? [ownerUserId longLongValue] : -1;
          const GenOnlineSession* session = GenOnline_GetSession();
          bool isHost = (session && session->userId == ownerId);

          DLOG_NETWORK("GenOnlineLobby: details id=%d map='%s' host='%s' seed=%d cash=%u slots=%d isHost=%d",
                       lobbyId, mapPathStr,
                       [hostName UTF8String], seedVal, cashVal, slotCount, isHost);

          if (isHost) {
            for (int i = 1; i < slotCount; ++i) {
              if (slots[i].slotState != 5 || slots[i].displayName[0] == '\0') {
                continue;
              }

              char optBuf[64];

              snprintf(optBuf, sizeof(optBuf), "PlayerTemplate=%d", slots[i].side);
              GenOnlineWS_InjectPlayerUTM(slots[i].displayName, "REQ", optBuf);

              snprintf(optBuf, sizeof(optBuf), "Color=%d", slots[i].color);
              GenOnlineWS_InjectPlayerUTM(slots[i].displayName, "REQ", optBuf);

              snprintf(optBuf, sizeof(optBuf), "Team=%d", slots[i].team);
              GenOnlineWS_InjectPlayerUTM(slots[i].displayName, "REQ", optBuf);

              snprintf(optBuf, sizeof(optBuf), "StartPos=%d", slots[i].startPos);
              GenOnlineWS_InjectPlayerUTM(slots[i].displayName, "REQ", optBuf);

              snprintf(optBuf, sizeof(optBuf), "%d", slots[i].hasMap);
              GenOnlineWS_InjectPlayerUTM(slots[i].displayName, "MAP", optBuf);

              if (slots[i].isAccepted) {
                GenOnlineWS_InjectPlayerUTM(slots[i].displayName, "accept", "");
              }

              DLOG_NETWORK("GenOnlineLobby: host-merge slot %d '%s' side=%d color=%d team=%d",
                           i, slots[i].displayName, slots[i].side, slots[i].color, slots[i].team);
            }
          } else {
            GenOnlineWS_InjectSlotListUTM(
                [hostName UTF8String], mapPathStr,
                0x3F, 0, 0,
                seedVal, 100, useStatsVal, cashVal,
                swRestriction, 0,
                slots, slotCount);
          }

          int64_t humanMemberIds[8];
          int humanCount = 0;
          for (int i = 0; i < slotCount; ++i) {
            if (slots[i].slotState == 5 && slots[i].userId > 0) {
              humanMemberIds[humanCount++] = slots[i].userId;
            }
          }
          GenOnlineP2P_SetLobbyMembers(humanMemberIds, humanCount);

          DLOG_NETWORK("GenOnlineLobby: FetchDetails complete. isHost=%d cached %d human members.",
                       isHost, humanCount);
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
                memberInfos[memberCount].slotState = 5;
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

static void postLobbyUpdate(NSDictionary* body) {
  int lobbyId = GenOnlineLobby_GetCreatedLobbyId();
  if (lobbyId <= 0) {
    DLOG_NETWORK("GenOnlineLobby: postLobbyUpdate skipped, no lobby id");
    return;
  }

  NSString* urlStr = [NSString
      stringWithFormat:@"%s/env/%s/contract/%s/Lobby/%d",
                       kLobbyApiURL, kLobbyEnv, kLobbyContract, lobbyId];

  NSMutableURLRequest* request = createAuthorizedRequest(urlStr, @"POST");

  NSError* jsonError = nil;
  NSData* jsonData = [NSJSONSerialization dataWithJSONObject:body
                                                    options:0
                                                      error:&jsonError];
  if (jsonError || !jsonData) {
    DLOG_NETWORK("GenOnlineLobby: postLobbyUpdate JSON serialize error");
    return;
  }

  [request setHTTPBody:jsonData];

  NSURLSessionDataTask* task = [getLobbySession()
      dataTaskWithRequest:request
        completionHandler:^(NSData* data, NSURLResponse* response,
                            NSError* error) {
          NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
          long status = httpResp ? [httpResp statusCode] : 0;
          if (error || status < 200 || status >= 300) {
            DLOG_NETWORK("GenOnlineLobby: postLobbyUpdate failed HTTP %ld err=%s",
                         status,
                         error ? [[error localizedDescription] UTF8String] : "none");
            return;
          }
          DLOG_NETWORK("GenOnlineLobby: postLobbyUpdate OK (HTTP %ld)", status);
        }];
  [task resume];
}

void GenOnlineLobby_UpdateMap(const char* map, const char* mapPath,
    int mapOfficial, int maxPlayers) {
  DLOG_NETWORK("GenOnlineLobby: UpdateMap map='%s' path='%s' official=%d maxPlayers=%d",
               map, mapPath, mapOfficial, maxPlayers);
  postLobbyUpdate(@{
    @"field" : @(0),
    @"map" : [NSString stringWithUTF8String:map],
    @"map_path" : [NSString stringWithUTF8String:mapPath],
    @"map_official" : @(mapOfficial ? YES : NO),
    @"max_players" : @(maxPlayers)
  });
}

void GenOnlineLobby_UpdateSide(int side, int startPos) {
  DLOG_NETWORK("GenOnlineLobby: UpdateSide side=%d startPos=%d", side, startPos);
  postLobbyUpdate(@{
    @"field" : @(1),
    @"side" : @(side),
    @"start_pos" : @(startPos)
  });
}

void GenOnlineLobby_UpdateColor(int color) {
  DLOG_NETWORK("GenOnlineLobby: UpdateColor color=%d", color);
  postLobbyUpdate(@{
    @"field" : @(2),
    @"color" : @(color)
  });
}

void GenOnlineLobby_UpdateStartPos(int startPos) {
  DLOG_NETWORK("GenOnlineLobby: UpdateStartPos startPos=%d", startPos);
  postLobbyUpdate(@{
    @"field" : @(3),
    @"startpos" : @(startPos)
  });
}

void GenOnlineLobby_UpdateTeam(int team) {
  DLOG_NETWORK("GenOnlineLobby: UpdateTeam team=%d", team);
  postLobbyUpdate(@{
    @"field" : @(4),
    @"team" : @(team)
  });
}

void GenOnlineLobby_UpdateStartingCash(unsigned int cash) {
  DLOG_NETWORK("GenOnlineLobby: UpdateStartingCash cash=%u", cash);
  postLobbyUpdate(@{
    @"field" : @(5),
    @"startingcash" : @(cash)
  });
}

void GenOnlineLobby_UpdateLimitSuperweapons(int limit) {
  DLOG_NETWORK("GenOnlineLobby: UpdateLimitSuperweapons limit=%d", limit);
  postLobbyUpdate(@{
    @"field" : @(6),
    @"limit_superweapons" : @(limit ? YES : NO)
  });
}

void GenOnlineLobby_ForceStart(void) {
  DLOG_NETWORK("GenOnlineLobby: ForceStart");
  postLobbyUpdate(@{
    @"field" : @(7)
  });
}

void GenOnlineLobby_UpdateHasMap(int hasMap) {
  DLOG_NETWORK("GenOnlineLobby: UpdateHasMap hasMap=%d", hasMap);
  postLobbyUpdate(@{
    @"field" : @(8),
    @"has_map" : @(hasMap ? YES : NO)
  });
}

void GenOnlineLobby_KickUser(long long userId) {
  DLOG_NETWORK("GenOnlineLobby: KickUser userId=%lld", userId);
  postLobbyUpdate(@{
    @"field" : @(11),
    @"userid" : @(userId)
  });
}

void GenOnlineLobby_SetSlotState(int slotIndex, int slotState) {
  DLOG_NETWORK("GenOnlineLobby: SetSlotState slot=%d state=%d", slotIndex, slotState);
  postLobbyUpdate(@{
    @"field" : @(12),
    @"slot_index" : @(slotIndex),
    @"slot_state" : @(slotState)
  });
}

void GenOnlineLobby_UpdateAISide(int slot, int side, int startPos) {
  DLOG_NETWORK("GenOnlineLobby: UpdateAISide slot=%d side=%d startPos=%d", slot, side, startPos);
  postLobbyUpdate(@{
    @"field" : @(13),
    @"slot" : @(slot),
    @"side" : @(side),
    @"start_pos" : @(startPos)
  });
}

void GenOnlineLobby_UpdateAIColor(int slot, int color) {
  DLOG_NETWORK("GenOnlineLobby: UpdateAIColor slot=%d color=%d", slot, color);
  postLobbyUpdate(@{
    @"field" : @(14),
    @"slot" : @(slot),
    @"color" : @(color)
  });
}

void GenOnlineLobby_UpdateAITeam(int slot, int team) {
  DLOG_NETWORK("GenOnlineLobby: UpdateAITeam slot=%d team=%d", slot, team);
  postLobbyUpdate(@{
    @"field" : @(15),
    @"slot" : @(slot),
    @"team" : @(team)
  });
}

void GenOnlineLobby_UpdateAIStartPos(int slot, int startPos) {
  DLOG_NETWORK("GenOnlineLobby: UpdateAIStartPos slot=%d startPos=%d", slot, startPos);
  postLobbyUpdate(@{
    @"field" : @(16),
    @"slot" : @(slot),
    @"start_pos" : @(startPos)
  });
}

void GenOnlineLobby_UpdateMaxCameraHeight(unsigned short maxCamHeight) {
  DLOG_NETWORK("GenOnlineLobby: UpdateMaxCameraHeight height=%u", maxCamHeight);
  postLobbyUpdate(@{
    @"field" : @(17),
    @"max_camera_height" : @(maxCamHeight)
  });
}

void GenOnlineLobby_UpdateJoinability(int joinability) {
  DLOG_NETWORK("GenOnlineLobby: UpdateJoinability joinability=%d", joinability);
  postLobbyUpdate(@{
    @"field" : @(18),
    @"joinability" : @(joinability)
  });
}

void GenOnlineLobby_MarkReady(int ready) {
  DLOG_NETWORK("GenOnlineLobby: MarkReady ready=%d", ready);
  GenOnlineWS_SendReady(ready);
}

#endif // __APPLE__
