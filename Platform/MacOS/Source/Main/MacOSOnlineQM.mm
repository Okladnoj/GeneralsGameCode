#import <Foundation/Foundation.h>
#include <string>
#include <vector>
#include "MacOSOnlineLogin.h"
#include "MacOSDebugLog.h"

#ifdef __APPLE__

static const char* kQMApiURL = "https://api.playgenerals.online";
static const char* kQMEnv = "live";
static const char* kQMContract = "1";

static NSURLSession* s_qmSession = nil;

static NSURLSession* getQMSession() {
  if (!s_qmSession) {
    NSURLSessionConfiguration* config =
        [NSURLSessionConfiguration ephemeralSessionConfiguration];
    config.timeoutIntervalForRequest = 15.0;
    s_qmSession = [NSURLSession sessionWithConfiguration:config];
  }
  return s_qmSession;
}

static NSMutableURLRequest* createQMRequest(NSString* urlStr,
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

bool GenOnlineQM_Register(int playlistId, const std::vector<int>& mapIndices,
                          unsigned int exeCRC, unsigned int iniCRC) {
  __block bool success = false;

  @autoreleasepool {
    NSString* urlStr = [NSString
        stringWithFormat:@"%s/env/%s/contract/%s/Matchmaking",
        kQMApiURL, kQMEnv, kQMContract];

    DLOG_NETWORK("QM_REGISTER: PUT %s (playlist=%d, maps=%zu, exe=0x%X, ini=0x%X)",
                 [urlStr UTF8String], playlistId, mapIndices.size(), exeCRC, iniCRC);

    NSMutableURLRequest* request = createQMRequest(urlStr, @"PUT");

    NSMutableArray* mapsArray = [NSMutableArray arrayWithCapacity:mapIndices.size()];
    for (int idx : mapIndices) {
      [mapsArray addObject:@(idx)];
    }

    NSDictionary* body = @{
      @"playlist" : @(playlistId),
      @"maps" : mapsArray,
      @"exe_crc" : @(exeCRC),
      @"ini_crc" : @(iniCRC)
    };

    NSError* jsonError = nil;
    NSData* jsonBody = [NSJSONSerialization dataWithJSONObject:body
                                                      options:0
                                                        error:&jsonError];
    if (jsonError) {
      DLOG_NETWORK("QM_REGISTER: JSON serialization error");
      return false;
    }
    [request setHTTPBody:jsonBody];

    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSURLSessionDataTask* task = [getQMSession()
        dataTaskWithRequest:request
          completionHandler:^(NSData* data, NSURLResponse* response,
                              NSError* error) {
            NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
            int statusCode = httpResp ? (int)[httpResp statusCode] : 0;

            DLOG_NETWORK("QM_REGISTER: status=%d error=%s",
                         statusCode,
                         error ? [[error description] UTF8String] : "none");

            success = (!error && statusCode >= 200 && statusCode < 300);
            dispatch_semaphore_signal(sema);
          }];
    [task resume];

    dispatch_semaphore_wait(sema, dispatch_time(DISPATCH_TIME_NOW, 15 * NSEC_PER_SEC));
  }

  return success;
}

bool GenOnlineQM_Widen() {
  __block bool success = false;

  @autoreleasepool {
    NSString* urlStr = [NSString
        stringWithFormat:@"%s/env/%s/contract/%s/Matchmaking/Widen",
        kQMApiURL, kQMEnv, kQMContract];

    DLOG_NETWORK("QM_WIDEN: POST %s", [urlStr UTF8String]);

    NSMutableURLRequest* request = createQMRequest(urlStr, @"POST");

    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSURLSessionDataTask* task = [getQMSession()
        dataTaskWithRequest:request
          completionHandler:^(NSData* data, NSURLResponse* response,
                              NSError* error) {
            NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
            int statusCode = httpResp ? (int)[httpResp statusCode] : 0;

            DLOG_NETWORK("QM_WIDEN: status=%d error=%s",
                         statusCode,
                         error ? [[error description] UTF8String] : "none");

            success = (!error && statusCode >= 200 && statusCode < 300);
            dispatch_semaphore_signal(sema);
          }];
    [task resume];

    dispatch_semaphore_wait(sema, dispatch_time(DISPATCH_TIME_NOW, 15 * NSEC_PER_SEC));
  }

  return success;
}

bool GenOnlineQM_Deregister() {
  __block bool success = false;

  @autoreleasepool {
    NSString* urlStr = [NSString
        stringWithFormat:@"%s/env/%s/contract/%s/Matchmaking",
        kQMApiURL, kQMEnv, kQMContract];

    DLOG_NETWORK("QM_DEREGISTER: DELETE %s", [urlStr UTF8String]);

    NSMutableURLRequest* request = createQMRequest(urlStr, @"DELETE");

    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSURLSessionDataTask* task = [getQMSession()
        dataTaskWithRequest:request
          completionHandler:^(NSData* data, NSURLResponse* response,
                              NSError* error) {
            NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
            int statusCode = httpResp ? (int)[httpResp statusCode] : 0;

            DLOG_NETWORK("QM_DEREGISTER: status=%d error=%s",
                         statusCode,
                         error ? [[error description] UTF8String] : "none");

            success = (!error && statusCode >= 200 && statusCode < 300);
            dispatch_semaphore_signal(sema);
          }];
    [task resume];

    dispatch_semaphore_wait(sema, dispatch_time(DISPATCH_TIME_NOW, 15 * NSEC_PER_SEC));
  }

  return success;
}

#endif
