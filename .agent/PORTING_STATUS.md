**Goal:** Fix 3D Rendering & Texture Fidelity.
**Build:** ✅ **SUCCEEDED** (932 objects).

---

## 🚀 Status Quo
- [x] **Subsystems:** Memory, Filesystem (.big), INI, Input, Window.
- [x] **Graphics:** Metal Bridge, SW DDS Decoding (DXT1/5), Cinematic Vignette, Navy Clear Color.
- [x] **3D Infrastructure:** Real `W3DView` integrated (currently stubbed for UI stability).

---

## 🔴 ACTIVE ISSUES
- **Gray Boxes UI:** UI textures (DDS/TGA) bind correctly but appear as gray blocks. Likely Alpha Blending or Color Modulation issue.
- **W3DView SIGSEGV:** Crash in `W3DView::draw()` due to `doRender` calling on uninitialized scene components.
- **Fonts/Strings:** `MacOSFontLibrary` and `DisplayStringManager` are stubs.

---

## 🏗 SHORT-TERM BACKLOG
1. **Debug Texture Rendering:** Setup Alpha Blending in `MacOSRenderer.mm` and verify pixel format swizzling.
2. **W3D Scene Initialization:** Properly setup `W3DDisplay::m_3DScene` to enable the rotating shell map.
3. **Audio:** Implement `MacOSAudioManager.mm`.


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

## 📝 DECISIONS & CONSTRAINTS
- **Hybrid W3D:** Lexicon/GUI callbaks are real; renderer is Metal bridge.
- **Precompiled Headers:** `PreRTS.h` is mandatory for all MacOS port files.
- **RA_MACOS:** Use macros to isolate porting logic.
