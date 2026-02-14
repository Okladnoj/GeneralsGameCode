#ifndef __MACOS_GAMEENGINE_HOOKS_H__
#define __MACOS_GAMEENGINE_HOOKS_H__

/**
 * Hook to log macOS-specific build and version information during engine init.
 */
void MacOS_LogBuildInfo();

/**
 * Hook to initialize performance timers for macOS.
 */
void MacOS_InitPerformanceTimers();

/**
 * Hook to log performance statistics after a subsystem has been initialized.
 * @param subsystemName The name of the subsystem that was just initialized.
 */
void MacOS_LogSubsystemInitTime(const char *subsystemName);

#endif // __MACOS_GAMEENGINE_HOOKS_H__
