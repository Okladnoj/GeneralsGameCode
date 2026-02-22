# macOS Port — Rendering Pipeline

> Last updated: 2026-02-22 — based on source-level code analysis and runtime logging.

---

## 1. Frame-Level Entry Point

```
MacOSGameClient::update()          // Platform/MacOS/Source/Main/MacOSGameClient.mm:162
  ├─ MacOS_PumpEvents()            // macOS event loop
  ├─ GameClient::update()          // Core/GameEngine/Source/GameClient/GameClient.cpp
  │   └─ Display::draw()           // virtual → W3DDisplay::draw()
  └─ (shell map forced on first frame via TheShell->showShellMap)
```

### W3DDisplay::draw() — Main Render Frame
**File:** `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DDisplay.cpp:1658`

```
W3DDisplay::draw()
  ├─ updateAverageFPS()
  ├─ calculateTerrainLOD()           // if auto LOD
  ├─ [pre-render updates]
  │   ├─ updateViews()               // recompute camera, frustum
  │   ├─ ParticleSystemManager::update()
  │   ├─ WaterRenderObj::updateRenderTargetTextures()    // water reflections
  │   └─ ProjectedShadowManager::updateRenderTargetTextures()
  │
  ├─ WW3D::Begin_Render()           // ← GPU frame start
  │   ├─ drawViews()                 // ← ALL 3D WORLD RENDERING
  │   ├─ TheInGameUI->DRAW()         // 2D UI overlay
  │   ├─ TheMouse->DRAW()
  │   └─ [letterbox, debug stats]
  └─ WW3D::End_Render()             // ← GPU frame end (present)
```

⚠️ **CRITICAL:** `m_breakTheMovie` must remain `FALSE` on macOS. Setting it to `TRUE` causes `W3DDisplay::draw()` (line 1849) to skip `WW3D::Begin_Render()`, which disables **ALL** 3D rendering.

---

## 2. WW3D Core Rendering

### WW3D::Begin_Render / End_Render
**File:** `Core/Libraries/Source/WWVegas/WW3D2/ww3d.cpp`

```
WW3D::Begin_Render(clear, clearZ, clearColor, minWaterOpacity)
  ├─ TextureLoader::Update()         // ← PROCESSES TEXTURE LOAD QUEUE (see §5)
  ├─ DX8Wrapper::Begin_Render()
  │   └─ MetalDevice8::BeginScene()  // creates MTLCommandBuffer, acquires drawable
  ├─ DX8Wrapper::Clear()             // clear framebuffer
  └─ [frame timing setup]

WW3D::End_Render()
  ├─ DX8Wrapper::End_Render()
  │   ├─ MetalDevice8::EndScene()    // endEncoding on current encoder
  │   └─ MetalDevice8::Present()     // presentDrawable + commit + waitUntilCompleted
  └─ [frame counters]
```

### drawViews() → W3DView::drawView()
**File:** `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DView.cpp`

```
W3DDisplay::drawViews()
  └─ for each View:
       W3DView::drawView()
         ├─ WW3D::Render(m_3DScene, m_3DCamera)    // ← MAIN 3D SCENE
         ├─ WW3D::Render(m_3DInterfaceScene)         // 3D interface overlay
         └─ WW3D::Render(m_2DScene)                  // 2D scene
```

---

## 3. Scene Rendering (W3D Scene Graph)

### WW3D::Render → RTS3DScene::Render
**File:** `Core/Libraries/Source/WWVegas/WW3D2/ww3d.cpp` (WW3D::Render)
**File:** `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DScene.cpp`

```
WW3D::Render(scene, camera)
  └─ scene->Render(camera)   // RTS3DScene::Render (line 972)
      ├─ Set fog state
      ├─ updatePlayerColorPasses()
      ├─ updateFixedLightEnvironments(rinfo)   // global lights, infantry light
      ├─ Customized_Render(rinfo)              // ← SUBMIT objects to batches
      └─ Flush(rinfo)                          // ← EXECUTE all draw calls
```

### Customized_Render() — Submit Phase (W3DScene.cpp:1103)
```
Customized_Render(rinfo)
  ├─ Visibility_Check(&camera)       // frustum cull all objects
  ├─ On_Frame_Update() for all objects in UpdateList
  │
  ├─ TERRAIN FIRST (if found):
  │   └─ terrainObject->Render(rinfo)  // HeightMapRenderObjClass::Render()
  │       with shroud material pass pushed
  │
  ├─ ALL OTHER OBJECTS (RenderList):
  │   for each visible RenderObjClass:
  │     └─ renderOneObject(rinfo, robj)
  │         ├─ compute light environment (dynamic lights per object)
  │         ├─ check shroud status (CLEAR/PARTIAL/FOGGED)
  │         └─ robj->Render(rinfo)
  │             ├─ HLodClass → picks LOD level → MeshClass::Render()
  │             └─ MeshClass → submits to DX8MeshRenderer batch
  │
  └─ Shadow/particle queue setup
```

### Flush() — Execute Phase (W3DScene.cpp:846)
```
Flush(rinfo)
  ├─ PrepareShadows()
  ├─ DoShadows(rinfo, false)                  // non-stencil shadows (decals)
  ├─ TheDX8MeshRenderer.Flush()               // ← ALL OPAQUE MESHES DRAWN HERE
  │   └─ for each batched mesh:
  │       MeshClass::Render_Skin/Rigid()
  │       → MeshMatDescClass::Do_Pass()
  │       → TextureClass::Apply() + ShaderClass::Apply()
  │       → DX8Wrapper::Draw_Triangles()
  ├─ flushOccludedObjectsIntoStencil(rinfo)   // behind-building markers
  ├─ SHD_FLUSH                                // custom shaders (CNC3-era)
  ├─ DoTrees(rinfo)                           // trees (alpha blend)
  ├─ DoShadows(rinfo, true)                   // stencil shadows
  ├─ Render_And_Clear_Static_Sort_Lists()      // water, etc.
  ├─ flushTranslucentObjects(rinfo)            // translucent meshes
  ├─ DoParticles(rinfo)                        // particle systems
  └─ SortingRendererClass::Flush()             // sorted translucent polygons
```

### Visibility & Culling

1. **Iterate RenderList** — all top-level `RenderObjClass` objects
2. **Force Visible** — `robj->Is_Force_Visible()` → immediate pass
3. **Hidden Check** — `robj->Is_Hidden()` → immediate reject
4. **Frustum Culling** — `camera->Cull_Sphere(robj->Get_Bounding_Sphere())`
5. **Gameplay Visibility** — stealth, fog of war checks
6. **Binning** — translucent, occluders, occludees, normal objects

---

## 4. Mesh Rendering (DX8 Draw Calls)

### MeshClass::Render → DX8 Pipeline
**File:** `Core/Libraries/Source/WWVegas/WW3D2/mesh.cpp`
**File:** `Core/Libraries/Source/WWVegas/WW3D2/meshmatdesc.cpp`

```
MeshClass::Render(rinfo)
  └─ MeshModelClass::Render_Skin() / Render_Rigid()
      └─ MeshMatDescClass::Do_Pass()
           │
           │  *** PATH A: Fixed-function (most terrain/objects) ***
           ├─ TextureClass::Apply(stage)           // texture.cpp:982
           │   ├─ if (!Initialized) Init()         // lazy-load texture
           │   ├─ DX8Wrapper::Set_DX8_Texture(stage, Peek_D3D_Base_Texture())
           │   │   └─ MetalDevice8::SetTexture(stage, pTexture)
           │   │       └─ stores texture for next draw call
           │   └─ Filter.Apply(stage)              // min/mag/mip filters
           │
           ├─ ShaderClass::Apply()                 // set render states
           │   └─ DX8Wrapper::Set_DX8_Render_State(...)
           │       └─ MetalDevice8::SetRenderState(...)
           │
           ├─ DX8Wrapper::Set_Vertex_Buffer(vb)
           ├─ DX8Wrapper::Set_Index_Buffer(ib)
           └─ DX8Wrapper::Draw_Triangles()
               └─ DX8Wrapper::Apply_Render_State_Changes()
                   └─ MetalDevice8::DrawIndexedPrimitive()
           │
           │  *** PATH B: W3DShaderManager (water, road, terrain blending) ***
           ├─ W3DShaderManager::setShader(shaderType, pass)
           │   ├─ getShaderTexture(stage)->Peek_D3D_Texture()
           │   │   └─ Returns IDirect3DTexture8* from TextureClass
           │   └─ DX8Wrapper::Set_DX8_Texture(stage, texture)
           └─ [same DX8 draw path as above]
```

### MetalDevice8::DrawIndexedPrimitive — GPU Submit
**File:** `Platform/MacOS/Source/Metal/MetalDevice8.mm`

```
MetalDevice8::DrawIndexedPrimitive(primType, baseVertex, minIndex, numVerts, startIdx, primCount)
  ├─ ensureRenderEncoder()           // create MTLRenderCommandEncoder if needed
  ├─ Build/cache MTLRenderPipelineState:
  │   ├─ vertex shader: maps FVF → Metal vertex function
  │   ├─ fragment shader: maps TSS state → Metal fragment function
  │   ├─ blend state from DX8 render states
  │   └─ pixel format from current render target
  ├─ [encoder setRenderPipelineState:pso]
  ├─ [encoder setFragmentTexture:m_BoundTextures[stage] atIndex:stage]
  ├─ Set vertex/fragment constants (transforms, material, fog)
  ├─ [encoder setVertexBuffer:vertexBuffer offset:... atIndex:0]
  └─ [encoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                        indexCount:primCount*3
                         indexType:MTLIndexTypeUInt16
                       indexBuffer:indexBuffer
                 indexBufferOffset:startIdx*2]
```

---

## 5. Texture Loading Pipeline

### Overview
```
TextureClass::ctor(name)           // Creates TextureClass with name, NO data
  → TextureClass::Init()           // Called lazily on first Apply(), or eagerly
    → TextureLoader::Request_Foreground_Loading(this)
      → if (Is_DX8_Thread()):
          TextureLoadTaskClass::Finish_Load()   // SYNCHRONOUS in main thread
            ├─ Begin_Load()
            │   ├─ Begin_Compressed_Load()      // for DDS/DXT textures
            │   │   ├─ Get texture info from file
            │   │   └─ D3DTexture = _Create_DX8_Texture(w, h, fmt, mips, MANAGED)
            │   │       └─ MetalDevice8::CreateTexture → MetalTexture8::ctor
            │   │           └─ [mtlDev newTextureWithDescriptor:desc]
            │   └─ Begin_Uncompressed_Load()    // for TGA textures
            │       └─ same pattern
            ├─ Load()
            │   ├─ Lock_Surfaces()
            │   │   └─ D3DTexture->LockRect(level, &lockedRect, ...)
            │   │       └─ MetalTexture8::LockRect → malloc staging buffer
            │   ├─ Load_Compressed_Mipmap() / Load_Uncompressed()
            │   │   └─ DDSFileClass::Copy_Level_To_Surface()
            │   │       └─ memcpy file data → locked staging buffer
            │   └─ [data is now in staging buffer]
            └─ End_Load()
                ├─ Unlock_Surfaces()
                │   └─ D3DTexture->UnlockRect(level)
                │       └─ MetalTexture8::UnlockRect
                │           ├─ [tex replaceRegion:... withBytes:stagingBuffer]
                │           │   └─ Data uploaded to Metal GPU texture! ✅
                │           └─ free(stagingBuffer)
                └─ Apply(initialize=true)
                    └─ TextureClass::Apply_New_Surface(D3DTexture)
                        ├─ old = Peek_D3D_Base_Texture()
                        ├─ if (old) old->Release()
                        ├─ Poke_Texture(D3DTexture)    // ← STORES in TextureClass
                        ├─ D3DTexture->AddRef()
                        └─ Initialized = true
```

### Key Insight: Texture Pointer Chain

```
TextureLoadTaskClass::D3DTexture        ──(Begin_Load creates)──→  MetalTexture8 [mtl=A]
                                                                      │
TextureLoadTaskClass::End_Load()        ──(Apply_New_Surface)──→  TextureClass::D3DTexture = ptr
                                                                      │
TextureClass::Apply(stage)              ──(Peek_D3D_Base_Texture)──→  same MetalTexture8 [mtl=A]
                                                                      │
DX8Wrapper::Set_DX8_Texture(stage, ^)   ──(stores in device)──→  MetalDevice8::m_BoundTextures[stage]
                                                                      │
MetalDevice8::DrawIndexedPrimitive()    ──([encoder setFragmentTexture])──→ GPU uses mtl=A ✅
```

### Terrain Texture (Separate Path)
**File:** `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/HeightMap.cpp`

```
HeightMapRenderObjClass creates terrain composite textures:
  ├─ _Create_DX8_Texture(2048, 1024, A1R5G5B5, 3, MANAGED)
  │   └─ MetalTexture8::ctor → creates Metal texture
  ├─ GetSurfaceLevel(0, &surface)
  │   └─ MetalTexture8::GetSurfaceLevel → MetalSurface8::ctor
  ├─ surface->LockRect(&lockedRect, ...)
  │   └─ MetalSurface8::LockRect → malloc buffer, memset(0)
  ├─ [HeightMap blends tile textures into surface buffer]
  ├─ surface->UnlockRect()
  │   └─ MetalSurface8::UnlockRect
  │       ├─ Convert 16-bit A1R5G5B5 → 32-bit BGRA8
  │       └─ [tex replaceRegion:...] → GPU upload
  └─ TextureClass(w,h) stores this texture for terrain rendering
```

### Surface Lifetime (⚠️ Critical Bug History)
The original implementation freed `m_LockedData` in `UnlockRect()`. This caused use-after-free crashes because W3D classes (W3DShroud, TerrainTex) hold surface pointers and write to them repeatedly without re-locking. The buffer now lives for the lifetime of the `MetalSurface8` object.

---

## 6. DX8 → Metal Adapter

### Key Components

| Component | Role |
|:---|:---|
| `MetalInterface8` | Implements `IDirect3D8` — adapter enumeration, device creation |
| `MetalDevice8` | Implements `IDirect3DDevice8` — `MTLDevice`, `MTLCommandQueue`, `CAMetalLayer` |
| `MetalTexture8` | Implements `IDirect3DTexture8` — wraps `MTLTexture` |
| `MetalSurface8` | Implements `IDirect3DSurface8` — staging buffer + parent texture upload |
| `MetalVertexBuffer8` | Implements `IDirect3DVertexBuffer8` — wraps vertex data |
| `MetalIndexBuffer8` | Implements `IDirect3DIndexBuffer8` — wraps index data |
| `D3DXStubs.mm` | Factory functions — isolates C++ from Objective-C++ |

### Format Mapping

| D3D Format | Metal Format |
|:---|:---|
| ARGB8 / XRGB8 | `MTLPixelFormatBGRA8Unorm` |
| DXT1 | `MTLPixelFormatBC1_RGBA` |
| DXT3 | `MTLPixelFormatBC2_RGBA` |
| DXT5 | `MTLPixelFormatBC3_RGBA` |
| 16-bit (R5G6B5, etc.) | Converted to BGRA8 in MetalSurface8 |

### Pipeline State Objects (PSO)

`GetPSO(fvf, tssKey, blendKey)` creates or retrieves from cache:

#### Vertex Descriptor (from FVF)

| FVF Flag | Attribute | Format | Size |
|:---|:---|:---|:---|
| `D3DFVF_XYZ` | attr[0] position | Float3 | 12B |
| `D3DFVF_XYZRHW` | attr[0] position | Float4 | 16B |
| `D3DFVF_DIFFUSE` | attr[1] color | UChar4Normalized | 4B |
| `D3DFVF_TEX1` | attr[2] texCoord0 | Float2 | 8B |
| `D3DFVF_NORMAL` | attr[3] normal | Float3 | 12B |
| `D3DFVF_SPECULAR` | attr[4] specular | UChar4Normalized | 4B |
| `D3DFVF_TEX2` | attr[5] texCoord1 | Float2 | 8B |

#### Uniform Buffers

| Buffer Index | Stage | Contents |
|:---|:---|:---|
| buffer(0) | Vertex | Vertex data (VB or inline) |
| buffer(1) | Vertex | `MetalUniforms` — World/View/Projection, screenSize, useProjection |
| buffer(2) | Fragment | `FragmentUniforms` — TSS config, textureFactor, fog, alpha test |
| buffer(3) | Vertex | `LightingUniforms` — lights, material, fog params |

---

## 7. Texture Stage State (TSS) → Metal Shader Mapping

The fixed-function DX8 pipeline uses Texture Stage States to control how textures are combined.
Our Metal port encodes TSS into a fragment shader key:

```
TSS State                    Metal Equivalent
─────────────────────────────────────────────────
D3DTSS_COLOROP               Fragment shader operation (DISABLE/SELECTARG1/MODULATE/...)
D3DTSS_COLORARG1/2           Fragment shader input (TEXTURE/DIFFUSE/CURRENT/TFACTOR)
D3DTSS_ALPHAOP               Same as color but for alpha channel
D3DTSS_ALPHAARG1/2           Same as color args
D3DTSS_TEXCOORDINDEX         Which UV set to use
D3DTSS_ADDRESSU/V            MTLSamplerAddressMode (wrap/clamp/mirror)
D3DTSS_MINFILTER/MAGFILTER   MTLSamplerMinMagFilter (point/linear)
D3DTSS_MIPFILTER             MTLSamplerMipFilter
```

---

## 8. Render State Flow

### DX8Wrapper State Caching
**File:** `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/dx8wrapper.cpp`

```
DX8Wrapper maintains:
  CurrentRenderState[256]     // D3DRS_* values
  CurrentTextureStageState[8][32]  // TSS per stage
  Textures[8]                 // bound textures per stage
  VertexBuffers[]             // bound vertex buffers
  IndexBuffer                 // bound index buffer
  Material                    // current D3DMATERIAL8

Apply_Render_State_Changes()  // called before EVERY draw call
  ├─ Compare cached vs desired states
  ├─ Only send changed states to Metal device
  └─ MetalDevice8::SetRenderState/SetTextureStageState(...)
```

---

## 9. Shell Map Loading Flow

On macOS, the intro/sizzle movie state machine is bypassed because `VideoPlayer::open()` returns `nullptr`:

```
MacOSGameClient::update()  (callCount == 0)
  → m_playIntro = FALSE
  → m_afterIntro = FALSE
  → GameClient::update()     ← base class, state machine skipped
  → TheShell->showShellMap(TRUE)
    → m_pendingFile = "Maps\ShellMapMD\ShellMapMD.map"
    → MSG_NEW_GAME dispatched (GAME_SHELL)
    → m_shellMapOn = TRUE
  → TheShell->showShell()
    → pushes MainMenu.wnd

Next frames:
  → GameLogic processes MSG_NEW_GAME
  → prepareNewGame() → startNewGame(FALSE)
  → Shell map terrain + objects loaded
  → isInGame=1, gameMode=GAME_SHELL
  → drawViews() renders 3D scene
```

---

## 10. 2D Rendering & UI

### Screen-Space Rendering (XYZRHW)
For `D3DFVF_XYZRHW` (screen-space / 2D) vertices, three critical overrides apply:

1. **Depth test & write disabled** — 2D UI must render on top of 3D geometry
2. **Back-face culling disabled** — The vertex shader flips Y for screen→NDC conversion, which reverses triangle winding. Without disabling culling, all 2D triangles are discarded.
3. **Projection bypass** — `useProjection == 2` uses screen-space→NDC transform instead of the standard MVP pipeline.

### UI Widget Rendering (W3D Gadgets)

```
MacOSGameWindowManager → W3DGameWindowManager → GameWindowManager
                                 ↓
                    W3DGadget*Draw functions
                    (PushButton, ComboBox, ListBox,
                     Slider, ProgressBar, StaticText, etc.)
                                 ↓
                    TheWindowManager->winDrawImage()
                                 ↓
                    TheDisplay->drawImage()
                                 ↓
                    Render2DClass → DX8Wrapper → MetalDevice8
```

### MacOSGameWindow (fontData Safety)
`W3DGameWindow` uses `Render2DSentenceClass` for text rendering, which requires `FontCharsClass` (initialised via Windows GDI `CreateFont`). On macOS, `fontData = nullptr` because fonts use CoreText/NSFont via `MacOSDisplayString`.

`MacOSGameWindow` overrides:
- `winSetFont()` — skips `m_textRenderer.Set_Font()` (avoids nullptr crash)
- `winSetText()` — skips `m_textRenderer.Build_Sentence()`
- `drawText()` — no-op (text is drawn through `MacOSDisplayString`)

---

## 11. Key File Reference

| Component | File | Key Methods |
|---|---|---|
| **Frame loop** | `W3DDisplay.cpp` | `draw()`, `drawViews()` |
| **Scene graph** | `W3DScene.cpp` | `Customized_Render()`, `Flush()` |
| **View/Camera** | `W3DView.cpp` | `drawView()` |
| **Mesh render** | `mesh.cpp`, `meshmatdesc.cpp` | `Render()`, `Do_Pass()` |
| **Texture class** | `texture.cpp` | `Init()`, `Apply()`, `Apply_New_Surface()` |
| **Texture loader** | `textureloader.cpp` | `Finish_Load()`, `Begin_Compressed_Load()` |
| **DX8 wrapper** | `dx8wrapper.cpp/.h` | `Set_DX8_Texture()`, `Draw_Triangles()` |
| **Metal device** | `MetalDevice8.mm` | `DrawIndexedPrimitive()`, `SetTexture()` |
| **Metal texture** | `MetalTexture8.mm` | `LockRect()`, `UnlockRect()` |
| **Metal surface** | `MetalSurface8.mm` | `LockRect()`, `UnlockRect()` |
| **Terrain** | `HeightMap.cpp` | `Render()`, terrain tex compositing |
| **Shader manager** | `W3DShaderManager.cpp` | `setShader()`, custom shader paths |
| **D3DX stubs** | `D3DXStubs.mm` | `D3DXCreateTexture`, `D3DXFilterTexture` |
| **Metal shaders** | `MacOSShaders.metal` | `vertex_main`, `fragment_main` |
| **Window manager** | `MacOSGameWindowManager.mm` | Widget creation, text rendering |

---

## 12. Debugging Notes

### stderr vs stdout
All diagnostic logging MUST use `printf()` + `fflush(stdout)` — NOT `fprintf(stderr, ...)`.
`stderr` is not captured in `game.log` on macOS.

### Texture Address Confusion
In logs, there are TWO different pointers per texture:
- **`tex=0xAAAA`** = `MetalTexture8*` (C++ object pointer)
- **`mtl=0xBBBB`** = `id<MTLTexture>` (Metal GPU texture handle, stored in `m_Texture`)

`MetalTexture8::Created` logs both: `tex=(void*)this  mtl=(void*)m_Texture`.
`MetalTexture8::UnlockRect` logs `(void*)m_Texture` as `tex=` (this is actually the **mtl** pointer).

### Texture Loading Timing
All ~500 textures are loaded **synchronously** via `Request_Foreground_Loading` **BEFORE** the first render frame (between update #0 and update #2). There is no race condition.

### Surface Upload Zero-Data
`MetalSurface8::LockRect()` allocates buffer with `memset(0)`. If caller writes data → `UnlockRect()` uploads non-zero data. Some surface levels show `nonZero=0` because the mipmap generation path may not write to all levels, or the `D3DXFilterTexture` stub is a no-op.

---

## 13. PixelShaderVersion = 0 (Critical Fix)

**File:** `Platform/MacOS/Source/Metal/MetalDevice8.mm` — `GetDeviceCaps()`

Metal cannot execute D3D8 pixel shader bytecode (`.pso` files). The game loads pixel shaders for terrain, water, and road rendering via `W3DShaderManager::LoadAndCreateD3DShader()`.

**Fix:** Report `PixelShaderVersion = 0x0000` in device caps, so `W3DShaderManager::getChipset()` returns `DC_GENERIC_2TEXTURE_ONLY` instead of `DC_GENERIC_PIXEL_SHADER_1_1`.

**Effect:** All terrain/road/extra-blend shaders use **multi-pass TSS** (Texture Stage State) rendering paths instead of pixel shaders:
- `flatTerrainShader2Stage` instead of `flatTerrainShaderPixelShader`
- `TerrainShader2Stage::set(pass)` configures TSS per-pass
- Pass count increases (e.g., 4 passes for terrain with cloud+noise vs 1 with pixel shader)

---

## 14. Terrain Rendering Pipeline (TerrainShader2Stage)

**Files:**
- `Core/GameEngineDevice/Source/W3DDevice/GameClient/HeightMap.cpp` — `HeightMapRenderObjClass::Render()`
- `Core/GameEngineDevice/Source/W3DDevice/GameClient/W3DShaderManager.cpp` — `TerrainShader2Stage::set()`
- `Core/GameEngineDevice/Source/W3DDevice/GameClient/BaseHeightMap.cpp` — `doTheLight()`

### Render Flow
```
HeightMap::Render()
 ├── Set_Material(m_vertexMaterialClass)  // PRELIT_DIFFUSE
 ├── Set_Shader(m_shaderClass)            // detailOpaqueShader
 ├── setTexture(0, m_stageZeroTexture)    // 2048x1024 composite
 ├── setTexture(1, m_stageZeroTexture)    // same texture, different UVs
 ├── st = ST_FLAT_TERRAIN_BASE_NOISE12    // with cloud + lightmap
 ├── devicePasses = getShaderPasses(st)   // → 4 (2 terrain + 2 cloud/noise)
 │
 ├── for pass in 0..3:
 │    ├── TerrainShader2Stage::set(pass):
 │    │    ├── Apply_Render_State_Changes()  ← runs ShaderClass::Apply()
 │    │    ├── case 0: SetTexture(0, tex) directly on device
 │    │    │   ColorOp=MODULATE(TEXTURE, DIFFUSE), AlphaOp=DISABLE
 │    │    │   ALPHABLENDENABLE=false, TEXCOORDINDEX=0
 │    │    │
 │    │    ├── case 1: SetTexture(0, tex) with UV set 1
 │    │    │   ColorOp=MODULATE, AlphaOp=MODULATE(texture alpha × diffuse alpha)
 │    │    │   ALPHABLENDENABLE=true, SRCBLEND=SRCALPHA, DSTBLEND=INVSRCALPHA
 │    │    │
 │    │    ├── case 2: Cloud layer (camera-space texture generation)
 │    │    └── case 3: Noise/lightmap layer
 │    │
 │    └── for each VB tile: Draw_Triangles(0, 338, 0, 196)
 │
 ├── renderShoreLines()        // alpha blended shoreline transitions
 ├── renderExtraBlendTiles()   // 3-way blend corners (ST_ROAD_BASE shader)
 └── drawRoads()               // road overlays
```

### Vertex Format
FVF = `0x252` = `D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2`

```
struct VertexFormatXYZNDUV2 {
    float x, y, z;        // world position
    float nx, ny, nz;     // normal (unused in PRELIT_DIFFUSE, zeroed)
    DWORD diffuse;        // ARGB: alpha=blend_amount, RGB=baked lighting
    float u1, v1;         // UV set 0: base terrain atlas mapping
    float u2, v2;         // UV set 1: individual tile transition mapping
};
```

**Alpha channel** in `diffuse`: controls pass 1 blend amount. Value comes from `getAlphaUVData()`. Alpha=0 means no texture blend at this vertex (only base terrain shows).

### Terrain Lighting
Terrain uses **pre-baked vertex lighting** (PRELIT_DIFFUSE mode):
- `doTheLight()` in `BaseHeightMap.cpp` calculates: ambient + N·L for each global light
- **Depth fade**: terrain below `waterPositionZ` is darkened (simulates light absorption in water)
- Result stored in vertex `diffuse` (ARGB), alpha from map's `getAlphaUVData()`

---

## 15. Water Rendering System

**File:** `Core/GameEngineDevice/Source/W3DDevice/GameClient/Water/W3DWater.cpp`

### Water Types
```
WATER_TYPE_0_TRANSLUCENT  ← USED IN ZERO HOUR (confirmed via printf)
WATER_TYPE_1_FB_REFLECTION  ← legacy, rarely used
WATER_TYPE_2_PVSHADER       ← disabled in ZH (#if 0 at line 1044)
WATER_TYPE_3_GRIDMESH       ← grid-based water mesh
```

### Render Flow (WATER_TYPE_0_TRANSLUCENT)
```
WaterRenderObjClass::Render()  [at static sort level 2, AFTER terrain+meshes]
 ├── renderWater()
 │    └── for each PolygonTrigger that isWaterArea():
 │        └── drawTrapezoidWater(points[4])
 │            ├── Generate vertex grid with wave animation
 │            ├── Vertex diffuse: RGB=water color (lit), Alpha=opacity from settings
 │            └── Draw with alpha blending (SRC_ALPHA / INV_SRC_ALPHA)
 │
 ├── renderWaterMesh()  // if m_doWaterGrid
 └── m_skyBox->Render()  // if TheGlobalData->m_drawSkyBox
```

### Key Points
- Water only draws at **PolygonTrigger** marked areas (NOT entire map)
- Shell map is an island — water covering flat areas is CORRECT
- `drawSea()` (vertex/pixel shader water) is **disabled** in ZH build (`#if 0`)
- Water uses `DEPTH_WRITE_DISABLE` — doesn't overwrite terrain in Z-buffer
- Skybox renders as part of WaterRenderObjClass (sort level 2)

---

## 16. Known Issues & Status

| Feature | Status | Notes |
|:--------|:-------|:------|
| Terrain textures | ✅ Working | Base + blend tiles render correctly |
| Roads | ✅ Working | Via `roadBuffer->drawRoads()` |
| Smoke/particles | ✅ Working | Particle system renders |
| Water surface | ✅ Working | WATER_TYPE_0_TRANSLUCENT via polygon triggers |
| Sky/skybox | ✅ Working | Rendered in sort level 2 |
| 3D mesh textures | ❌ White | BC1/DXT1 textures not applied (lighting/TSS issue) |
| Unit visibility | ❌ Not visible | Need investigation in game mission |
| Mipmap generation | ❌ Stub | D3DXFilterTexture is no-op |
