#import <Foundation/Foundation.h>
#include <string>
#include "MacOSOnlineLogin.h"
#include "MacOSDebugLog.h"

#ifdef __APPLE__

static const char* kStatsApiURL = "https://api.playgenerals.online";
static const char* kStatsEnv = "live";
static const char* kStatsContract = "1";

static NSURLSession* s_statsSession = nil;

static NSURLSession* getStatsSession() {
  if (!s_statsSession) {
    NSURLSessionConfiguration* config =
        [NSURLSessionConfiguration ephemeralSessionConfiguration];
    config.timeoutIntervalForRequest = 10.0;
    s_statsSession = [NSURLSession sessionWithConfiguration:config];
  }
  return s_statsSession;
}

static NSMutableURLRequest* createStatsRequest(NSString* urlStr,
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

static void appendIntArray(NSMutableString* kvStr, NSArray* arr,
                           const char* key) {
  for (NSUInteger i = 0; i < [arr count]; ++i) {
    int val = [arr[i] intValue];
    if (val > 0) {
      [kvStr appendFormat:@"\\%s%lu\\%d", key, (unsigned long)i, val];
    }
  }
}

static void appendScalarInt(NSMutableString* kvStr, const char* key,
                            int val) {
  if (val > 0) {
    [kvStr appendFormat:@"\\%s\\%d", key, val];
  }
}

static NSString* jsonStatsToKVPairs(NSDictionary* stats) {
  NSMutableString* kv = [NSMutableString string];

  struct { NSString* jsonKey; const char* kvKey; } arrayMaps[] = {
    {@"wins",           "wins"},
    {@"losses",         "losses"},
    {@"games",          "games"},
    {@"duration",       "duration"},
    {@"unitsKilled",    "unitsKilled"},
    {@"unitsLost",      "unitsLost"},
    {@"unitsBuilt",     "unitsBuilt"},
    {@"buildingsKilled","buildingsKilled"},
    {@"buildingsLost",  "buildingsLost"},
    {@"buildingsBuilt", "buildingsBuilt"},
    {@"earnings",       "earnings"},
    {@"techCaptured",   "techCaptured"},
    {@"discons",        "discons"},
    {@"desyncs",        "desyncs"},
    {@"surrenders",     "surrenders"},
    {@"gamesOf2p",      "gamesOf2p"},
    {@"gamesOf3p",      "gamesOf3p"},
    {@"gamesOf4p",      "gamesOf4p"},
    {@"gamesOf5p",      "gamesOf5p"},
    {@"gamesOf6p",      "gamesOf6p"},
    {@"gamesOf7p",      "gamesOf7p"},
    {@"gamesOf8p",      "gamesOf8p"},
    {@"customGames",    "customGames"},
    {@"QMGames",        "QMGames"},
  };

  for (auto& m : arrayMaps) {
    NSArray* arr = stats[m.jsonKey];
    if ([arr isKindOfClass:[NSArray class]]) {
      appendIntArray(kv, arr, m.kvKey);
    }
  }

  struct { NSString* jsonKey; const char* kvKey; } scalarMaps[] = {
    {@"locale",                     "locale"},
    {@"gamesAsRandom",              "random"},
    {@"lastGeneral",                "lastGeneral"},
    {@"gamesInRowWithLastGeneral",  "genInRow"},
    {@"builtParticleCannon",        "builtCannon"},
    {@"builtNuke",                  "builtNuke"},
    {@"builtSCUD",                  "builtSCUD"},
    {@"challengeMedals",            "challenge"},
    {@"battleHonors",               "battle"},
    {@"winsInARow",                 "WinRow"},
    {@"maxWinsInARow",              "WinRowMax"},
    {@"lossesInARow",               "LossRow"},
    {@"maxLossesInARow",            "LossRowMax"},
    {@"disconsInARow",              "DCRow"},
    {@"maxDisconsInARow",           "DCRowMax"},
    {@"desyncsInARow",              "DSRow"},
    {@"maxDesyncsInARow",           "DSRowMax"},
    {@"lastLadderPort",             "ladderPort"},
  };

  for (auto& m : scalarMaps) {
    NSNumber* val = stats[m.jsonKey];
    if ([val isKindOfClass:[NSNumber class]]) {
      appendScalarInt(kv, m.kvKey, [val intValue]);
    }
  }

  NSString* lastFPSStr = stats[@"lastFPS"];
  if ([lastFPSStr isKindOfClass:[NSNumber class]]) {
    float fps = [(NSNumber*)lastFPSStr floatValue];
    if (fps > 0.0f) {
      [kv appendFormat:@"\\fps\\%g", fps];
    }
  }

  NSString* options = stats[@"options"];
  if ([options isKindOfClass:[NSString class]] && [options length] > 0) {
    [kv appendFormat:@"\\options\\%@", options];
  }

  NSString* systemSpec = stats[@"systemSpec"];
  if ([systemSpec isKindOfClass:[NSString class]] && [systemSpec length] > 0) {
    [kv appendFormat:@"\\systemSpec\\%@", systemSpec];
  }

  NSString* ladderHost = stats[@"lastLadderHost"];
  if ([ladderHost isKindOfClass:[NSString class]] && [ladderHost length] > 0) {
    [kv appendFormat:@"\\ladderHost\\%@", ladderHost];
  }

  return kv;
}

std::string GenOnlineStats_FetchKVPairs(int profileId) {
  @autoreleasepool {
    NSString* urlStr = [NSString
        stringWithFormat:@"%s/env/%s/contract/%s/PlayerStats/%d",
        kStatsApiURL, kStatsEnv, kStatsContract, profileId];

    DLOG_NETWORK("STATS_FETCH: GET %s", [urlStr UTF8String]);

    NSMutableURLRequest* request = createStatsRequest(urlStr, @"GET");

    __block std::string result;
    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSURLSessionDataTask* task = [getStatsSession()
        dataTaskWithRequest:request
          completionHandler:^(NSData* data, NSURLResponse* response,
                              NSError* error) {
            NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
            int statusCode = httpResp ? (int)[httpResp statusCode] : 0;

            DLOG_NETWORK("STATS_FETCH: response status=%d error=%s",
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
              DLOG_NETWORK("STATS_FETCH: JSON parse error");
              dispatch_semaphore_signal(sema);
              return;
            }

            NSDictionary* stats = json[@"stats"];
            if (![stats isKindOfClass:[NSDictionary class]]) {
              DLOG_NETWORK("STATS_FETCH: no 'stats' object in response");
              dispatch_semaphore_signal(sema);
              return;
            }

            NSString* kvPairs = jsonStatsToKVPairs(stats);
            result = std::string([kvPairs UTF8String]);
            DLOG_NETWORK("STATS_FETCH: converted to KV len=%zu", result.size());

            dispatch_semaphore_signal(sema);
          }];
    [task resume];

    dispatch_semaphore_wait(sema, dispatch_time(DISPATCH_TIME_NOW, 15 * NSEC_PER_SEC));

    return result;
  }
}

void GenOnlineStats_UpdateKVPairs(int profileId,
                                  const std::string& kvPairs) {
  @autoreleasepool {
    NSString* urlStr = [NSString
        stringWithFormat:@"%s/env/%s/contract/%s/PlayerStats",
        kStatsApiURL, kStatsEnv, kStatsContract];

    DLOG_NETWORK("STATS_UPDATE: PUT %s kvLen=%zu",
                 [urlStr UTF8String], kvPairs.size());

    NSMutableURLRequest* request = createStatsRequest(urlStr, @"PUT");

    NSString* kvNS = [NSString stringWithUTF8String:kvPairs.c_str()];
    NSMutableArray* statArray = [NSMutableArray array];

    struct { const char* kvKey; int count; bool isArray; } statOrder[] = {
      {"wins", 15, true},
      {"losses", 15, true},
      {"games", 15, true},
      {"duration", 15, true},
      {"unitsKilled", 15, true},
      {"unitsLost", 15, true},
      {"unitsBuilt", 15, true},
      {"buildingsKilled", 15, true},
      {"buildingsLost", 15, true},
      {"buildingsBuilt", 15, true},
      {"earnings", 15, true},
      {"techCaptured", 15, true},
      {"discons", 15, true},
      {"desyncs", 15, true},
      {"surrenders", 15, true},
      {"gamesOf2p", 15, true},
      {"gamesOf3p", 15, true},
      {"gamesOf4p", 15, true},
      {"gamesOf5p", 15, true},
      {"gamesOf6p", 15, true},
      {"gamesOf7p", 15, true},
      {"gamesOf8p", 15, true},
      {"customGames", 15, true},
      {"QMGames", 15, true},
    };

    NSMutableDictionary* kvDict = [NSMutableDictionary dictionary];
    NSString* remaining = kvNS;
    while ([remaining length] > 0) {
      NSRange firstSlash = [remaining rangeOfString:@"\\"];
      if (firstSlash.location == NSNotFound) break;
      remaining = [remaining substringFromIndex:firstSlash.location + 1];

      NSRange secondSlash = [remaining rangeOfString:@"\\"];
      if (secondSlash.location == NSNotFound) break;

      NSString* key = [remaining substringToIndex:secondSlash.location];
      remaining = [remaining substringFromIndex:secondSlash.location + 1];

      NSRange thirdSlash = [remaining rangeOfString:@"\\"];
      NSString* val;
      if (thirdSlash.location == NSNotFound) {
        val = remaining;
        remaining = @"";
      } else {
        val = [remaining substringToIndex:thirdSlash.location];
        remaining = [remaining substringFromIndex:thirdSlash.location];
      }

      kvDict[key] = val;
    }

    for (auto& stat : statOrder) {
      for (int i = 0; i < stat.count; ++i) {
        NSString* key = [NSString stringWithFormat:@"%s%d", stat.kvKey, i];
        NSString* val = kvDict[key];
        [statArray addObject:val ? @([val intValue]) : @0];
      }
    }

    struct { const char* kvKey; } scalarOrder[] = {
      {"locale"}, {"random"},
      {NULL}, {NULL},
      {"fps"},
      {"lastGeneral"}, {"genInRow"},
      {"challenge"}, {"battle"},
      {NULL}, {NULL},
      {"WinRow"}, {"WinRowMax"},
      {"LossRow"}, {"LossRowMax"},
      {"DCRow"}, {"DCRowMax"},
      {"DSRow"}, {"DSRowMax"},
      {"builtCannon"}, {"builtNuke"}, {"builtSCUD"},
      {"ladderPort"},
    };

    for (auto& s : scalarOrder) {
      if (s.kvKey) {
        NSString* key = [NSString stringWithUTF8String:s.kvKey];
        NSString* val = kvDict[key];
        [statArray addObject:val ? @([val intValue]) : @0];
      } else {
        [statArray addObject:[NSNull null]];
      }
    }

    NSError* jsonError = nil;
    NSData* body = [NSJSONSerialization dataWithJSONObject:statArray
                                                  options:0
                                                    error:&jsonError];
    if (jsonError) {
      DLOG_NETWORK("STATS_UPDATE: JSON serialization error");
      return;
    }

    [request setHTTPBody:body];

    dispatch_semaphore_t sema = dispatch_semaphore_create(0);

    NSURLSessionDataTask* task = [getStatsSession()
        dataTaskWithRequest:request
          completionHandler:^(NSData* data, NSURLResponse* response,
                              NSError* error) {
            NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
            int statusCode = httpResp ? (int)[httpResp statusCode] : 0;
            DLOG_NETWORK("STATS_UPDATE: response status=%d error=%s",
                         statusCode,
                         error ? [[error description] UTF8String] : "none");
            dispatch_semaphore_signal(sema);
          }];
    [task resume];

    dispatch_semaphore_wait(sema, dispatch_time(DISPATCH_TIME_NOW, 15 * NSEC_PER_SEC));
  }
}

#endif
