#pragma once
#include "d3d8.h"

#ifdef __cplusplus
extern "C" {
#endif

HRESULT WINAPI D3DXAssembleShader(const void *pSrcData, UINT SrcDataLen,
                                  DWORD Flags, LPD3DXBUFFER *ppConstants,
                                  LPD3DXBUFFER *ppCompiledShader,
                                  LPD3DXBUFFER *ppCompilationErrors);

HRESULT WINAPI D3DXGetErrorStringA(HRESULT hr, char *pBuffer, UINT BufferSize);

UINT WINAPI D3DXGetFVFVertexSize(DWORD FVF);

#ifdef __cplusplus
}
#endif
