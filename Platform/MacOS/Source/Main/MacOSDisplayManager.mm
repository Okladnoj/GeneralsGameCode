/**
 * MacOSDisplayManager.mm — Implementation
 *
 * Centralizes all resolution management for the macOS port.
 * When setResolution() is called, it atomically updates:
 *   1. NSWindow frame/contentRect
 *   2. CAMetalLayer drawableSize
 *   3. MetalDevice8 screen dimensions + depth texture + viewport
 *   4. TheGlobalData resolution
 */
#ifdef __APPLE__

#import <AppKit/AppKit.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#import <CoreGraphics/CoreGraphics.h>

#include "MacOSDisplayManager.h"
#include <algorithm>
#include <cstdio>
#include <set>

// Forward declarations to avoid circular includes
// MetalDevice8 is accessed via extern global — see MetalDevice8.mm
class MetalDevice8;

// Extern: The global MetalDevice8 instance pointer.
// This is the actual IDirect3DDevice8* created by MetalInterface8::CreateDevice().
// We access it to call updateScreenSize() when resolution changes.
extern MetalDevice8* g_theMetalDevice;

// Extern: GlobalData resolution — updated when the resolution changes.
// These are defined in GameEngine/Source/Common/GlobalData.cpp
struct GlobalData;
extern GlobalData* TheWritableGlobalData;

// Extern C bridge function defined in MetalDevice8.mm
// Updates MetalDevice8 screen dimensions, depth texture, and viewport.
extern "C" void MacOS_UpdateMetalDeviceScreenSize(int width, int height);

// Extern C bridge function defined in dx8wrapper.cpp
// Updates DX8Wrapper resolution + Render2DClass screen coordinates.
extern "C" void MacOS_UpdateDX8Resolution(int w, int h);

// We access m_xResolution and m_yResolution via the public header
#include "Common/GlobalData.h"


// ─────────────────────────────────────────────────────
//  Singleton
// ─────────────────────────────────────────────────────

MacOSDisplayManager::MacOSDisplayManager() = default;

MacOSDisplayManager& MacOSDisplayManager::instance() {
    static MacOSDisplayManager s_instance;
    return s_instance;
}

// ─────────────────────────────────────────────────────
//  init
// ─────────────────────────────────────────────────────

void MacOSDisplayManager::init(void* nsWindowHandle) {
    m_window = nsWindowHandle;

    if (m_window) {
        NSWindow* win = (__bridge NSWindow*)m_window;
        CGSize viewSize = win.contentView.bounds.size;
        m_width = (int)viewSize.width;
        m_height = (int)viewSize.height;
        fprintf(stderr, "[MacOSDisplayManager] Initialized with window %p, size %dx%d\n",
                m_window, m_width, m_height);
    } else {
        fprintf(stderr, "[MacOSDisplayManager] WARNING: Initialized without window handle\n");
    }

    m_initialized = true;
}

// ─────────────────────────────────────────────────────
//  setResolution — the critical atomic update function
// ─────────────────────────────────────────────────────

bool MacOSDisplayManager::setResolution(int width, int height) {
    if (width <= 0 || height <= 0) {
        fprintf(stderr, "[MacOSDisplayManager] ERROR: Invalid resolution %dx%d\n", width, height);
        return false;
    }

    fprintf(stderr, "[MacOSDisplayManager] setResolution: %dx%d -> %dx%d\n",
            m_width, m_height, width, height);

    // Guard: prevent windowDidResize: → syncToWindowSize() from firing
    // while we're in the middle of setting up the new resolution.
    m_isSettingResolution = true;

    // 1. Resize the NSWindow (content area = width x height)
    resizeWindow(width, height);

    // 2. Update CAMetalLayer drawable size
    updateMetalLayer(width, height);

    // 3. Update MetalDevice8 (screen dimensions + depth texture + viewport)
    updateMetalDevice(width, height);

    // 4. Update game engine resolution (TheGlobalData)
    updateGameResolution(width, height);

    // 5. Store resolution — this is the authoritative value
    m_width = width;
    m_height = height;

    m_isSettingResolution = false;

    fprintf(stderr, "[MacOSDisplayManager] Resolution set to %dx%d successfully\n", width, height);
    return true;
}

// Extern C bridge function defined in W3DDisplay.cpp  
// Mirrors OptionsMenu Accept: setDisplayMode + GlobalData + recreateWindowLayouts etc.
extern "C" void MacOS_ApplyDisplayResolution(int w, int h);

void MacOSDisplayManager::syncToWindowSize() {
    // Don't respond to resize events caused by our own setResolution()
    if (m_isSettingResolution) return;
    if (!m_window) return;

    NSWindow* win = (__bridge NSWindow*)m_window;
    CGSize viewSize = win.contentView.bounds.size;
    int newW = (int)viewSize.width;
    int newH = (int)viewSize.height;

    if (newW == m_width && newH == m_height) return;  // no change

    fprintf(stderr, "[MacOSDisplayManager] syncToWindowSize: %dx%d -> %dx%d (user resize)\n",
            m_width, m_height, newW, newH);

    // Update macOS-specific Metal components
    updateMetalLayer(newW, newH);
    updateMetalDevice(newW, newH);

    // Apply the resolution change through the same path as Options menu Accept.
    // This updates DX8Wrapper, Render2DClass, Display, TacticalView, Mouse,
    // Shell layouts, InGameUI, and everything else.
    MacOS_ApplyDisplayResolution(newW, newH);

    m_width = newW;
    m_height = newH;
}

// ─────────────────────────────────────────────────────
//  resizeWindow — update NSWindow contentRect
// ─────────────────────────────────────────────────────

void MacOSDisplayManager::resizeWindow(int w, int h) {
    if (!m_window) return;

    NSWindow* win = (__bridge NSWindow*)m_window;
    NSScreen* screen = [win screen] ?: [NSScreen mainScreen];

    // frameRectForContentRect: calculates the outer frame needed to have
    // a content area of exactly w×h (automatically accounts for title bar height).
    NSRect contentRect = NSMakeRect(0, 0, w, h);
    NSRect newFrame = [win frameRectForContentRect:contentRect];

    // Center horizontally, align top of window to top of visible frame
    // so the title bar is always visible (just below macOS menu bar).
    NSRect visibleFrame = screen.visibleFrame;
    newFrame.origin.x = (visibleFrame.size.width - newFrame.size.width) / 2 + visibleFrame.origin.x;
    newFrame.origin.y = NSMaxY(visibleFrame) - newFrame.size.height;

    [win setFrame:newFrame display:YES animate:NO];

    fprintf(stderr, "[MacOSDisplayManager] Window resized: requested content %dx%d, "
            "actual content %gx%g, titlebar height=%g\n",
            w, h, win.contentView.bounds.size.width, win.contentView.bounds.size.height,
            newFrame.size.height - h);
}

// ─────────────────────────────────────────────────────
//  updateMetalLayer — update CAMetalLayer drawableSize
// ─────────────────────────────────────────────────────

void MacOSDisplayManager::updateMetalLayer(int w, int h) {
    if (!m_window) return;

    NSWindow* win = (__bridge NSWindow*)m_window;
    NSView* contentView = win.contentView;

    if (contentView.layer && [contentView.layer isKindOfClass:[CAMetalLayer class]]) {
        CAMetalLayer* layer = (CAMetalLayer*)contentView.layer;

        // Keep contentsScale at 1.0 — game renders pixel-perfect,
        // macOS handles Retina upscaling
        layer.contentsScale = 1.0;
        layer.drawableSize = CGSizeMake(w, h);

        fprintf(stderr, "[MacOSDisplayManager] CAMetalLayer drawableSize set to %dx%d\n", w, h);
    } else {
        fprintf(stderr, "[MacOSDisplayManager] WARNING: No CAMetalLayer found on contentView\n");
    }
}

// ─────────────────────────────────────────────────────
//  updateMetalDevice — MetalDevice8 screen size, depth texture, viewport
// ─────────────────────────────────────────────────────

void MacOSDisplayManager::updateMetalDevice(int w, int h) {
    if (!g_theMetalDevice) {
        fprintf(stderr, "[MacOSDisplayManager] WARNING: g_theMetalDevice is null, skipping Metal update\n");
        return;
    }

    MacOS_UpdateMetalDeviceScreenSize(w, h);
}

// ─────────────────────────────────────────────────────
//  updateGameResolution — TheGlobalData + TheDisplay
// ─────────────────────────────────────────────────────

void MacOSDisplayManager::updateGameResolution(int w, int h) {
    // Update game engine global resolution
    if (TheWritableGlobalData) {
        TheWritableGlobalData->m_xResolution = w;
        TheWritableGlobalData->m_yResolution = h;
        fprintf(stderr, "[MacOSDisplayManager] GlobalData resolution set to %dx%d\n", w, h);
    }
}

// ─────────────────────────────────────────────────────
//  Screen info queries
// ─────────────────────────────────────────────────────

int MacOSDisplayManager::getScreenWidth() const {
    NSScreen* screen = [NSScreen mainScreen];
    return screen ? (int)screen.frame.size.width : 0;
}

int MacOSDisplayManager::getScreenHeight() const {
    NSScreen* screen = [NSScreen mainScreen];
    return screen ? (int)screen.frame.size.height : 0;
}

float MacOSDisplayManager::getBackingScaleFactor() const {
    NSScreen* screen = [NSScreen mainScreen];
    return screen ? (float)screen.backingScaleFactor : 1.0f;
}

MacOSDisplayManager::DisplayMode MacOSDisplayManager::getCurrentDesktopMode() const {
    // Return in POINTS (consistent with mode enumeration)
    NSScreen* screen = [NSScreen mainScreen];
    if (screen) {
        return { (int)screen.frame.size.width, (int)screen.frame.size.height, 60 };
    }
    return { 800, 600, 60 };
}

// ─────────────────────────────────────────────────────
//  Display mode enumeration
// ─────────────────────────────────────────────────────

void MacOSDisplayManager::enumerateModes() {
    if (m_modesEnumerated) return;
    m_modesEnumerated = true;
    m_modes.clear();

    // Use a set of (w, h) pairs for deduplication
    std::set<std::pair<int,int>> seen;

    // 1. Enumerate system display modes via CGDisplayCopyAllDisplayModes
    CGDirectDisplayID display = CGMainDisplayID();
    CFArrayRef allModes = CGDisplayCopyAllDisplayModes(display, nullptr);
    if (allModes) {
        CFIndex count = CFArrayGetCount(allModes);
        for (CFIndex i = 0; i < count; i++) {
            CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(allModes, i);
            int w = (int)CGDisplayModeGetWidth(mode);   // points
            int h = (int)CGDisplayModeGetHeight(mode);   // points
            double hz = CGDisplayModeGetRefreshRate(mode);
            if (hz < 1.0) hz = 60.0;

            if (w < 800 || h < 600) continue;

            auto key = std::make_pair(w, h);
            if (seen.insert(key).second) {
                m_modes.push_back({ w, h, (int)hz });
            }
        }
        CFRelease(allModes);
    }

    // 2. Add standard gaming resolutions that fit within the screen
    NSScreen* screen = [NSScreen mainScreen];
    int screenW = screen ? (int)screen.frame.size.width : 1920;
    int screenH = screen ? (int)screen.frame.size.height : 1080;

    static const DisplayMode standardModes[] = {
        { 800,  600,  60 },
        { 1024, 768,  60 },
        { 1152, 864,  60 },
        { 1280, 720,  60 },
        { 1280, 800,  60 },
        { 1280, 960,  60 },
        { 1280, 1024, 60 },
        { 1366, 768,  60 },
        { 1440, 900,  60 },
        { 1600, 900,  60 },
        { 1600, 1200, 60 },
        { 1680, 1050, 60 },
        { 1920, 1080, 60 },
        { 1920, 1200, 60 },
        { 2560, 1440, 60 },
        { 2560, 1600, 60 },
        { 3840, 2160, 60 },
    };

    for (const auto& mode : standardModes) {
        if (mode.w <= screenW && mode.h <= screenH) {
            auto key = std::make_pair(mode.w, mode.h);
            if (seen.insert(key).second) {
                m_modes.push_back(mode);
            }
        }
    }

    // 3. Sort by area (ascending), then by width
    std::sort(m_modes.begin(), m_modes.end(), [](const DisplayMode& a, const DisplayMode& b) {
        int areaA = a.w * a.h;
        int areaB = b.w * b.h;
        if (areaA != areaB) return areaA < areaB;
        return a.w < b.w;
    });

    // 4. Fallback: ensure at least 800x600
    if (m_modes.empty()) {
        m_modes.push_back({ 800, 600, 60 });
    }

    fprintf(stderr, "[MacOSDisplayManager] Enumerated %zu display modes:\n", m_modes.size());
    for (const auto& mode : m_modes) {
        fprintf(stderr, "  %d x %d @ %d Hz\n", mode.w, mode.h, mode.hz);
    }
}

const std::vector<MacOSDisplayManager::DisplayMode>& MacOSDisplayManager::getAvailableModes() {
    enumerateModes();
    return m_modes;
}

// ─────────────────────────────────────────────────────
//  Extern C bridge — called from DX8Wrapper (dx8wrapper.cpp)
//  when Set_Device_Resolution or Set_Render_Device need to
//  resize the window on macOS.
// ─────────────────────────────────────────────────────

extern "C" void MacOS_SetDisplayResolution(int w, int h) {
    fprintf(stderr, "[MacOSDisplayManager] MacOS_SetDisplayResolution called: %dx%d\n", w, h);
    MacOSDisplayManager::instance().setResolution(w, h);
}

#endif // __APPLE__
