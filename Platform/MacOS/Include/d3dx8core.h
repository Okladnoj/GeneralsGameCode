#pragma once
#include "d3d8.h"

#ifdef __cplusplus
extern "C" {
#endif

inline HRESULT D3DXAssembleShader(const void *pSrcData, UINT SrcDataLen,
                                  DWORD Flags, LPD3DXBUFFER *ppConstants,
                                  LPD3DXBUFFER *ppCompiledShader,
                                  LPD3DXBUFFER *ppCompilationErrors) {
  return 0; // D3D_OK
}

inline HRESULT D3DXGetErrorStringA(HRESULT hr, char *pBuffer, UINT BufferSize) {
  if (pBuffer && BufferSize > 0) {
    pBuffer[0] = 0;
  }
  return 0; // D3D_OK
}

inline UINT D3DXGetFVFVertexSize(DWORD FVF) {
  UINT size = 0;
  if (FVF & D3DFVF_XYZRHW)
    size += 16;
  else if (FVF & D3DFVF_XYZ)
    size += 12;

  if (FVF & D3DFVF_XYZB1)
    size += 4;
  if (FVF & D3DFVF_XYZB2)
    size += 8;
  if (FVF & D3DFVF_XYZB3)
    size += 12;
  if (FVF & D3DFVF_XYZB4)
    size += 16;
  if (FVF & D3DFVF_XYZB5)
    size += 20;

  if (FVF & D3DFVF_NORMAL)
    size += 12;
  if (FVF & D3DFVF_PSIZE)
    size += 4;
  if (FVF & D3DFVF_DIFFUSE)
    size += 4;
  if (FVF & D3DFVF_SPECULAR)
    size += 4;

  UINT nTex = (FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
  for (UINT i = 0; i < nTex; i++) {
    // Default size is 2 floats = 8 bytes
    size += 8;
  }
  return size;
}

#ifdef __cplusplus
}
#endif
