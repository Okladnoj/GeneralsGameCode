#import "../../Include/MacOSWindowManager.h"
#import "always.h"
#import <Cocoa/Cocoa.h>

@interface MacOSWindowDelegate
    : NSObject <NSWindowDelegate, NSApplicationDelegate>
@end

static BOOL g_ShouldQuit = NO;

@implementation MacOSWindowDelegate
- (void)windowWillClose:(NSNotification *)notification {
  g_ShouldQuit = YES;
  [NSApp terminate:nil];
}
@end
static id g_WindowDelegate = nil;
static id g_AppDelegate = nil;

#import "Common/AsciiString.h"
#import "Common/GlobalData.h"
#import "Common/UnicodeString.h"
#import "Common/version.h"

extern int GameMain();
extern "C" void MacOS_InitRenderer(void *windowHandle);

#include "Common/CommandLine.h"

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

    printf("MacOS_Main: Entering GameMain()...\n");
    fflush(stdout);

    // Give Cocoa time to settle
    [NSThread sleepForTimeInterval:0.1];

    // Force bring to front
    [NSApp activateIgnoringOtherApps:YES];

    // Finish launching to ensure NSApp is ready
    [NSApp finishLaunching];

    // Parse command line and init global data
    CommandLine::parseCommandLineForStartup();

    // Create window
    int w = 1024;
    int h = 768;
    if (TheGlobalData) {
      w = TheGlobalData->m_xResolution;
      h = TheGlobalData->m_yResolution;
    }
    void *window =
        MacOS_CreateWindow(w, h, "Command & Conquer Generals (macOS)");

    // Pump events for a bit to ensure window shows up
    for (int i = 0; i < 30; ++i) {
      MacOS_PumpEvents();
      [NSThread sleepForTimeInterval:0.01];
    }

    // Initialize renderer
    MacOS_InitRenderer(window);

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
  printf("MacOS_CreateWindow: Window created at %p, title: %s, thread=%p\n",
         (__bridge void *)window, title,
         (__bridge void *)[NSThread currentThread]);
  [window setBackgroundColor:[NSColor redColor]];
  fflush(stdout);
  [window makeKeyAndOrderFront:nil];
  [window orderFrontRegardless];
  [window center];

  g_WindowDelegate = [[MacOSWindowDelegate alloc] init];
  [window setDelegate:g_WindowDelegate];

  [NSApp activateIgnoringOtherApps:YES];
  [window makeKeyWindow];
  [window makeMainWindow];

  printf("MacOS_CreateWindow: Window visibility: %s\n",
         [window isVisible] ? "YES" : "NO");
  fflush(stdout);

  return (__bridge void *)window;
}

#import "StdKeyboard.h"
#import "StdMouse.h"

void MacOS_PumpEvents() {
  @autoreleasepool {
    NSEvent *event;
    while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                       untilDate:[NSDate distantPast]
                                          inMode:NSDefaultRunLoopMode
                                         dequeue:YES])) {

      unsigned int timestamp = (unsigned int)([event timestamp] * 1000.0);

      switch ([event type]) {
      case NSEventTypeKeyDown:
        if (TheKeyboard) {
          unsigned char keyCode = [event keyCode];
          ((StdKeyboard *)TheKeyboard)->addEvent(keyCode, true, timestamp);
        }
        break;
      case NSEventTypeKeyUp:
        if (TheKeyboard) {
          unsigned char keyCode = [event keyCode];
          ((StdKeyboard *)TheKeyboard)->addEvent(keyCode, false, timestamp);
        }
        break;

      case NSEventTypeMouseMoved:
      case NSEventTypeLeftMouseDragged:
      case NSEventTypeRightMouseDragged:
      case NSEventTypeOtherMouseDragged: {
        if (TheMouse) {
          NSPoint loc = [event locationInWindow];
          // Note: Window might be nil if event is not directed to a window
          NSWindow *win = [event window];
          if (win) {
            NSRect content = [win contentRectForFrameRect:[win frame]];
            int x = (int)loc.x;
            int y = (int)(content.size.height - loc.y);
            ((StdMouse *)TheMouse)
                ->addEvent(MACOS_MOUSE_MOVE, x, y, 0, 0, timestamp);
          }
        }
        break;
      }

      case NSEventTypeLeftMouseDown:
        if (TheMouse) {
          NSPoint loc = [event locationInWindow];
          NSWindow *win = [event window];
          if (win) {
            NSRect content = [win contentRectForFrameRect:[win frame]];
            int x = (int)loc.x;
            int y = (int)(content.size.height - loc.y);
            ((StdMouse *)TheMouse)
                ->addEvent(MACOS_MOUSE_LBUTTON_DOWN, x, y, 0, 0, timestamp);
          }
        }
        break;
      case NSEventTypeLeftMouseUp:
        if (TheMouse) {
          NSPoint loc = [event locationInWindow];
          NSWindow *win = [event window];
          if (win) {
            NSRect content = [win contentRectForFrameRect:[win frame]];
            int x = (int)loc.x;
            int y = (int)(content.size.height - loc.y);
            ((StdMouse *)TheMouse)
                ->addEvent(MACOS_MOUSE_LBUTTON_UP, x, y, 0, 0, timestamp);
          }
        }
        break;

      case NSEventTypeRightMouseDown:
        if (TheMouse) {
          NSPoint loc = [event locationInWindow];
          NSWindow *win = [event window];
          if (win) {
            NSRect content = [win contentRectForFrameRect:[win frame]];
            int x = (int)loc.x;
            int y = (int)(content.size.height - loc.y);
            ((StdMouse *)TheMouse)
                ->addEvent(MACOS_MOUSE_RBUTTON_DOWN, x, y, 0, 0, timestamp);
          }
        }
        break;
      case NSEventTypeRightMouseUp:
        if (TheMouse) {
          NSPoint loc = [event locationInWindow];
          NSWindow *win = [event window];
          if (win) {
            NSRect content = [win contentRectForFrameRect:[win frame]];
            int x = (int)loc.x;
            int y = (int)(content.size.height - loc.y);
            ((StdMouse *)TheMouse)
                ->addEvent(MACOS_MOUSE_RBUTTON_UP, x, y, 0, 0, timestamp);
          }
        }
        break;

      case NSEventTypeScrollWheel:
        if (TheMouse) {
          int delta = (int)([event scrollingDeltaY]);
          ((StdMouse *)TheMouse)
              ->addEvent(MACOS_MOUSE_WHEEL, 0, 0, 0, delta, timestamp);
        }
        break;

      default:
        break;
      }

      [NSApp sendEvent:event];
    }
  }
}

bool MacOS_ShouldQuit() { return g_ShouldQuit; }
