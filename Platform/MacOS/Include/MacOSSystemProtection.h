#ifndef MACOS_SYSTEM_PROTECTION_H
#define MACOS_SYSTEM_PROTECTION_H

#if defined(__APPLE__)
#include <stdint.h>

// 1. Define Engine-side renames to avoid clash with Apple types
// These will be active for all engine code.
#define Byte EngineByte
#define Rect EngineRect
#define Point EnginePoint
#define RGBColor EngineRGBColor
#define FileInfo EngineFileInfo
#define ChunkHeader EngineChunkHeader
#define WideChar EngineWideChar
#define GetCurrentThread EngineGetCurrentThread
#define GetCurrentProcess EngineGetCurrentProcess
#define LoadResource EngineLoadResource
#define Handle EngineHandle

typedef char EngineByte;
typedef char16_t EngineWideChar;
typedef int32_t EngineBOOL;
typedef int32_t EngineHRESULT;
typedef void *EngineHandle;

// 2. Temporarily undefine them to include system headers safely
#undef Byte
#undef Rect
#undef Point
#undef RGBColor
#undef FileInfo
#undef ChunkHeader
#undef WideChar
#undef GetCurrentThread
#undef GetCurrentProcess
#undef LoadResource
#undef Handle

// Protect against 'interface' macro clash
#ifdef interface
#undef interface
#endif

// Include system headers while names are restored
#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>
#else
#include <CoreFoundation/CoreFoundation.h>
#endif
#include <objc/objc.h>

// 3. Re-enable Engine-side renames
#define Byte EngineByte
#define Rect EngineRect
#define Point EnginePoint
#define RGBColor EngineRGBColor
#define FileInfo EngineFileInfo
#define ChunkHeader EngineChunkHeader
#define WideChar EngineWideChar
#define GetCurrentThread EngineGetCurrentThread
#define GetCurrentProcess EngineGetCurrentProcess
#define LoadResource EngineLoadResource
#define Handle EngineHandle

#ifndef interface
#ifndef __OBJC__
#define interface struct
#endif
#endif

// Some specific fixes for Apple's BOOL if needed
// But usually letting it be is best.

#undef PI

#endif // __APPLE__
#endif // MACOS_SYSTEM_PROTECTION_H
