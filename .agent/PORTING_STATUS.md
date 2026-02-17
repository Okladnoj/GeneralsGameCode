**Goal:** Fix 3D Rendering & Texture Fidelity.![alt text](image.png)
**Build:** ✅ **SUCCEEDED** (932 objects).

---

## 🚀 Status Quo
- [x] **Subsystems:** Memory (fixed pool race condition), Filesystem (.big), INI, Input, Window.
- [x] **Audio Engine:** ✅ **WORKING (UI & Sounds)**. `MacOSAudioManager` implemented using `AVAudioEngine`. Supports extraction from `.big` archives with local caching.
- [x] **Graphics:** Metal Bridge, SW DDS Decoding (DXT1/5), Cinematic Vignette, Navy Clear Color.
- [x] **Initialization:** ✅ **VERIFIED**. Direct3D 8 wrapper (mocks) correctly implemented, linked, and initialized on macOS. `IsInitted` flag is set, and mock devices are successfully created.
- [x] **3D Infrastructure:** Real `W3DView` integrated; `RTS3DScene` pools explicitly initialized.
- [x] **Input Handling:** ✅ **VERIFIED**. MacOS events are correctly translated and routed. Menu buttons and sliders respond to clicks with 100% accuracy.

---

## 🔴 ACTIVE ISSUES
- [x] **Fonts & Text:** ✅ **WORKING**. Dynamic strings and menu labels are rendered correctly.
- [x] **Gray Boxes UI:** ✅ **VERIFIED**. Intentional debug markers from the original engine, not a rendering bug.
- **3D World Rendering:** Currently non-functional. **Cause identified:** Crash in `Render_FVF_Category_Container_List`. The engine tries to iterate through empty or malformed FVT category lists when using mock devices.
- **Exit Crash:** `Signal 11` in `ObjectPoolClass<MultiListNodeClass>` destructor during application shutdown.
- **Lint Noise:** Massive amount of false/real lint errors in W3D files.

---

## 🏗 SHORT-TERM BACKLOG
1. **3D Rendering Pipeline:** Implement multi-attribute vertex formats (Normals, Tangents), enable Depth/Stencil buffer, and expand Metal shaders for basic lighting.
2. **3D Positional Audio & Music:** Transition mixer to `AVAudioEnvironmentNode` and implement music track playback.
3. **Fix Exit Crash:** Debug `ObjectPoolClass` destructor `Signal 11` during cleanup.
4. **Lint Cleanup:** Address critical compilation warnings in `MacOSRenderDevice_Internal.h`.


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
