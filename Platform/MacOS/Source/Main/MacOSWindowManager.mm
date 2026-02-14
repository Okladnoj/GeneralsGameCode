#import "../../Include/MacOSWindowManager.h"
#import "always.h"
#import <Cocoa/Cocoa.h>

@interface MacOSWindowDelegate
    : NSObject <NSWindowDelegate, NSApplicationDelegate>
@end

@implementation MacOSWindowDelegate
- (void)windowWillClose:(NSNotification *)notification {
  [NSApp terminate:nil];
}
@end

static BOOL g_ShouldQuit = NO;

#import "Common/AsciiString.h"
#import "Common/UnicodeString.h"
#import "Common/version.h"

extern int GameMain();

int MacOS_Main(int argc, char *argv[]) {
  @autoreleasepool {
    printf("MacOS_Main: Initializing TheVersion...\n");
    fflush(stdout);
    TheVersion = new Version();
    printf("MacOS_Main: TheVersion = %p\n", (void *)TheVersion);
    fflush(stdout);
    printf("MacOS_Main: Initializing NSApp...\n");
    fflush(stdout);
    [NSApplication sharedApplication];
    printf("MacOS_Main: Setting Activation Policy...\n");
    fflush(stdout);
    [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
    [NSApp finishLaunching];

    printf("MacOS_Main: Entering GameMain()...\n");
    fflush(stdout);

    // Force bring to front
    [NSApp activateIgnoringOtherApps:YES];

    return GameMain();
  }
}

void *MacOS_CreateWindow(int width, int height, const char *title) {
  [NSApplication sharedApplication];
  [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

  NSRect frame = NSMakeRect(0, 0, width, height);
  NSWindow *window = [[NSWindow alloc]
      initWithContentRect:frame
                styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable |
                          NSWindowStyleMaskMiniaturizable
                  backing:NSBackingStoreBuffered
                    defer:NO];

  [window setTitle:[NSString stringWithUTF8String:title]];
  [window makeKeyAndOrderFront:nil];
  [window center];

  MacOSWindowDelegate *delegate = [[MacOSWindowDelegate alloc] init];
  [window setDelegate:delegate];

  [NSApp activateIgnoringOtherApps:YES];

  return (__bridge void *)window;
}

void MacOS_PumpEvents() {
  @autoreleasepool {
    NSEvent *event;
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:[NSDate distantPast]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES])) {
      fprintf(stderr, "Event: %lu\n", (unsigned long)[event type]);
      [NSApp sendEvent:event];
    }
  }
}

bool MacOS_ShouldQuit() { return g_ShouldQuit; }
