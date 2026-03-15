#include <string>
#import <Foundation/Foundation.h>
#include "MacOSOnlineWebSocket.h"
#include "MacOSOnlineWSBridge.h"
#include "MacOSOnlineP2P.h"
#include "MacOSOnlineLobby.h"
#include "MacOSDebugLog.h"

void GenOnlineWS_InjectBuddyMessage(int profileID, const char* nick, const char* text);
void GenOnlineWS_InjectBuddyRequest(int profileID, const char* nick, const char* reason);
void GenOnlineWS_InjectBuddyStatus(int profileID, const char* nick, int gpStatus, const char* statusString, const char* location);

extern "C" {
void GenOnlineWS_InjectQMStatus(int qmStatus);
void GenOnlineWS_InjectQMJoinLobby(long long lobbyId);
void GenOnlineWS_InjectGameStart();
}

#ifdef __APPLE__

static GenOnlineWSState s_wsState = GenOnlineWSState::Disconnected;
static NSURLSessionWebSocketTask* s_wsTask = nil;
static NSURLSession* s_wsSession = nil;
static CFAbsoluteTime s_lastPingTime = 0;
static const CFTimeInterval kPingInterval = 15.0;

static void scheduleReceive();

static void handleStartGame(NSDictionary* json) {
  DLOG_NETWORK("GenOnlineWS: START_GAME received — game is launching!");
}

static void handleStartSignalling(NSDictionary* json) {
  NSNumber* userId = json[@"user_id"];
  NSNumber* lobbyId = json[@"lobby_id"];
  NSNumber* preferredPort = json[@"preferred_port"];
  if (!userId) return;

  long long uid = [userId longLongValue];
  long long lid = lobbyId ? [lobbyId longLongValue] : 0;
  int port = preferredPort ? [preferredPort intValue] : 0;
  GenOnlineP2P_OnStartSignalling(lid, uid, port);
}

static void handleNetworkSignal(NSDictionary* json) {
  NSNumber* userId = json[@"target_user_id"];
  NSArray* payload = json[@"payload"];
  if (!userId) return;

  long long uid = [userId longLongValue];
  int payloadLen = payload ? (int)[payload count] : 0;

  unsigned char payloadBytes[256];
  int bytesToCopy = (payloadLen > 256) ? 256 : payloadLen;
  for (int i = 0; i < bytesToCopy; ++i) {
    payloadBytes[i] = (unsigned char)[[payload objectAtIndex:i] unsignedCharValue];
  }

  GenOnlineP2P_OnNetworkSignal(uid, payloadBytes, bytesToCopy);
}

static void handleDisconnectPlayer(NSDictionary* json) {
  NSNumber* userId = json[@"user_id"];
  NSNumber* lobbyId = json[@"lobby_id"];
  if (!userId) return;

  long long uid = [userId longLongValue];
  long long lid = lobbyId ? [lobbyId longLongValue] : 0;
  GenOnlineP2P_OnDisconnectPlayer(lid, uid);
}

static void handleFullMeshCheckResponse(NSDictionary* json) {
  GenOnlineP2P_OnFullMeshCheckResponse();
}

static void handleFullMeshCheckComplete(NSDictionary* json) {
  NSDictionary* connectivityMap = json[@"connectivity_map"];
  DLOG_NETWORK("GenOnlineWS: FULL_MESH_CHECK_COMPLETE — %lu entries",
               connectivityMap ? (unsigned long)[connectivityMap count] : 0);
}

static void handleChatFromServer(NSDictionary* json) {
  NSString* displayName = json[@"display_name"];
  NSString* messageText = json[@"message"];
  NSNumber* action = json[@"action"];
  if (!displayName || !messageText) return;

  int isAction = (action && [action boolValue]) ? 1 : 0;
  GenOnlineWS_InjectChatMessage(
      [displayName UTF8String], [messageText UTF8String], isAction);
}

static NSMutableDictionary<NSString*, NSNumber*>* s_currentMembers = nil;

static void handleMemberListUpdate(NSDictionary* json) {
  NSArray* members = json[@"members"];
  if (!members || ![members isKindOfClass:[NSArray class]]) return;

  if (!s_currentMembers) {
    s_currentMembers = [NSMutableDictionary new];
  }

  NSMutableDictionary<NSString*, NSNumber*>* newMembers = [NSMutableDictionary dictionaryWithCapacity:members.count];
  for (NSDictionary* member in members) {
    NSString* name = member[@"Name"];
    NSNumber* userId = member[@"UserID"];
    if (!name) continue;
    newMembers[name] = userId ?: @(0);
  }

  for (NSString* name in s_currentMembers) {
    if (!newMembers[name]) {
      GenOnlineWS_InjectMemberLeft([name UTF8String]);
    }
  }

  for (NSString* name in newMembers) {
    if (!s_currentMembers[name]) {
      GenOnlineWS_InjectMemberJoin([name UTF8String], [newMembers[name] intValue]);
    }
  }

  DLOG_NETWORK("GenOnlineWS: member list: %lu total (was %lu)",
               (unsigned long)newMembers.count,
               (unsigned long)s_currentMembers.count);

  [s_currentMembers setDictionary:newMembers];
}

static void handleLobbyChatFromServer(NSDictionary* json) {
  NSString* sender = json[@"sender"];
  NSString* message = json[@"message"];
  if (!sender || !message) return;

  NSString* actualMessage = message;
  NSString* actualSender = sender;

  if ([message hasPrefix:@"["]) {
    NSRange closeBracket = [message rangeOfString:@"] "];
    if (closeBracket.location != NSNotFound) {
      actualSender = [message substringWithRange:NSMakeRange(1, closeBracket.location - 1)];
      actualMessage = [message substringFromIndex:closeBracket.location + closeBracket.length];
    }
  }

  if ([actualMessage hasPrefix:@"__UTM__SL:"]) {
    NSString* options = [actualMessage substringFromIndex:[@"__UTM__SL:" length]];
    GenOnlineWS_InjectStagingRoomUTM([actualSender UTF8String], [options UTF8String]);
    DLOG_NETWORK("GenOnlineWS: injected UTM_SL from '%s'", [actualSender UTF8String]);
    return;
  }

  if ([actualMessage hasPrefix:@"__UTM__"]) {
    NSString* rest = [actualMessage substringFromIndex:[@"__UTM__" length]];
    std::string full([rest UTF8String]);
    size_t slashPos = full.find('/');
    if (slashPos != std::string::npos) {
      std::string cmd = full.substr(0, slashPos);
      std::string opts = full.substr(slashPos + 1);
      GenOnlineWS_InjectPlayerUTM([actualSender UTF8String], cmd.c_str(), opts.c_str());
      DLOG_NETWORK("GenOnlineWS: injected UTM_PLAYER from '%s' cmd='%s' opts='%s'",
                   [actualSender UTF8String], cmd.c_str(), opts.c_str());
    }
    return;
  }

  GenOnlineWS_InjectChatMessage(
      [actualSender UTF8String],
      [actualMessage UTF8String],
      false);

  DLOG_NETWORK("GenOnlineWS: lobby chat from '%s': %s",
               [actualSender UTF8String], [actualMessage UTF8String]);
}

static void handleLobbyUpdate(NSDictionary* json) {
  int lobbyId = GenOnlineLobby_GetCreatedLobbyId();

  DLOG_NETWORK("GenOnlineWS: lobby update notification, fetching details for id=%d",
               lobbyId);

  if (lobbyId > 0) {
    GenOnlineLobby_FetchDetails(lobbyId);
  }
}

static void handleLobbyListUpdate(NSDictionary* json) {
  DLOG_NETWORK("GenOnlineWS: lobby list update notification, fetching via REST");
  GenOnlineLobby_FetchList();
}

static void handleMessage(NSURLSessionWebSocketMessage* message) {
  if (message.type != NSURLSessionWebSocketMessageTypeString) return;

  NSString* text = message.string;
  DLOG_NETWORK("GenOnlineWS: recv: %s", [text UTF8String]);

  NSData* jsonData = [text dataUsingEncoding:NSUTF8StringEncoding];
  NSError* err = nil;
  NSDictionary* json =
      [NSJSONSerialization JSONObjectWithData:jsonData options:0 error:&err];
  if (err || !json) {
    DLOG_NETWORK("GenOnlineWS: JSON parse error on received message");
    return;
  }

  NSNumber* msgId = json[@"msg_id"];
  if (!msgId) return;

  int wsMessageId = [msgId intValue];
  DLOG_NETWORK("GenOnlineWS: msg_id=%d", wsMessageId);

  switch (wsMessageId) {
  case 2:
    handleChatFromServer(json);
    break;
  case 4:
    handleMemberListUpdate(json);
    break;
  case 6:
    handleLobbyUpdate(json);
    break;
  case 7:
    handleLobbyListUpdate(json);
    break;
  case 11:
    handleLobbyChatFromServer(json);
    break;
  case 12:
    handleNetworkSignal(json);
    break;
  case 13:
    handleStartGame(json);
    break;
  case 15:
    DLOG_NETWORK("GenOnlineWS: PONG received");
    break;
  case 17:
    handleStartSignalling(json);
    break;
  case 18:
    handleDisconnectPlayer(json);
    break;
  case 27:
    handleFullMeshCheckResponse(json);
    break;
  case 28:
    handleFullMeshCheckComplete(json);
    break;
  case 20: {
    long long lobbyId = [json[@"lobby_id"] longLongValue];
    DLOG_NETWORK("GenOnlineWS: MATCHMAKING_JOIN_LOBBY lobby_id=%lld", lobbyId);
    GenOnlineWS_InjectQMJoinLobby(lobbyId);
    break;
  }
  case 21: {
    DLOG_NETWORK("GenOnlineWS: MATCHMAKING_START_GAME — injecting GAMESTART");
    GenOnlineWS_InjectGameStart();
    break;
  }
  case 22: {
    NSString* message = json[@"message"];
    DLOG_NETWORK("GenOnlineWS: MATCHMAKING_MESSAGE: '%s'",
                 message ? [message UTF8String] : "");
    break;
  }
  case 29: {
    NSString* displayName = json[@"display_name"];
    DLOG_NETWORK("GenOnlineWS: SOCIAL_NEW_FRIEND_REQUEST from '%s'",
                 displayName ? [displayName UTF8String] : "");
    GenOnlineWS_InjectBuddyRequest(0, displayName ? [displayName UTF8String] : "",
                                    "wants to be your friend");
    break;
  }
  case 31: {
    long long sourceUserId = [json[@"source_user_id"] longLongValue];
    NSString* message = json[@"message"];
    DLOG_NETWORK("GenOnlineWS: SOCIAL_FRIEND_CHAT from %lld: '%s'",
                 sourceUserId, message ? [message UTF8String] : "");
    GenOnlineWS_InjectBuddyMessage((int)sourceUserId, "",
                                    message ? [message UTF8String] : "");
    break;
  }
  case 32: {
    NSString* displayName = json[@"display_name"];
    bool online = [json[@"online"] boolValue];
    DLOG_NETWORK("GenOnlineWS: SOCIAL_FRIEND_STATUS_CHANGED '%s' online=%d",
                 displayName ? [displayName UTF8String] : "", online);
    GenOnlineWS_InjectBuddyStatus(0, displayName ? [displayName UTF8String] : "",
                                   online ? 1 : 0, online ? "Online" : "Offline", "");
    break;
  }
  case 35: {
    int numOnline = [json[@"num_online"] intValue];
    int numPending = [json[@"num_pending"] intValue];
    DLOG_NETWORK("GenOnlineWS: SOCIAL_OVERALL_STATUS online=%d pending=%d",
                 numOnline, numPending);
    break;
  }
  case 36: {
    NSString* displayName = json[@"display_name"];
    DLOG_NETWORK("GenOnlineWS: SOCIAL_FRIEND_REQUEST_ACCEPTED by '%s'",
                 displayName ? [displayName UTF8String] : "");
    GenOnlineWS_InjectBuddyStatus(0, displayName ? [displayName UTF8String] : "",
                                   1, "Online", "");
    break;
  }
  case 37: {
    DLOG_NETWORK("GenOnlineWS: SOCIAL_FRIENDS_LIST_DIRTY (re-fetch needed)");
    break;
  }
  default:
    DLOG_NETWORK("GenOnlineWS: unhandled msg_id=%d", wsMessageId);
    break;
  }
}

static void scheduleReceive() {
  if (!s_wsTask) return;
  if (s_wsState != GenOnlineWSState::Connected) return;

  [s_wsTask
      receiveMessageWithCompletionHandler:^(
          NSURLSessionWebSocketMessage* message, NSError* error) {
        if (error) {
          DLOG_NETWORK("GenOnlineWS: receive error: %s",
                       [[error localizedDescription] UTF8String]);
          s_wsState = GenOnlineWSState::Error;
          return;
        }

        handleMessage(message);
        scheduleReceive();
      }];
}

static void sendJSON(NSDictionary* json) {
  if (!s_wsTask || s_wsState != GenOnlineWSState::Connected) {
    NSNumber* msgId = json[@"msg_id"];
    DLOG_NETWORK("GenOnlineWS: sendJSON DROPPED (not connected, state=%d) msg_id=%d",
                 (int)s_wsState, msgId ? [msgId intValue] : -1);
    return;
  }

  NSError* err = nil;
  NSData* data = [NSJSONSerialization dataWithJSONObject:json
                                                options:0
                                                  error:&err];
  if (err) return;

  NSString* str = [[NSString alloc] initWithData:data
                                        encoding:NSUTF8StringEncoding];
  NSURLSessionWebSocketMessage* msg =
      [[NSURLSessionWebSocketMessage alloc] initWithString:str];

  [s_wsTask sendMessage:msg
      completionHandler:^(NSError* error) {
        if (error) {
          DLOG_NETWORK("GenOnlineWS: send error: %s",
                       [[error localizedDescription] UTF8String]);
        }
      }];
}

void GenOnlineWS_Connect(const char* wsUri, const char* sessionToken) {
  if (s_wsState == GenOnlineWSState::Connected ||
      s_wsState == GenOnlineWSState::Connecting) {
    DLOG_NETWORK("GenOnlineWS: already connected/connecting");
    return;
  }

  DLOG_NETWORK("GenOnlineWS: connecting to %s", wsUri);

  NSString* urlStr = [NSString stringWithUTF8String:wsUri];
  NSURL* url = [NSURL URLWithString:urlStr];
  if (!url) {
    DLOG_NETWORK("GenOnlineWS: invalid URL: %s", wsUri);
    s_wsState = GenOnlineWSState::Error;
    return;
  }

  NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:url];
  NSString* bearerToken =
      [NSString stringWithFormat:@"Bearer %s", sessionToken];
  [request setValue:bearerToken forHTTPHeaderField:@"Authorization"];

  if (!s_wsSession) {
    NSURLSessionConfiguration* config =
        [NSURLSessionConfiguration defaultSessionConfiguration];
    s_wsSession = [NSURLSession sessionWithConfiguration:config];
  }

  s_wsTask = [s_wsSession webSocketTaskWithRequest:request];
  s_wsState = GenOnlineWSState::Connecting;
  s_lastPingTime = CFAbsoluteTimeGetCurrent();

  [s_wsTask resume];

  [s_wsTask sendPingWithPongReceiveHandler:^(NSError* error) {
    if (error) {
      DLOG_NETWORK("GenOnlineWS: initial ping failed: %s",
                   [[error localizedDescription] UTF8String]);
      s_wsState = GenOnlineWSState::Error;
      return;
    }

    DLOG_NETWORK("GenOnlineWS: CONNECTED (pong received)");
    s_wsState = GenOnlineWSState::Connected;
    scheduleReceive();
  }];
}

void GenOnlineWS_Disconnect() {
  DLOG_NETWORK("GenOnlineWS: disconnecting");
  if (s_wsTask) {
    [s_wsTask cancelWithCloseCode:NSURLSessionWebSocketCloseCodeNormalClosure
                           reason:nil];
    s_wsTask = nil;
  }
  s_wsState = GenOnlineWSState::Disconnected;
}

void GenOnlineWS_Update() {
  if (s_wsState != GenOnlineWSState::Connected) return;

  CFAbsoluteTime now = CFAbsoluteTimeGetCurrent();
  if (now - s_lastPingTime < kPingInterval) return;
  s_lastPingTime = now;

  GenOnlineWS_SendPing();
}

GenOnlineWSState GenOnlineWS_GetState() { return s_wsState; }

void GenOnlineWS_SendPing() {
  NSDictionary* msg = @{@"msg_id" : @(14)};
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent PING");
}

static volatile int s_measuredLatencyMs = -1;
static volatile bool s_latencyMeasureInFlight = false;

void GenOnlineWS_StartLatencyMeasurement() {
  s_measuredLatencyMs = -1;
  s_latencyMeasureInFlight = true;

  if (!s_wsTask || s_wsState != GenOnlineWSState::Connected) {
    DLOG_NETWORK("GenOnlineWS: StartLatencyMeasurement — WS not connected, will retry in update");
    return;
  }

  CFAbsoluteTime startTime = CFAbsoluteTimeGetCurrent();

  [s_wsTask sendPingWithPongReceiveHandler:^(NSError* error) {
    if (!error) {
      CFAbsoluteTime elapsed = CFAbsoluteTimeGetCurrent() - startTime;
      int rttMs = (int)(elapsed * 1000.0);
      s_measuredLatencyMs = rttMs;
      DLOG_NETWORK("GenOnlineWS: latency measured = %dms (WS ping/pong)", rttMs);
    } else {
      DLOG_NETWORK("GenOnlineWS: latency ping failed: %s",
                   [[error localizedDescription] UTF8String]);
      s_measuredLatencyMs = -1;
    }
    s_latencyMeasureInFlight = false;
  }];
}

int GenOnlineWS_GetMeasuredLatency() {
  if (s_latencyMeasureInFlight) {
    if (s_wsState == GenOnlineWSState::Connected && s_measuredLatencyMs == -1) {
      CFAbsoluteTime startTime = CFAbsoluteTimeGetCurrent();
      [s_wsTask sendPingWithPongReceiveHandler:^(NSError* error) {
        if (!error) {
          CFAbsoluteTime elapsed = CFAbsoluteTimeGetCurrent() - startTime;
          int rttMs = (int)(elapsed * 1000.0);
          s_measuredLatencyMs = rttMs;
          DLOG_NETWORK("GenOnlineWS: latency measured (retry) = %dms", rttMs);
        }
        s_latencyMeasureInFlight = false;
      }];
    }
    return -1;
  }
  return s_measuredLatencyMs;
}

void GenOnlineWS_SendChangeRoom(int roomId) {
  NSDictionary* msg = @{@"msg_id" : @(3), @"room" : @(roomId)};
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent ChangeRoom(%d)", roomId);
}

void GenOnlineWS_SendChat(const char* message) {
  NSString* msgStr = [NSString stringWithUTF8String:message];
  NSDictionary* msg = @{
    @"msg_id" : @(201),
    @"message" : msgStr,
    @"action" : @(NO)
  };
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent Chat: %s", message);
}

void GenOnlineWS_SendLobbyChat(const char* message) {
  NSString* msgStr = [NSString stringWithUTF8String:message];
  NSDictionary* msg = @{
    @"msg_id" : @(10),
    @"message" : msgStr
  };
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent LobbyChat: %s", message);
}

void GenOnlineWS_SendReady(int ready) {
  NSDictionary* msg = @{
    @"msg_id" : @(5),
    @"ready" : @(ready ? YES : NO)
  };
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent Ready(%d)", ready);
}

void GenOnlineWS_SendStartGame() {
  NSDictionary* msg = @{ @"msg_id" : @(13) };
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent StartGame");
}

void GenOnlineWS_SendStartCountdown() {
  NSDictionary* msg = @{ @"msg_id" : @(23) };
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent StartCountdown");
}

void GenOnlineWS_SendRemovePassword() {
  NSDictionary* msg = @{ @"msg_id" : @(24) };
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent RemovePassword");
}

void GenOnlineWS_SendChangePassword(const char* password) {
  NSString* pwStr = [NSString stringWithUTF8String:password];
  NSDictionary* msg = @{
    @"msg_id" : @(25),
    @"password" : pwStr
  };
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent ChangePassword");
}

void GenOnlineWS_SendFullMeshCheckBegin() {
  NSDictionary* msg = @{ @"msg_id" : @(26) };
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent FullMeshCheckBegin");
}

void GenOnlineWS_SendFullMeshCheckResponse(const long long* userIds, int count) {
  NSMutableArray* connectivityMap = [NSMutableArray arrayWithCapacity:count];
  for (int i = 0; i < count; ++i) {
    [connectivityMap addObject:@(userIds[i])];
  }
  NSDictionary* msg = @{
    @"msg_id" : @(27),
    @"connectivity_map" : connectivityMap
  };
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent FullMeshCheckResponse (%d peers)", count);
}

void GenOnlineWS_SendRequestSignalling(long long targetUserId) {
  NSDictionary* msg = @{
    @"msg_id" : @(19),
    @"target_user_id" : @(targetUserId)
  };
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent RequestSignalling(target=%lld)", targetUserId);
}

void GenOnlineWS_SendSignal(long long targetUserId, const unsigned char* payload, int payloadLen) {
  NSMutableArray* payloadArr = [NSMutableArray arrayWithCapacity:payloadLen];
  for (int i = 0; i < payloadLen; ++i) {
    [payloadArr addObject:@(payload[i])];
  }
  NSDictionary* msg = @{
    @"msg_id" : @(12),
    @"target_user_id" : @(targetUserId),
    @"payload" : payloadArr
  };
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent Signal(target=%lld, %d bytes)", targetUserId, payloadLen);
}

void GenOnlineWS_SendFriendChat(long long targetUserId, const char* message) {
  NSDictionary* msg = @{
    @"msg_id" : @(30),
    @"target_user_id" : @(targetUserId),
    @"message" : [NSString stringWithUTF8String:message]
  };
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent FriendChat(target=%lld, msg=%s)", targetUserId, message);
}

#endif // __APPLE__
