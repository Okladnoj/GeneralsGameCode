#import <Foundation/Foundation.h>
#include "MacOSOnlineWebSocket.h"
#include "MacOSDebugLog.h"

#ifdef __APPLE__

static GenOnlineWSState s_wsState = GenOnlineWSState::Disconnected;
static NSURLSessionWebSocketTask* s_wsTask = nil;
static NSURLSession* s_wsSession = nil;
static CFAbsoluteTime s_lastPingTime = 0;
static const CFTimeInterval kPingInterval = 15.0;

static void scheduleReceive();

static void handleMessage(NSURLSessionWebSocketMessage* message) {
  if (message.type == NSURLSessionWebSocketMessageTypeString) {
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

    int id = [msgId intValue];
    DLOG_NETWORK("GenOnlineWS: msg_id=%d", id);
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
  if (!s_wsTask || s_wsState != GenOnlineWSState::Connected) return;

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
  NSDictionary* msg = @{@"msg_id" : @(1)};
  sendJSON(msg);
  DLOG_NETWORK("GenOnlineWS: sent PING");
}

void GenOnlineWS_SendChangeRoom(int roomId) {
  NSDictionary* msg = @{@"msg_id" : @(200), @"room" : @(roomId)};
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

#endif // __APPLE__
