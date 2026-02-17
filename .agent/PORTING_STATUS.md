**Goal:** Fix UI Rendering & Enable Main Menu.
**Build:** ✅ **SUCCEEDED** (936 objects, clean build OK).

---

## 🚀 Status Quo
- [x] **Subsystems:** Memory, Filesystem (.big), INI, Input, Window.
- [x] **Audio Engine:** ✅ `MacOSAudioManager` via `AVAudioEngine`. `.big` extraction + local caching.
- [x] **Graphics:** Metal Backend Stage 0 (Direct DX8 Adapter). Stable BeginScene/Clear/Present.
- [x] **SW DDS Decoding:** DXT1/5 textures handled. Lazy Metal buffer sync.
- [x] **Initialization:** ✅ All critical subsystems init (TheGameClient, TheGameLogic, TheAI, ThePlayerList).
- [x] **3D Infrastructure:** `W3DView` integrated; `RTS3DScene` pools initialized.
- [x] **Input Handling:** ✅ Mouse 1:1 absolute coordinates. Fixed 2x scaling bug (`m_inputMovesAbsolute`).
- [x] **Dynamic Window Size:** ✅ `MacOS_GetScreenSize()` replaces hardcoded 800×600.
- [x] **Stable Runtime:** ✅ Game window runs without crashing. Shell/menu loop ticks stably (1500+ heartbeats).
- [x] **Main Menu:** ✅ **FULLY VISIBLE**. All buttons rendered with text and styling.
- [x] **Options Menu:** ✅ Opens without crash. Null guards for `TheInGameUI`, `TheGameLogic`, `TheRecorder`, `TheLookAtTranslator`.
- [x] **UI Pipeline:** ✅ `DrawTexturedRect` → `m_UIPS` (24-byte stride). Fixed diagonal artifacts.
- [x] **Screenshot Module:** ✅ `MacOSScreenshot.h/.mm` — captures Metal drawable to PNG.
- [x] **Textures:** ✅ TGA loading works. Logo texture renders correctly.
- [x] **TerrainVisual Init:** ✅ Gracefully caught `ERROR_BUG` (0xDEAD0001) from `W3DTerrainVisual` init. Allows rest of init to proceed.
- [x] **GameLogic Init:** ✅ `TheGameLogic` now properly initializes (was `nullptr` before due to exception in TheTerrainVisual).

---

## 🔴 ACTIVE ISSUES
- [x] ✅ UI vertex descriptor mismatch → fixed.
- [x] ✅ Shell not loading main menu → `ShellMapOn=No`.
- [x] ✅ Signal 11 in `MainMenuUpdate` → null `TheGameLogic` guard.
- [x] ✅ Signal 11 in `Mouse::drawTooltip` → null `TheScriptEngine` guard.
- [x] ✅ Signal 11 in `AudioManager::shouldPlayLocally` → null `ThePlayerList` guard.
- [x] ✅ Signal 11 in `MainMenuSystem` (`signalUIInteract`) → null `TheScriptEngine` guards (13 calls).
- [x] ✅ Mouse 2x coordinate scaling → `Mouse::reset()` set `m_inputMovesAbsolute=FALSE`; `StdMouse::reset()` now restores TRUE.
- [x] ✅ Mouse offset by title bar → direct `locationInWindow` coordinates (1:1 with content view).
- [x] ✅ Signal 11 in `OptionsMenuInit` → null guards for `TheInGameUI`, `TheGameLogic`, `WinAdvancedDisplay`, etc.
- [x] ✅ Signal 11 in `CampaignManager::setCampaign` → null `TheCampaignManager` guard in MainMenu faction buttons.
- [x] ✅ Signal 11 in `prepareCampaignGame` → null `TheCampaignManager` guard for `getCurrentMap()` and `setGameDifficulty()`.
- [x] ✅ **Text Over-Contrast** → fragment shader was forcing `alpha=1.0` (`float4(c.rgb, 1.0)`), destroying text transparency. Fixed to `c * in.color`.
- [x] ✅ **TheTerrainVisual init crash** → `W3DTerrainVisual::init()` threw `ERROR_BUG` (memory pool `operator new` assertion). Wrapped creation+init in try-catch. `TheTerrainVisual = nullptr` on failure.
- [x] ✅ **TheGameLogic was nullptr** → Root cause: exception in `TheTerrainVisual` init during `GameClient::init()` escaped to `GameEngine::init()`'s catch block, skipping `TheAI` and `TheGameLogic` init. Fixed by containing the exception.
- [x] ✅ **Signal 11 in `InGameUI::placeBuildAvailable`** → `destroyPlacementIcons()` dereferenced null `TheTerrainVisual`. Added null guards.
- [x] ✅ **Signal 11 in `InGameUI::reset`** → `TheControlBar->reset()` and `TheTacticalView->setDefaultView()` dereferenced null globals. Added null guards.
- **3D World Rendering:** Non-functional (needs full 3D pipeline).

---

## 🏗 SHORT-TERM BACKLOG
1. **Metal Backend Stage 1:** Implement vertex/index buffer binding in `MetalDevice8` and real `DrawPrimitive` calls.
2. **Root-Cause TerrainVisual ERROR_BUG:** The `NEW_REF(HeightMapRenderObjClass, ())` in `W3DTerrainVisual::init()` triggers "operator new should not be called" assertion → `throw ERROR_BUG`. Memory pool for `HeightMapRenderObjClass` may not be created, or MEMORY_POOL_GLUE isn't set up correctly for macOS.
2. **3D Rendering Pipeline:** W3D vertex formats, depth/stencil, Metal shaders for lighting. Background/terrain rendering.
3. **Texture Loading for all UI:** Ensure remaining `.tga`/`.dds` textures for buttons/backgrounds load correctly.
4. **3D Audio & Music:** `AVAudioEnvironmentNode` for positional audio. Music track playback.
5. **Exit Crash:** `ObjectPoolClass` destructor Signal 11 during cleanup.

---

## 📚 External References
| Project | GitHub Link | Local Paths | Description |
| :--- | :--- | :--- | :--- |
| **Fighter19 (Linux)** | [Fighter19/CnC_Generals](https://github.com/Fighter19/CnC_Generals_Zero_Hour) | `/Users/okji/dev/games/CnC_Generals_Zero_Hour` | Native Linux port. **Promising:** `sdl-gpu` and `gles` branches contain SDL3 GPU/GLSL implementations. |
| **TheSuperHackers** | [TheSuperHackers/Generals](https://github.com/TheSuperHackers/GeneralsGameCode) | `/Users/okji/dev/games/GeneralsX` | Modernized C++20 branch. `feature/dxvk-native-migration` is active as of Feb 2026. |
| **Aspyr Port (macOS)** | N/A | `/Applications/Command & Conquer™ Generals Zero Hour.app` | **Local Reference:** Binary only. Used for asset migration and behavioral reference. |
| **GeneralsGamePatch** | N/A | `/Users/okji/dev/games/GeneralsGamePatch` | **Complementary project:** Fixing and improving game data and assets (INI, GUI, AI, maps, models, textures, audio, localization). |
| **Local Knowledge Base** | N/A | `Platform/MacOS/ReferenceDocs/` | **Extracted Wisdom:** Collected lessons, architecture notes, and launch commands from historical branches. |

---

## ⚠️ BUILD NOTES
- **Clean build recommended** after source changes: `rm -rf build/macos/*.o && bash build_macos.sh`.
- **938 objects** total.

---

##  DECISIONS & CONSTRAINTS
- **Hybrid W3D:** Lexicon/GUI callbacks are real; renderer is Metal bridge.
- **Precompiled Headers:** `PreRTS.h` mandatory for all MacOS port files.
- **RA_MACOS:** Macros to isolate porting logic.
- **ShellMapOn = No:** Disabled 3D shell map — requires full 3D pipeline. MapCache correctly finds `maps\shellmapmd\shellmapmd.map`, but `TheGlobalData->m_shellMapOn` is set FALSE during init.
- **TheTerrainVisual = nullptr:** On macOS, terrain visual init fails and is set to null. All code accessing `TheTerrainVisual` must null-check.
- **Null-Safety Pattern:** All globals (`TheGameLogic`, `TheScriptEngine`, `ThePlayerList`, `TheInGameUI`, `TheRecorder`, `TheLookAtTranslator`, `TheControlBar`, `TheTacticalView`, `TheTerrainVisual`, etc.) need null checks during shell/menu phase.
- **Mouse Input Mode:** `StdMouse` uses `m_inputMovesAbsolute = TRUE` (absolute coordinates). `Mouse::reset()` overrides to FALSE — `StdMouse::reset()` must restore to TRUE.
- **Mouse Coordinates:** Direct `locationInWindow` coordinates are 1:1 with content view in borderless/fullscreen window.
