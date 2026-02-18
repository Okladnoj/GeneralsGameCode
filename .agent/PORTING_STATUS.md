**Goal:** Metal Backend Stage 7+ — Rendering correctness.
**Build:** ✅ **SUCCEEDED** (941 objects, clean build OK).
**Last updated:** 2026-02-18 13:50

---

## 🚀 Status Quo
- [x] **Subsystems:** Memory, Filesystem (.big), INI, Input, Window.
- [x] **Audio Engine:** ✅ `MacOSAudioManager` via `AVAudioEngine`. `.big` extraction + local caching.
- [x] **Metal Stage 0 (Skeleton):** ✅ MetalDevice8 + MetalInterface8.
- [x] **Metal Stage 1 (Scene+Clear):** ✅ BeginScene/EndScene/Present/Clear + Viewport.
- [x] **Metal Stage 2 (VB/IB):** ✅ MetalVertexBuffer8/IndexBuffer8 + Lock/Unlock + W3DNEW.
- [x] **Metal Stage 3 (Pipeline+Draw):** ✅ FVF→VertexDescriptor, PSO cache, DrawPrimitive/DrawIndexedPrimitive.
- [x] **Metal Stage 4 (Transforms):** ✅ SetTransform/GetTransform, uniform buffer with World/View/Projection.
- [x] **Metal Stage 5 (Textures):** 🟢 **Consolidated.** All loading via `D3DXStubs` → `MetalDevice8`. No legacy `MacOSRenderDevice` deps.
- [x] **Metal Stage 6 (Render States):** ✅ Dynamic PSO (blend factors, color write mask), depth/stencil state cache.
- [ ] **Metal Stage 7 (TSS):** ✅ Full DX8 TSS (2 stages, sampler cache, FragmentUniforms).
- [ ] **Metal Stage 8 (Lighting):** ✅ Full DX8 per-vertex (4 lights, materials, normals).
- [ ] **Metal Stage 9 (Fog):** ✅ Full DX8 vertex fog.
- [ ] **Metal Stage 10 (Depth+RT):** ✅ Depth+Stencil buffer (`Depth32Float_Stencil8`).
- [x] **SW DDS Decoding:** DXT1/5 textures handled. Lazy Metal buffer sync.
- [x] **Initialization:** ✅ All critical subsystems init (TheGameClient, TheGameLogic, TheAI, ThePlayerList).
- [x] **3D Infrastructure:** `W3DView` integrated; `RTS3DScene` pools initialized.
- [x] **Input Handling:** ✅ Mouse 1:1 absolute coordinates. Fixed 2x scaling bug.
- [x] **Dynamic Window Size:** ✅ `MacOS_GetScreenSize()` replaces hardcoded 800×600.
- [x] **Stable Runtime:** ✅ Game runs stably. Shell/menu loop ticks.
- [ ] **Main Menu:** ⚠️ **Layout Visible**, but textures missing.
    - [x] Buttons exist and are clickable.
    - [ ] **REGRESSION:** Buttons are white rectangles (Texture binding issue).
    - [ ] **REGRESSION:** Text is missing/invisible.
- [x] **Legacy Pipeline Removal:** ✅ `MacOSRenderDevice` and `Shaders.metal` removed. Codebase unified on `MetalDevice8`.

---

## 🔴 ACTIVE ISSUES
- [ ] 🔴 **White UI Buttons:** `winDrawImage` is called, but textures render as white. Likely `SetTexture` not propagating or texture loading failing silently in new `D3DXStubs` path.
- [ ] 🔴 **Missing Text:** `MacOSDisplayString::draw` executes, but text is invisible. Likely texture alpha/binding issue or shader config for text.
- [x] ✅ **Legacy Backend Removed:** Cleaned `files.sh`, rewrote `D3DXStubs.mm` and `MacOSDisplayString.mm` to remove all `GetRenderDevice()` calls.
- [x] ✅ **Build Fixes:** Fixed linker errors, removed dead code (`D3DFormat_To_WW3DFormat`, unused stubs).

---

## 🏗 SHORT-TERM BACKLOG (ordered by priority)
1.  **Fix UI Textures (White Rectangles):**
    -   Verify `D3DXCreateTextureFromFileExA` in `D3DXStubs.mm` correctly populates `MetalTexture8`.
    -   Check `MetalDevice8::SetTexture` binding to encoder.
2.  **Fix Text Rendering:**
    -   Verify `MacOSDisplayString` font texture creation.
    -   Check alpha blending state during text draw.
3.  **Fix XYZRHW vertex mismatch:**
    -   [ ] Split into vertex_main_2d/vertex_main_3d or use `float4` in VertexIn.
4.  **Metal Stage 5 completion: 16-bit texture format conversion**
    -   [ ] R5G6B5, A1R5G5B5, A4R4G4B4 → CPU conversion to BGRA8.
5.  **Root-Cause TerrainVisual ERROR_BUG:** The `NEW_REF` assertion in `W3DTerrainVisual::init()`.

---

## 📚 External References
| Project | GitHub Link | Local Paths | Description |
| :--- | :--- | :--- | :--- |
| **Fighter19 (Linux)** | [Fighter19/CnC_Generals](https://github.com/Fighter19/CnC_Generals_Zero_Hour) | `/Users/okji/dev/games/CnC_Generals_Zero_Hour` | Native Linux port. |
| **TheSuperHackers** | [TheSuperHackers/Generals](https://github.com/TheSuperHackers/GeneralsGameCode) | `/Users/okji/dev/games/GeneralsX` | Modernized C++20 branch. |
| **Aspyr Port (macOS)** | N/A | `/Applications/Command & Conquer™ Generals Zero Hour.app` | Binary reference. |

---

## ⚠️ BUILD NOTES
- **Clean build recommended:** `rm -rf build/macos/*.o && bash build_macos.sh`.
- **941 objects** total.

---

##  DECISIONS & CONSTRAINTS
- **Unified Pipeline:** All rendering MUST go through `MetalDevice8`. No side-channels.
- **Null-Safety:** All globals (`TheGameLogic`, `TheInGameUI`, etc.) must be null-checked.
- **Texture Loading:** Uses `D3DXStubs` shim to load via `MetalDevice8` (LockRect/UnlockRect).
