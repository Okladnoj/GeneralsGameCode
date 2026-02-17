**Goal:** Fix UI Rendering & Enable Main Menu.
**Build:** ✅ **SUCCEEDED** (936 objects).

---

## 🚀 Status Quo
- [x] **Subsystems:** Memory, Filesystem (.big), INI, Input, Window.
- [x] **Audio Engine:** ✅ `MacOSAudioManager` via `AVAudioEngine`. `.big` extraction + local caching.
- [x] **Graphics:** Metal Bridge, SW DDS Decoding (DXT1/5). Lazy Metal buffer sync.
- [x] **Initialization:** ✅ D3D8 wrapper (mocks). Fixed startup crash in `W3DBridgeBuffer`.
- [x] **3D Infrastructure:** `W3DView` integrated; `RTS3DScene` pools initialized.
- [x] **Input Handling:** ✅ Mouse 1:1 absolute coordinates. Fixed 2x scaling bug (`m_inputMovesAbsolute`).
- [x] **Dynamic Window Size:** ✅ `MacOS_GetScreenSize()` replaces hardcoded 800×600.
- [x] **Stable Runtime:** ✅ Game window runs without crashing. Shell/menu loop ticks stably.
- [x] **Main Menu:** ✅ **FULLY VISIBLE**. All buttons rendered with text and styling.
- [x] **Options Menu:** ✅ Opens without crash. Null guards for `TheInGameUI`, `TheGameLogic`, `TheRecorder`, `TheLookAtTranslator`.
- [x] **UI Pipeline:** ✅ `DrawTexturedRect` → `m_UIPS` (24-byte stride). Fixed diagonal artifacts.
- [x] **Screenshot Module:** ✅ `MacOSScreenshot.h/.mm` — captures Metal drawable to PNG.
- [x] **Textures:** ✅ TGA loading works. Logo texture renders correctly.

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
- **3D World Rendering:** Non-functional (needs full 3D pipeline).

---

## 🏗 SHORT-TERM BACKLOG
1. **3D Rendering Pipeline:** W3D vertex formats, depth/stencil, Metal shaders for lighting. Background/terrain rendering.
2. **Texture Loading for all UI:** Ensure remaining `.tga`/`.dds` textures for buttons/backgrounds load correctly.
3. **3D Audio & Music:** `AVAudioEnvironmentNode` for positional audio. Music track playback.
4. **Exit Crash:** `ObjectPoolClass` destructor Signal 11 during cleanup.

---

## ⚠️ BUILD NOTES
- **Clean build recommended** after source changes: `rm -rf build/ && bash build_macos.sh`.
- **936 objects** total.

---

##  DECISIONS & CONSTRAINTS
- **Hybrid W3D:** Lexicon/GUI callbacks are real; renderer is Metal bridge.
- **Precompiled Headers:** `PreRTS.h` mandatory for all MacOS port files.
- **RA_MACOS:** Macros to isolate porting logic.
- **ShellMapOn = No:** Disabled 3D shell map — requires full 3D pipeline.
- **Null-Safety Pattern:** All globals (`TheGameLogic`, `TheScriptEngine`, `ThePlayerList`, `TheInGameUI`, `TheRecorder`, `TheLookAtTranslator`, etc.) need null checks during shell/menu phase.
- **Mouse Input Mode:** `StdMouse` uses `m_inputMovesAbsolute = TRUE` (absolute coordinates). `Mouse::reset()` overrides to FALSE — `StdMouse::reset()` must restore to TRUE.
- **Mouse Coordinates:** Direct `locationInWindow` coordinates are 1:1 with content view in borderless/fullscreen window.
