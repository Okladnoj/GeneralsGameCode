#import <Cocoa/Cocoa.h>
#include "MacOSMapDownloader.h"
#include "MacOSDebugLog.h"

#ifdef __APPLE__

static const char* kRankedMapsURL =
    "https://gentool.net/download/Maps_All_Ranked_ZH.zip";
static const char* kCustomMapsURL =
    "https://github.com/TheSuperHackers/GeneralsRankedMaps/archive/refs/heads/main.zip";

static GenMapDownloadState s_state = GenMapDownloadState::Idle;
static int s_progress = 0;

static NSWindow* s_downloadWindow = nil;
static NSProgressIndicator* s_progressBar = nil;
static NSTextField* s_statusLabel = nil;
static NSButton* s_rankedButton = nil;
static NSButton* s_customButton = nil;
static NSButton* s_closeButton = nil;

static NSString* getMapsDirectory() {
    NSString* appSupport = [NSSearchPathForDirectoriesInDomains(
        NSApplicationSupportDirectory, NSUserDomainMask, YES) firstObject];
    if (!appSupport) {
        return nil;
    }
    return [appSupport stringByAppendingPathComponent:@"Generals Zero Hour/Maps"];
}

bool GenMapDownloader_HasRankedMaps() {
    NSString* mapsDir = getMapsDirectory();
    if (!mapsDir) {
        return false;
    }

    NSFileManager* fm = [NSFileManager defaultManager];
    BOOL isDir = NO;
    if (![fm fileExistsAtPath:mapsDir isDirectory:&isDir] || !isDir) {
        return false;
    }

    NSArray* contents = [fm contentsOfDirectoryAtPath:mapsDir error:nil];
    int mapCount = 0;
    for (NSString* item in contents) {
        if ([item hasPrefix:@"."]) {
            continue;
        }
        NSString* subPath = [mapsDir stringByAppendingPathComponent:item];
        BOOL subIsDir = NO;
        if ([fm fileExistsAtPath:subPath isDirectory:&subIsDir] && subIsDir) {
            mapCount++;
        }
    }

    DLOG_NETWORK("GenMapDownloader: HasRankedMaps — %d map folders found", mapCount);
    return mapCount >= 5;
}

static void setButtonsEnabled(bool download, bool close) {
    if (s_rankedButton)  [s_rankedButton setEnabled:download];
    if (s_customButton)  [s_customButton setEnabled:download];
    if (s_closeButton)   [s_closeButton setEnabled:close];
}

static void updateUI(NSString* status, int progress, bool enableDownload, bool enableClose) {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (s_statusLabel) {
            [s_statusLabel setStringValue:status];
        }
        if (s_progressBar) {
            [s_progressBar setDoubleValue:(double)progress];
        }
        setButtonsEnabled(enableDownload, enableClose);
    });
}

static void extractZip(NSString* zipPath, NSString* destDir, NSString* stripPrefix) {
    s_state = GenMapDownloadState::Extracting;
    updateUI(@"Extracting maps...", 100, false, false);

    NSFileManager* fm = [NSFileManager defaultManager];
    [fm createDirectoryAtPath:destDir withIntermediateDirectories:YES attributes:nil error:nil];

    NSString* extractDir = destDir;
    if (stripPrefix) {
        extractDir = [NSTemporaryDirectory() stringByAppendingPathComponent:@"MapExtractTemp"];
        [fm removeItemAtPath:extractDir error:nil];
    }

    NSTask* task = [[NSTask alloc] init];
    [task setLaunchPath:@"/usr/bin/unzip"];
    [task setArguments:@[@"-o", zipPath, @"-d", extractDir]];
    [task setStandardOutput:[NSFileHandle fileHandleWithNullDevice]];
    [task setStandardError:[NSFileHandle fileHandleWithNullDevice]];

    NSError* launchError = nil;
    [task launchAndReturnError:&launchError];

    if (launchError) {
        DLOG_NETWORK("GenMapDownloader: unzip launch error: %s",
                     [[launchError localizedDescription] UTF8String]);
        s_state = GenMapDownloadState::Error;
        updateUI(@"Extraction failed — unzip launch error", 0, true, true);
        return;
    }

    [task waitUntilExit];
    int exitCode = [task terminationStatus];

    [fm removeItemAtPath:zipPath error:nil];

    if (exitCode != 0) {
        DLOG_NETWORK("GenMapDownloader: unzip exit code %d", exitCode);
        s_state = GenMapDownloadState::Error;
        updateUI(@"Extraction failed", 0, true, true);
        return;
    }

    if (stripPrefix) {
        NSString* innerDir = [extractDir stringByAppendingPathComponent:stripPrefix];
        BOOL isDir = NO;
        int copied = 0;
        if ([fm fileExistsAtPath:innerDir isDirectory:&isDir] && isDir) {
            NSArray* subDirs = [fm contentsOfDirectoryAtPath:innerDir error:nil];
            for (NSString* subDir in subDirs) {
                NSString* subPath = [innerDir stringByAppendingPathComponent:subDir];
                BOOL subIsDir = NO;
                if (![fm fileExistsAtPath:subPath isDirectory:&subIsDir] || !subIsDir) {
                    continue;
                }
                NSArray* maps = [fm contentsOfDirectoryAtPath:subPath error:nil];
                for (NSString* mapFolder in maps) {
                    NSString* src = [subPath stringByAppendingPathComponent:mapFolder];
                    NSString* dst = [destDir stringByAppendingPathComponent:mapFolder];
                    [fm removeItemAtPath:dst error:nil];
                    [fm moveItemAtPath:src toPath:dst error:nil];
                    copied++;
                }
            }
        }
        [fm removeItemAtPath:extractDir error:nil];
        printf("GenMapDownloader: moved %d map folders from inner dirs\n", copied);
        fflush(stdout);
    }

    DLOG_NETWORK("GenMapDownloader: extraction complete");
    s_state = GenMapDownloadState::Done;
    s_progress = 100;
    updateUI(@"Maps installed!", 100, true, true);
}

static void startDownloadWithURL(const char* urlStr, NSString* zipName,
                                  NSString* label, NSString* stripPrefix) {
    s_state = GenMapDownloadState::Downloading;
    s_progress = 0;
    NSString* statusMsg = [NSString stringWithFormat:@"Downloading %@...", label];
    updateUI(statusMsg, 0, false, false);

    NSURL* url = [NSURL URLWithString:@(urlStr)];
    NSString* destZip = [NSTemporaryDirectory() stringByAppendingPathComponent:zipName];
    NSString* mapsDir = getMapsDirectory();

    NSURLSessionConfiguration* config = [NSURLSessionConfiguration defaultSessionConfiguration];
    config.timeoutIntervalForResource = 600.0;

    NSURLSession* session = [NSURLSession
        sessionWithConfiguration:config
                        delegate:nil
                   delegateQueue:nil];

    NSURLSessionDownloadTask* task = [session
        downloadTaskWithURL:url
          completionHandler:^(NSURL* location, NSURLResponse* response, NSError* error) {
              if (error) {
                  DLOG_NETWORK("GenMapDownloader: download error: %s",
                               [[error localizedDescription] UTF8String]);
                  s_state = GenMapDownloadState::Error;
                  updateUI(@"Download failed — check internet connection", 0, true, true);
                  return;
              }

              NSHTTPURLResponse* httpResp = (NSHTTPURLResponse*)response;
              long status = [httpResp statusCode];
              if (status < 200 || status >= 300) {
                  DLOG_NETWORK("GenMapDownloader: HTTP %ld", status);
                  s_state = GenMapDownloadState::Error;
                  NSString* msg = [NSString stringWithFormat:@"Download failed — HTTP %ld", status];
                  updateUI(msg, 0, true, true);
                  return;
              }

              NSFileManager* fm = [NSFileManager defaultManager];
              [fm removeItemAtPath:destZip error:nil];

              NSError* moveError = nil;
              [fm moveItemAtURL:location toURL:[NSURL fileURLWithPath:destZip] error:&moveError];
              if (moveError) {
                  DLOG_NETWORK("GenMapDownloader: move error: %s",
                               [[moveError localizedDescription] UTF8String]);
                  s_state = GenMapDownloadState::Error;
                  updateUI(@"Failed to save download", 0, true, true);
                  return;
              }

              NSDictionary* attrs = [fm attributesOfItemAtPath:destZip error:nil];
              long long fileSize = [attrs fileSize];
              DLOG_NETWORK("GenMapDownloader: downloaded %lld bytes to %s",
                           fileSize, [destZip UTF8String]);

              dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
                  extractZip(destZip, mapsDir, stripPrefix);
              });
          }];

    NSProgress* downloadProgress = [task progress];
    __block NSString* progressLabel = label;
    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^{
        while (s_state == GenMapDownloadState::Downloading) {
            double fraction = downloadProgress.fractionCompleted;
            int pct = (int)(fraction * 100.0);
            if (pct != s_progress) {
                s_progress = pct;
                NSString* msg = [NSString stringWithFormat:
                    @"Downloading %@... %d%%", progressLabel, pct];
                updateUI(msg, pct, false, false);
            }
            [NSThread sleepForTimeInterval:0.3];
        }
    });

    [task resume];
    DLOG_NETWORK("GenMapDownloader: download started from %s", urlStr);
}

static void onRankedClicked(id sender) {
    startDownloadWithURL(kRankedMapsURL,
                         @"Maps_All_Ranked_ZH.zip",
                         @"ranked maps",
                         nil);
}

static void onCustomClicked(id sender) {
    startDownloadWithURL(kCustomMapsURL,
                         @"GeneralsRankedMaps.zip",
                         @"custom maps (backlog)",
                         @"GeneralsRankedMaps-main/backlog");
}

static void dismissWindow() {
    if (!s_downloadWindow) {
        return;
    }

    [s_downloadWindow orderOut:nil];

    NSWindow* gameWin = [[NSApp windows] firstObject];
    if (gameWin && gameWin != s_downloadWindow) {
        [gameWin makeKeyAndOrderFront:nil];
    }

    s_downloadWindow = nil;
    s_progressBar = nil;
    s_statusLabel = nil;
    s_rankedButton = nil;
    s_customButton = nil;
    s_closeButton = nil;
}

@interface GenMapDownloaderActionHandler : NSObject
+ (void)onRankedAction:(id)sender;
+ (void)onCustomAction:(id)sender;
+ (void)onCloseAction:(id)sender;
@end

@implementation GenMapDownloaderActionHandler
+ (void)onRankedAction:(id)sender {
    onRankedClicked(sender);
}
+ (void)onCustomAction:(id)sender {
    onCustomClicked(sender);
}
+ (void)onCloseAction:(id)sender {
    dismissWindow();
}
@end

void GenMapDownloader_ShowWindow() {
    if (s_downloadWindow) {
        [s_downloadWindow makeKeyAndOrderFront:nil];
        return;
    }

    CGFloat winW = 480;
    CGFloat winH = 230;

    NSRect frame = NSMakeRect(0, 0, winW, winH);
    NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable;

    s_downloadWindow = [[NSWindow alloc] initWithContentRect:frame
                                                   styleMask:styleMask
                                                     backing:NSBackingStoreBuffered
                                                       defer:NO];
    [s_downloadWindow setTitle:@"Map Downloader"];
    [s_downloadWindow center];
    [s_downloadWindow setLevel:NSFloatingWindowLevel];
    [s_downloadWindow setReleasedWhenClosed:NO];

    NSView* content = [s_downloadWindow contentView];

    NSTextField* titleLabel = [NSTextField labelWithString:@"Download Maps"];
    titleLabel.font = [NSFont boldSystemFontOfSize:14];
    titleLabel.frame = NSMakeRect(20, winH - 40, winW - 40, 25);
    [content addSubview:titleLabel];

    NSTextField* infoLabel = [NSTextField wrappingLabelWithString:
        @"Download ranked maps from gentool.net or custom/fixed maps "
        @"from TheSuperHackers. Maps will be placed in your user Maps folder."];
    infoLabel.font = [NSFont systemFontOfSize:12];
    infoLabel.frame = NSMakeRect(20, winH - 85, winW - 40, 40);
    [content addSubview:infoLabel];

    s_progressBar = [[NSProgressIndicator alloc] initWithFrame:
        NSMakeRect(20, winH - 115, winW - 40, 20)];
    [s_progressBar setStyle:NSProgressIndicatorStyleBar];
    [s_progressBar setMinValue:0.0];
    [s_progressBar setMaxValue:100.0];
    [s_progressBar setDoubleValue:0.0];
    [s_progressBar setIndeterminate:NO];
    [content addSubview:s_progressBar];

    s_statusLabel = [NSTextField labelWithString:@"Ready"];
    s_statusLabel.font = [NSFont systemFontOfSize:11];
    s_statusLabel.frame = NSMakeRect(20, winH - 140, winW - 40, 18);
    [content addSubview:s_statusLabel];

    bool hasRanked = GenMapDownloader_HasRankedMaps();

    s_rankedButton = [[NSButton alloc] initWithFrame:
        NSMakeRect(20, 12, 140, 32)];
    [s_rankedButton setTitle:hasRanked ? @"Reinstall Ranked" : @"Download Ranked"];
    [s_rankedButton setBezelStyle:NSBezelStyleRounded];
    [s_rankedButton setTarget:[GenMapDownloaderActionHandler class]];
    [s_rankedButton setAction:@selector(onRankedAction:)];
    [content addSubview:s_rankedButton];

    s_customButton = [[NSButton alloc] initWithFrame:
        NSMakeRect(168, 12, 150, 32)];
    [s_customButton setTitle:@"Download Custom"];
    [s_customButton setBezelStyle:NSBezelStyleRounded];
    [s_customButton setTarget:[GenMapDownloaderActionHandler class]];
    [s_customButton setAction:@selector(onCustomAction:)];
    [content addSubview:s_customButton];

    s_closeButton = [[NSButton alloc] initWithFrame:
        NSMakeRect(winW - 100, 12, 80, 32)];
    [s_closeButton setTitle:@"Close"];
    [s_closeButton setBezelStyle:NSBezelStyleRounded];
    [s_closeButton setTarget:[GenMapDownloaderActionHandler class]];
    [s_closeButton setAction:@selector(onCloseAction:)];
    [content addSubview:s_closeButton];

    if (hasRanked) {
        [s_statusLabel setStringValue:@"Ranked maps already installed"];
        [s_progressBar setDoubleValue:100.0];
    }

    s_state = GenMapDownloadState::Idle;

    [s_downloadWindow makeKeyAndOrderFront:nil];
    [s_downloadWindow orderFrontRegardless];
    DLOG_NETWORK("GenMapDownloader: window shown");
}

GenMapDownloadState GenMapDownloader_GetState() {
    return s_state;
}

int GenMapDownloader_GetProgress() {
    return s_progress;
}

#endif // __APPLE__
