#pragma once
#include "imagehlp_compat.h"
#include "win_compat.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _IMAGEHLP_MODULE64 {
  DWORD SizeOfStruct;
  DWORD64 BaseOfImage;
  DWORD ImageSize;
  DWORD TimeDateStamp;
  DWORD CheckSum;
  DWORD NumSyms;
  DWORD SymType;
  CHAR ModuleName[32];
  CHAR ImageName[256];
  CHAR LoadedImageName[256];
} IMAGEHLP_MODULE64, *PIMAGEHLP_MODULE64;

#ifdef __cplusplus
}
#endif
