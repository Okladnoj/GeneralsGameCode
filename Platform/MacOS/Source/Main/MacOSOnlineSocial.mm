#import <Foundation/Foundation.h>
#include <string>
#include <vector>
#include "MacOSOnlineLogin.h"
#include "MacOSDebugLog.h"

#ifdef __APPLE__

static const char* kSocialApiURL = "https://api.playgenerals.online";
static const char* kSocialEnv = "live";
static const char* kSocialContract = "1";

static NSURLSession* s_socialSession = nil;

static NSURLSession* getSocialSession() {
  if (!s_socialSession) {
    NSURLSessionConfiguration* config =
        [NSURLSessionConfiguration ephemeralSessionConfiguration];
    config.timeoutIntervalForRequest = 10.0;
    s_socialSession = [NSURLSession sessionWithConfiguration:config];
  }
  return s_socialSession;
}

static NSMutableURLRequest* createSocialRequest(NSString* urlStr,
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

struct GenOnlineFriendEntry {
  int64_t userId;
  char displayName[64];
  bool online;
  char presence[64];
};

struct GenOnlineFriendList {
  std::vector<GenOnlineFriendEntry> friends;
  std::vector<GenOnlineFriendEntry> pendingRequests;
};

GenOnlineFriendList GenOnlineSocial_FetchFriends() {
  GenOnlineFriendList result;

  @autoreleasepool {
    NSString* urlStr = [NSString
        stringWithFormat:@"%s/env/%s/contract/%s/Social/Friends",
        kSocialApiURL, kSocialEnv, kSocialContract];

    DLOG_NETWORK("SOCIAL_FETCH: GET %s", [urlStr UTF8String]);

    NSMutableURLRequest* request = createSocialRequest(urlStr, @"GET");

    __block GenOnlineFriendList blockResult;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSURLSessionDataTask* task = [getSocialSession()
        dataTaskWithRequest:request
          completionHandler:^(NSData* data, NSURLResponse* response,
                              NSError* error) {
            NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
            int statusCode = httpResp ? (int)[httpResp statusCode] : 0;

            DLOG_NETWORK("SOCIAL_FETCH: status=%d error=%s",
                         statusCode,
                         error ? [[error description] UTF8String] : "none");

            if (error || statusCode != 200 || !data) {
              dispatch_semaphore_signal(sema);
              return;
            }

            NSError* jsonError = nil;
            NSDictionary* json = [NSJSONSerialization JSONObjectWithData:data
                                    options:0 error:&jsonError];
            if (jsonError || ![json isKindOfClass:[NSDictionary class]]) {
              DLOG_NETWORK("SOCIAL_FETCH: JSON parse error");
              dispatch_semaphore_signal(sema);
              return;
            }

            NSArray* friends = json[@"friends"];
            if ([friends isKindOfClass:[NSArray class]]) {
              for (NSDictionary* f in friends) {
                GenOnlineFriendEntry entry = {};
                entry.userId = [f[@"user_id"] longLongValue];
                NSString* name = f[@"display_name"];
                if (name) strlcpy(entry.displayName, [name UTF8String], sizeof(entry.displayName));
                entry.online = [f[@"online"] boolValue];
                NSString* pres = f[@"presence"];
                if (pres) strlcpy(entry.presence, [pres UTF8String], sizeof(entry.presence));
                blockResult.friends.push_back(entry);
              }
            }

            NSArray* pending = json[@"pending_requests"];
            if ([pending isKindOfClass:[NSArray class]]) {
              for (NSDictionary* p in pending) {
                GenOnlineFriendEntry entry = {};
                entry.userId = [p[@"user_id"] longLongValue];
                NSString* name = p[@"display_name"];
                if (name) strlcpy(entry.displayName, [name UTF8String], sizeof(entry.displayName));
                blockResult.pendingRequests.push_back(entry);
              }
            }

            DLOG_NETWORK("SOCIAL_FETCH: %zu friends, %zu pending",
                         blockResult.friends.size(),
                         blockResult.pendingRequests.size());

            dispatch_semaphore_signal(sema);
          }];
    [task resume];

    dispatch_semaphore_wait(sema, dispatch_time(DISPATCH_TIME_NOW, 15 * NSEC_PER_SEC));
    result = blockResult;
  }

  return result;
}

static void socialRESTCall(NSString* method, NSString* path) {
  @autoreleasepool {
    NSString* urlStr = [NSString
        stringWithFormat:@"%s/env/%s/contract/%s/Social/%@",
        kSocialApiURL, kSocialEnv, kSocialContract, path];

    DLOG_NETWORK("SOCIAL_REST: %s %s", [method UTF8String], [urlStr UTF8String]);

    NSMutableURLRequest* request = createSocialRequest(urlStr, method);

    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSURLSessionDataTask* task = [getSocialSession()
        dataTaskWithRequest:request
          completionHandler:^(NSData* data, NSURLResponse* response,
                              NSError* error) {
            NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
            int statusCode = httpResp ? (int)[httpResp statusCode] : 0;
            DLOG_NETWORK("SOCIAL_REST: response status=%d error=%s",
                         statusCode,
                         error ? [[error description] UTF8String] : "none");
            dispatch_semaphore_signal(sema);
          }];
    [task resume];

    dispatch_semaphore_wait(sema, dispatch_time(DISPATCH_TIME_NOW, 15 * NSEC_PER_SEC));
  }
}

void GenOnlineSocial_SendFriendRequest(int64_t targetUserId) {
  NSString* path = [NSString stringWithFormat:@"Friends/Requests/%lld", targetUserId];
  socialRESTCall(@"PUT", path);
}

void GenOnlineSocial_AcceptFriendRequest(int64_t targetUserId) {
  NSString* path = [NSString stringWithFormat:@"Friends/Requests/%lld", targetUserId];
  socialRESTCall(@"POST", path);
}

void GenOnlineSocial_RejectFriendRequest(int64_t targetUserId) {
  NSString* path = [NSString stringWithFormat:@"Friends/Requests/%lld", targetUserId];
  socialRESTCall(@"DELETE", path);
}

void GenOnlineSocial_RemoveFriend(int64_t targetUserId) {
  NSString* path = [NSString stringWithFormat:@"Friends/%lld", targetUserId];
  socialRESTCall(@"DELETE", path);
}

#endif
