#pragma once

#ifndef _WIN32
#include <stdint.h>

#define BINKPRELOADALL 0x00000001
#define BINKSURFACE32 0
#define BINKSURFACE24 1
#define BINKSURFACE565 3
#define BINKSURFACE555 4

typedef struct BINK {
  uint32_t Width;
  uint32_t Height;
  uint32_t Frames;
  uint32_t FrameNum;
  uint32_t LastFrameNum;
  uint32_t FrameRate;
  uint32_t FrameRateDiv;
  uint32_t ReadError;
  uint32_t OpenFlags;
} BINK, *HBINK;

#ifdef __cplusplus
extern "C" {
#endif

extern HBINK BinkOpen(const char *name, uint32_t flags);
extern void BinkClose(HBINK bink);
extern int32_t BinkDoFrame(HBINK bink);
extern void BinkNextFrame(HBINK bink);
extern int32_t BinkWait(HBINK bink);
extern void BinkGoto(HBINK bink, uint32_t frame, uint32_t flags);
extern void BinkCopyToBuffer(HBINK bink, void *dest, int32_t destpitch,
                             int32_t destheight, int32_t destx, int32_t desty,
                             uint32_t flags);
extern void BinkSetVolume(HBINK bink, uint32_t track, int32_t volume);
extern void BinkSetSoundTrack(uint32_t track, const int32_t *bins);
extern int32_t BinkSoundUseDirectSound(void *driver);

#ifdef __cplusplus
}
#endif

#endif
