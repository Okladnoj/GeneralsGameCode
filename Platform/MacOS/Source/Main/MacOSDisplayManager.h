/**
 * MacOSDisplayManager — Centralized display resolution manager for macOS
 *
 * This singleton manages the synchronization of all resolution-dependent
 * components when the game resolution changes:
 *   1. NSWindow contentRect (macOS window size in points)
 *   2. CAMetalLayer drawableSize (Metal rendering surface)
 *   3. MetalDevice8 screen dimensions (DX8 wrapper)
 *   4. Depth texture (recreated to match drawable)
 *   5. Viewport (rendering viewport dimensions)
 *   6. TheGlobalData resolution (game engine)
 *
 * All resolution changes should flow through setResolution() to guarantee
 * atomic, consistent updates across the entire pipeline.
 */
#pragma once

#ifdef __APPLE__

#include <vector>

class MacOSDisplayManager {
public:
    static MacOSDisplayManager& instance();

    /// Initialize with the game window. Must be called once after window creation.
    void init(void* nsWindowHandle);

    /// Change resolution — updates ALL components atomically.
    /// Returns true on success, false if the resolution could not be applied.
    bool setResolution(int width, int height);

    /// Sync engine/Metal to the current window content size.
    /// Called when the user manually resizes the window (from windowDidResize:).
    void syncToWindowSize();

    /// Get current game resolution (the resolution we render at)
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    /// Get screen info (logical points of the main display)
    int getScreenWidth() const;
    int getScreenHeight() const;
    float getBackingScaleFactor() const;

    /// Display mode for enumeration
    struct DisplayMode {
        int w;
        int h;
        int hz;
    };

    /// Get available display modes (deduplicated, sorted, ≥ 800×600).
    /// Includes both system-detected modes and standard gaming resolutions
    /// that fit within the screen.
    const std::vector<DisplayMode>& getAvailableModes();

    /// Get the current desktop display mode (in points — consistent with enumeration)
    DisplayMode getCurrentDesktopMode() const;

    /// Whether init() has been called
    bool isInitialized() const { return m_initialized; }

    /// Get the window handle
    void* getWindowHandle() const { return m_window; }

private:
    MacOSDisplayManager();
    ~MacOSDisplayManager() = default;

    // Non-copyable
    MacOSDisplayManager(const MacOSDisplayManager&) = delete;
    MacOSDisplayManager& operator=(const MacOSDisplayManager&) = delete;

    void* m_window = nullptr;  // NSWindow* (bridged)
    int m_width = 800;
    int m_height = 600;
    bool m_initialized = false;
    bool m_isSettingResolution = false;  // guard against re-entrancy from windowDidResize

    std::vector<DisplayMode> m_modes;
    bool m_modesEnumerated = false;

    void enumerateModes();
    void resizeWindow(int w, int h);
    void updateMetalLayer(int w, int h);
    void updateMetalDevice(int w, int h);
    void updateGameResolution(int w, int h);
};

#endif // __APPLE__
