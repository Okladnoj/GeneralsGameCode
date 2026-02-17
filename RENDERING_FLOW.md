# Rendering Flow (macOS/Metal)

This document maps the rendering lifecycle from high-level engine logic down to low-level Metal API calls.

## Overview Flow

```mermaid
graph TD
    A[MacOSMain.mm :: main] --> B[MacOSWindowManager.mm :: MacOS_Main]
    B --> C[GameMain.cpp :: GameMain]
    C --> D[GameEngine.cpp :: execute]
    
    subgraph "Initialization & Object Creation"
        D1[GameLogic::sendObjectCreated] --> D2[TheThingFactory::newDrawable]
        D2 --> D3[W3DScene::Add_Render_Object]
        D3 --> RenderList[RenderList]
    end

    subgraph "Main Loop"
        D --> E[MacOS_PumpEvents]
        D --> F[GameEngine::update]
        F --> G[GameClient::UPDATE]
        G --> H[MacOSDisplay::draw]
    end

    subgraph "MacOSDisplay :: draw"
        H --> H1[WW3D::Begin_Render]
        H --> H2[Display::draw]
        H2 --> H3[W3DView::draw]
        H --> H4[WindowManager::winRepaint -UI-]
        H --> H5[WW3D::End_Render]
    end

    subgraph "Scene Rendering Chain"
        H3 --> I[W3DView::draw :: Calls doRender]
        I --> J[RTS3DScene::doRender :: Calls DRAW]
        J --> K[WW3D::Render -Scene, Cam-]
        K --> L[RTS3DScene::Render :: Main Entry]
        L --> M[RTS3DScene::Customized_Render]
    end

    subgraph "Visibility & Culling (Customized_Render)"
        M --> N[Visibility_Check]
        N --> N1[CameraClass::Cull_Sphere]
        N1 --> N2[CollisionMath::Overlap_Test -Frustum-]
        N2 --> N3[BUG FIX: AAPlane Sphere Test Corrected]
        N --> O[Shroud/Stealth Logic]
        N --> P[Categorization into Render Bins]
    end

    subgraph "Metal Backend (DX8 Adapter)"
        M --> Q[Flush / renderOneObject]
        Q --> R[MetalDevice8 :: DrawPrimitive]
        R --> S[Metal API / Command Encoders]
    end
```

## Metal DirectX 8 Adapter (Stage 0)

The macOS port uses a direct implementation of the `IDirect3D8` and `IDirect3DDevice8` interfaces (defined in `d3d8_stub.h`) to bridge the engine's legacy calls to the Apple Metal API.

### Key Components

- **`MetalInterface8`**: Implements `IDirect3D8`. Handles adapter enumeration and device creation.
- **`MetalDevice8`**: Implements `IDirect3DDevice8`. Manages `MTLDevice`, `MTLCommandQueue`, and the `CAMetalLayer`.
- **`D3DXStubs.cpp`**: Contains `extern "C"` factory functions to create these objects, isolating C++ from Objective-C++ where possible.

### `win_compat.h` / Metal Isolation

To avoid naming conflicts between Windows API stubs in `win_compat.h` and macOS system headers (e.g., `LoadResource`, `GetCurrentThread`), the following patterns are used:

1.  **Opaque Pointers**: `MetalDevice8.h` uses `void*` for all Metal/ObjC types to avoid importing `<Metal/Metal.h>` in the header.
2.  **Import Order**: In `.mm` files, macOS framework headers are imported **before** any headers that include `win_compat.h`.
3.  **Conditional Guards**: Conflicting functions in `win_compat.h` are wrapped in `#if !defined(__OBJC__)`.

### Frame Lifecycle in `MetalDevice8`

1.  **`BeginScene`**: Acquires the next `id<CAMetalDrawable>` and creates a `id<MTLCommandBuffer>`.
2.  **`Clear`**: Ends the current encoder (if any) and creates a new `MTLRenderCommandEncoder` with clear actions.
3.  **`DrawPrimitive` (Stage 1)**: Will bind vertex/index buffers and issue Metal draw commands.
4.  **`Present`**: Commits the command buffer and presents the drawable.


## Detailed Visibility Check (`RTS3DScene::Visibility_Check`)

Located in `GeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/W3DScene.cpp`.

1.  **Iterate `RenderList`**: Loops through all top-level `RenderObjClass` objects.
2.  **Force Visible**: If `robj->Is_Force_Visible()`, object is marked visible immediately.
3.  **Hidden Check**: If `robj->Is_Hidden()`, object is marked invisible.
4.  **Frustum Culling**: `bool isVisible = !camera->Cull_Sphere(robj->Get_Bounding_Sphere())`.
    *   **Frustum Normals**: The 6 planes of the frustum are oriented with normals pointing **OUTWARD** (defined in `FrustumClass::Init`).
    *   **Overlap Mapping**: Vegas math uses a specific mapping for `CollisionMath::OverlapType`:
        *   `POS` (0x01) == `OUTSIDE` (Positive half-space relative to outward normal).
        *   `NEG` (0x02) == `INSIDE` (Negative half-space, inside the frustum).
    *   **Logic**: An object is rejected (`OUTSIDE`) if its bounding sphere is entirely in the `POS` half-space of *any* frustum plane.
5.  **Gameplay Visibility**:
    *   `draw->isDrawableEffectivelyHidden()`: Check for stealth/special states.
    *   `draw->getFullyObscuredByShroud()`: Check if under fog of war.
6.  **Binning (Categorization)**:
    *   **Translucent**: Added to `m_translucentObjectsBuffer`.
    *   **Occluders**: Structures added to `m_potentialOccluders`.
    *   **Occludees**: Units/Objects that can be hidden behind buildings (`m_potentialOccludees`).
    *   **Normal**: Everything else added to `m_nonOccludersOrOccludee`.

## Visibility States & Bits (`RenderObjClass`)

The final visibility is a bitwise combination of states defined in `Core/Libraries/Source/WWVegas/WW3D2/rendobj.h`:

*   `IS_VISIBLE` (0x100): Set by `Visibility_Check` if the object's sphere is inside the camera frustum.
*   `IS_NOT_HIDDEN` (0x200): Set by `Set_Hidden(false)`. Represents gameplay-level visibility (e.g., script-hidden).
*   `IS_NOT_ANIMATION_HIDDEN` (0x400): Managed by the animation system.
*   **`IS_REALLY_VISIBLE`**: A macro combining all three: `IS_VISIBLE | IS_NOT_HIDDEN | IS_NOT_ANIMATION_HIDDEN`.

**Logic Flow**:
1.  `Visibility_Check` sets/clears the `IS_VISIBLE` bit based on frustum.
2.  `Customized_Render` calls `robj->Is_Really_Visible()`.
3.  If any bit is missing (e.g., object is in frustum but hidden by script), it won't be rendered.

## Hierarchical Culling (AABTree)

Before the per-object `Visibility_Check`, the engine uses an `AABTreeCullSystemClass` to reject entire branches of objects.
*   Uses `AAPlaneClass` (Axis-Aligned Planes) for fast intersection tests.
*   *Bug Note*: Corrected `AAPlaneClass` sphere overlap test directly improves AABTree culling accuracy.

## Shadow Lifecycle

Shadows are handled in `W3DScene::Flush` (before rendering objects) and during `renderOneObject`.

1.  **`PrepareShadows()`**: Called at the start of `Flush`.
2.  **`W3DVolumetricShadow::Update()`**: 
    *   Determines if shadow needs rebuild (position/light change).
    *   Checks if shadow center is within the "visible universe" bounding box.
3.  **`W3DVolumetricShadowManager::renderShadows()`**:
    *   Sets up Stencil states for shadow volumes.
    *   Renders volumes into Stencil buffer (Z-Pass/Z-Fail algorithms).
    *   Masks out areas with occluded player colors (to avoid drawing shadows on "ghost" silhouettes).

## Null Safety During Shell Phase (macOS-specific)

On macOS, several game globals (`TheGameLogic`, `TheInGameUI`, `TheTacticalView`, `TheScriptEngine`, `TheTerrainVisual`) may not be initialized when the shell/menu UI begins processing events. Additionally, `TheTerrainVisual` is explicitly set to `nullptr` after its `init()` throws `ERROR_BUG` (memory pool issue with `HeightMapRenderObjClass`).

### Affected Call Paths (Fixed)

| Crash Location | Root Cause | Fix |
|:---|:---|:---|
| `W3DView::getPickRay` | `m_3DCamera` null | Added null check, return zero ray |
| `W3DView::getAxisAlignedViewRegion` | UB `if (this == nullptr)` optimized away | Removed UB check; `getPickRay` now safe |
| `InGameUI::areTooltipsDisabled` | `TheGameLogic->getFrame()` with null `TheGameLogic` | Guard returns `FALSE` |
| `ControlBar::showBuildTooltipLayout` | `TheInGameUI` / `TheScriptEngine` null | Guard returns early |
| `ControlBarPopupDescriptionUpdateFunc` | `TheControlBar` / `TheScriptEngine` null | Guard returns early |
| `LookAtTranslator::translateGameMessage` | `TheGameLogic` / `TheTacticalView` null in `MSG_FRAME_TICK` | Guard returns `disp` |
| `Mouse::drawTooltip` | `TheScriptEngine->getFade()` with null `TheScriptEngine` | Guard skips fade check |
| `AudioManager::shouldPlayLocally` | `ThePlayerList->getLocalPlayer()` with null `ThePlayerList` | Guard returns `TRUE` (UI sounds play) |
| `MainMenuSystem` (all `signalUIInteract` calls) | `TheScriptEngine` null during shell phase | Added `if (TheScriptEngine)` guards |
| `MainMenuUpdate` | `TheGameLogic->isInGame()` with null `TheGameLogic` | Added null guard |
| `OptionsMenuInit` (scroll anchors) | `TheInGameUI->getDrawRMBScrollAnchor()` with null `TheInGameUI` | Added `if (TheInGameUI)` guards |
| `OptionsMenuInit` (in-game check) | `TheGameLogic->isInGame()` with null `TheGameLogic` | Added `if (TheGameLogic &&)` guard |
| `OptionsMenuInit` (`WinAdvancedDisplay`) | `WinAdvancedDisplay->winHide()` with null window | Added null guard |
| `saveOptions` (scroll anchors, refresh) | `TheInGameUI->...` / `TheRecorder` / `TheLookAtTranslator` null | Added null guards |
| `OptionsMenuSystem` (accept path) | `TheGameLogic->isInGame()` with null `TheGameLogic` | Added `!TheGameLogic \|\|` guard |
| `InGameUI::reset` (TheControlBar) | `TheControlBar->reset()` with null `TheControlBar` | Added `if (TheControlBar)` guard |
| `InGameUI::reset` (TheTacticalView) | `TheTacticalView->setDefaultView()` with null `TheTacticalView` | Added `if (TheTacticalView)` guard |
| `InGameUI::destroyPlacementIcons` | `TheTerrainVisual->removeFactionBibDrawable/removeAllBibs()` with null `TheTerrainVisual` | Added null guards |

### TerrainVisual Init Exception Flow

```
GameEngine::init()
  └→ GameClient::init()            (via initSubsystem)
       └→ createTerrainVisual()     → new W3DTerrainVisual()
           └→ W3DTerrainVisual::init()
               └→ NEW_REF(HeightMapRenderObjClass, ())
                   └→ ASSERTION FAILURE: "operator new should not be called"
                   └→ throw ERROR_BUG (0xDEAD0001)
       └→ CAUGHT in try-catch → TheTerrainVisual = nullptr
       └→ init() continues normally
  └→ initSubsystem(TheAI)         ← NOW REACHED ✅
  └→ initSubsystem(TheGameLogic)  ← NOW REACHED ✅
```

### Pattern

When adding new code that accesses game-phase globals during rendering or message processing, always guard with:
```cpp
if (!TheGameLogic || !TheInGameUI) return;
```

## Mouse Input (macOS-specific)

### Coordinate System
`[event locationInWindow]` returns coordinates in the content view's coordinate space. For borderless/fullscreen windows, these are 1:1 with the game coordinate system (after Y-flip). No `convertPoint:fromView:nil` conversion is needed.

### 2x Scaling Bug (Fixed)
**Root cause:** `Mouse::reset()` sets `m_inputMovesAbsolute = FALSE`, putting the mouse in relative mode. In relative mode, `moveMouse()` does `pos += coords`. Since `StdMouse::addEvent()` also sets `m_currMouse.pos = coords`, the position was doubled (`coords + coords = 2x`).

**Fix:** `StdMouse::reset()` now restores `m_inputMovesAbsolute = TRUE` after calling `Mouse::reset()`. In absolute mode, `moveMouse()` does `pos = coords` — no doubling.
