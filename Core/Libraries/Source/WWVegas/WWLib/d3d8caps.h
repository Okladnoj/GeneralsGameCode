#pragma once
#if defined(_WIN32) || defined(BUILD_WITH_D3D8)
#include <d3d8.h>
#else
#include "d3d8_stub.h"
#endif

typedef struct _D3DCAPS8 D3DCAPS8;
