#pragma once
#include "win_compat.h"

typedef struct IDirectSound *LPDIRECTSOUND;
typedef struct IDirectSoundBuffer *LPDIRECTSOUNDBUFFER;

struct IDirectSound : public IUnknown {
  virtual HRESULT STDMETHODCALLTYPE GetSpeakerConfig(
      LPDWORD pdwSpeakerConfig) = 0;
  virtual HRESULT STDMETHODCALLTYPE SetSpeakerConfig(DWORD dwSpeakerConfig) = 0;
};

#define DSSPEAKER_CONFIG(dw) ((BYTE)(dw))
#define DSSPEAKER_DIRECTOUT 0x00000000
#define DSSPEAKER_HEADPHONE 0x00000001
#define DSSPEAKER_MONO 0x00000002
#define DSSPEAKER_QUAD 0x00000003
#define DSSPEAKER_STEREO 0x00000004
#define DSSPEAKER_SURROUND 0x00000005
#define DSSPEAKER_5POINT1 0x00000006
#define DSSPEAKER_7POINT1 0x00000007
#define DSSPEAKER_HEADSET 0x00000001 // Synonym for headphone in many contexts
