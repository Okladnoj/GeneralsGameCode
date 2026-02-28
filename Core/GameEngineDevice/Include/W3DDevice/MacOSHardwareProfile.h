/*
**  Command & Conquer Generals Zero Hour(tm)
**  Copyright 2025 Electronic Arts Inc.
**
**  macOS Hardware Profile — centralized fake hardware identity
**  for the Metal port. On macOS, CPUDetectClass and D3D adapter
**  identifier return zeros. This header provides inline functions
**  that report a high-end 2003-era hardware profile, enabling
**  VeryHigh quality settings and pixel shader code paths.
**
**  Usage: #include this header and call the functions wherever
**  hardware detection is needed, guarded by #ifdef __APPLE__.
*/

#ifndef MACOS_HARDWARE_PROFILE_H
#define MACOS_HARDWARE_PROFILE_H

#ifdef __APPLE__

#include "Common/GameLOD.h"  // ChipsetType, CpuType, StaticGameLODLevel, MemValueType

namespace MacOSHardware {

/// Chipset to report — PS 2.0 enables all terrain/water shader paths
inline ChipsetType GetChipsetType() { return DC_GENERIC_PIXEL_SHADER_2_0; }

/// CPU type for LOD preset matching
inline CpuType GetCpuType() { return P4; }

/// CPU frequency in MHz — high enough to pass all LOD presets
inline int GetCpuFreqMHz() { return 3000; }

/// Total RAM in bytes — 2 GB, enough to pass memory checks
inline MemValueType GetTotalRAM() { return (MemValueType)2048 * 1024 * 1024; }

/// GPU performance index — maximum quality
inline StaticGameLODLevel GetGPUPerformance() { return STATIC_GAME_LOD_VERY_HIGH; }

/// Benchmark indices — high values to pass all thresholds
inline float GetIntBenchIndex()   { return 10.0f; }
inline float GetFloatBenchIndex() { return 10.0f; }
inline float GetMemBenchIndex()   { return 10.0f; }

}  // namespace MacOSHardware

#endif  // __APPLE__
#endif  // MACOS_HARDWARE_PROFILE_H
