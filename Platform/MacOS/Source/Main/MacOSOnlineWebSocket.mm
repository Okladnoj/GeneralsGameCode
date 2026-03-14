#import <Foundation/Foundation.h>
#include "MacOSOnlineWebSocket.h"
#include "MacOSOnlineWSBridge.h"
#include "MacOSOnlineP2P.h"
#include "MacOSOnlineLobby.h"
#include "MacOSDebugLog.h"

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

static void handleMemberListUpdate(NSDictionary* json) {
  NSArray* members = json[@"members"];
  if (!members || ![members isKindOfClass:[NSArray class]]) return;

  GenOnlineWS_InjectMemberListBegin();

  for (NSDictionary* member in members) {
    NSString* name = member[@"display_name"];
    NSNumber* userId = member[@"user_id"];
    if (!name) continue;

    int profileID = userId ? [userId intValue] : 0;
    GenOnlineWS_InjectMemberJoin([name UTF8String], profileID);
  }

  DLOG_NETWORK("GenOnlineWS: member list updated (%lu members)",
               (unsigned long)[members count]);
}

static void handleLobbyChatFromServer(NSDictionary* json) {
  NSString* sender = json[@"sender"];
  NSString* message = json[@"message"];
  if (!sender || !message) return;

  GenOnlineWS_InjectChatMessage(
      [sender UTF8String],
      [message UTF8String],
      false);

  DLOG_NETWORK("GenOnlineWS: lobby chat from '%s': %s",
               [sender UTF8String], [message UTF8String]);
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

#endif // __APPLE__
