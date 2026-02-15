# � CRITICAL AGENT PROTOCOL
> **MANDATORY:** Before every response, the Agent MUST verify compliance with the "Communication Preferences" and "Hard Constraints" sections below. Failure to communicate in the specified language or follow these rules is a project-level failure.

---

## 🌎 Communication Preferences
- **Language:** **Russian/Русский** (Все отчеты и пояснения для пользователя должны быть на русском языке).
- **Code & Docs:** **English** (All code comments, internal documentation, and technical notes in files must be in English).
- **Style:** Technical, concise, results-oriented.

---

## 📍 Project Status: macOS Port (ARM64)
**Current Phase:** Phase 4: 3D Bridge (W3D) (ACTIVE)
**Current Goal:** Implement Uniform Buffers and Matrix Transformation Bridge
**Build Status:** ✅ **SUCCEEDED** (Executable linked with 916+ objects)
**Known Issues:**
- **Memory Manager Stabilization**: Resolved a critical crash during static initialization by ensuring `preMainInitMemoryManager()` runs before any static constructors and correctly handles global `operator new/delete` fallbacks.
- **Header Conflicts**: Fully resolved globally via `MacOSSystemProtection.h` and `new.h` shim.
- **Metal Backend (ACTIVE)**: Encountering memory ownership crashes in Metal driver when it calls overridden `delete`.

### 🛠 Practical Build & Verification
**Target:** `build/macos/GeneralsExecutable`
**Phase 3 Progress:**
- [x] macOS User Data Directory setup
- [x] Metal rendering pipeline (Textured Quad)
- [x] **ZERO HOUR ASSET MIGRATION** (Copied from /Applications/CnC-ZH.app)
- [x] **BIG ARCHIVE EXTRACTION** (Full INI and Window definitions extracted)
- [x] **GameEngine Compilation:** Linked 918 source files successfully.
- [x] **Subsystem Init:** `Eva::init()` and `ScienceStore::init()` now complete correctly.
- [x] **Memory Management:** **STABILIZED**: Static init order fixed; global `new`/`delete` safe for macOS.
- [ ] **Metal Subsystem Debugging**: Investigating memory ownership issues in `MTLCompiler` bridge. (In Progress)

---

## 🏗 Subsystem Porting Progress
| Subsystem | Status | Technical Debt | Notes |
| :--- | :---: | :--- | :--- |
| **Core Utilities** | 🟢 | Low | `AsciiString`, `ThreadClass`, `MemoryPool` implemented. |
| **GameEngine Core** |  | Low | **SUCCESS**: `GameEngine::init()` completed, loop reached. |
| **GUI/Windowing** | � | Low | **SUCCESS**: `GameTextManager` fixed (CSF_LABEL tag corrected). Menu text is now visible. |
| **Filesystem** | 🟢 | Low | `ArchiveFileSystem` и `LocalFileSystem` работают стабильно. Поддержка `.big` и `.csf`. |
| **Audio (TheAudio)** | 🔴 | High | Только заглушки. |
| **Graphics (W3D)** | 🟡 | Critical | **ANALYSIS**: Using MSL Uber-shader for fixed pipeline emulation. |
| **Input (Mouse/KB)** | 🟡 | Medium | Cocoa events mapped to engine. |
| **Networking (GameSpy)** | 🟡 | Medium | **LINKED**: Real GameSpy SDK linked. |

---

## 🔄 Loop Counters
| Task Area | Iterations | Last Issue |
| :--- | :---: | :--- |
| **Stubs/Linking** | 35+ | **Dependency Resolution**: Added more files to resolve symbols for `ControlBar`, `GUI`, and `ObjectUpdating`. |
| **Runtime Init** | 10+ | **Eva.ini Crash**: Resolved "Unknown block token 'Priority'" by correctly skipping already parsed blocks. |

---

## 🗺️ Roadmap & Milestones

### ✅ Phase 1: Core Foundation (COMPLETED)
- INI-based Registry, `CriticalSection` (pthread), standard types in `win_compat.h`.

### ✅ Phase 2: Engine Integration (COMPLETED)
- **Goal reached:** Executable linked with real GameLogic, Networking, and Engine Core.
- **Recent:** Resolved massive duplicate symbol conflicts and integrated 914 source files.
- **Next:** Runtime debugging of `GameEngine::init()` and data file mounting.

### 🚀 Phase 3: Logistics & I/O (COMPLETED)
- [x] **Data Access Audit:** Проверен доступ к файлам в CWD. Настроен корректный путь для пользовательских данных в `~/Library/Application Support/`.
- [x] **Filesystem Implementation:** Реализация `ArchiveFileSystem` и `LocalFileSystem` (macOS/POSIX). **УСПЕШНО:** Поддержка `.big` архивов, корректный endianness на ARM64, регистронезависимость.
- [x] **Native Input:** Начать реализацию `Mouse` и `Keyboard` (macOS events). **УСПЕШНО:** Добавлен Cocoa event pump и обработка ввода в `stubs.mm`, `StdMouse.mm` и `StdKeyboard.mm`.

### 🏔 Phase 4: Media & Rendering (PLANNED)
- [ ] **CoreAudio Bridge:** Реализация `AudioManager`.
- [x] **Native Window:** Создание `NSWindow` и `NSView`. **УСПЕШНО:** Окно создается и отображается при запуске.
- [ ] **Metal/OpenGL Bridge:** Портирование графики `W3D`.

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

## 🔍 Discovery: Graphics Solutions (Research Feb 2026)
### 1. Fighter19 `sdl-gpu` Branch
- **Key File:** `GeneralsMD/Code/Libraries/Source/WWVegas/WW3D2/sdl3wrapper.cpp`
- **Solution:** Implements `IRenderDevice` interface using SDL3's new GPU API.
- **Status:** Advanced. Includes `.glsl` shaders for 2D, shadows, and terrain. High potential for reuse in Metal bridge.

### 2. Fighter19 `emscripten` / `gles` Branches (FRESH: Feb 2026)
- **Key Concept:** Uses **WineD3D** translated via **GL4ES** to target OpenGL ES.
- **Insight:** "Initialize and use WineD3D8 via GL4ES" (Commit 349920c2).
- **Pro:** Reuses legacy DX8 code with minimal changes. Possible path for rapid prototyping.

### 3. GeneralsX `feature/dxvk-native-migration` (FRESH: Feb 2026)
- **Status:** Massive refactor (3000+ files). Moving towards modern backend (Vulkan/Vulkan-native).
- **Note:** Very active development, overlaps with our goals but focuses on generalized multiplatform support.

### 4. GeneralsX `old-multiplatform-attempt` (Historical)
- **Documentation:** `docs/WORKDIR/phases/legacy/metal_port/`
- **Confirmation:** Confirmed texture loading from .big to Metal was successful (Phase 37.1).
- **Lessons:** Detailed diagnostic logs on blank screens and viewport issues.

---

## �️ Graphics Porting Roadmap (Metal Bridge)

Данный план основан на успешных практиках `fighter19` (архитектура) и `GeneralsX` (опыт работы с Metal).

### Phase 1: Foundation (Current)
- [x] **Interface Definition**: Портировать `IRenderDevice` и `ITexture` из `fighter19/sdl-gpu`. Это даст нам чистый мост между W3D и Metal.
- [x] **Metal Graphics Context**: Улучшить `MacOSRenderer.mm` для поддержки очередей команд (Command Queues) и управления жизненным циклом ресурсов.

### Phase 2: 2D & UI Rendering
- [x] **Coordinate Projection**: Реализовать проекцию DirectX (0,0 в левом верхнем углу) в Metal NDC.
- [x] **Primitive Drawing**: Реализовать отрисовку линий и прямоугольников (интерфейс `DrawIndexed` готов).
- [x] **Text Engine**: Подключить `MacOSDisplayString` к нашему Metal-рендереру (используется debug font atlas).

### Phase 3: Texture Pipeline (COMPLETED)
- [x] **Asset Extraction**: Использовать логику из `GeneralsX` для извлечения `.tga` и `.dds` из `.big` архивов.
- [x] **Texture Cache**: Реализовать кэширование `MTLTexture` для минимизации переключений контекста.
- [x] **Sampler States**: Настроить фильтрацию (Linear/Point) в соответствии с настройками W3D.

### Phase 4: 3D Bridge (W3D) (ACTIVE)
- [x] **Shader Integration**: Реализован Uber-shader (MSL) для эмуляции фиксированного конвейера W3D. Поддержка текстурирования, альфа-теста, градиентов и тумана на основе битовой маски `ShaderClass`.
- [x] **Matrix/Shader Bridge**: Полноценный проброс матриц (World/View/Projection) и состояний шейдера из W3D в Metal-рендерер.
- [x] **Buffer Management**: Реализованы `MacOSVertexBufferClass` и `MacOSIndexBufferClass` с поддержкой `MTLBuffer`.
- [x] **Linkage Fix**: Успешное разрешение символов `IRenderDevice` и `GetRenderDevice` в глобальном пространстве имен.

### Phase 5: Engine Foundation (COMPLETED)
- [x] **Initialization Loop**: `GameEngine::init` успешно проходит инициализацию всех подсистем (`TheGlobalData`, `TheGameClient` и т.д.).
- [x] **Native Window**: Создание нативного окна macOS через Objective-C бридж (`MacOS_CreateWindow`).
- [x] **Main Loop**: Успешный вход в основной цикл `GameClient::update` и обработка кадров.
- [x] **Display Bridge**: Связка `TheDisplay` с Metal рендерером работает, окно отображает фоновый цвет (magenta).
- [x] **Resource Migration**: Замена текущих ресурсов (`Data_Extracted`) на улучшенные из `GeneralsGamePatch`.

### Phase 6: UI & Interaction (ACTIVE)
- [x] **MacOSGameClient Completion**: Успешно скомпилирован `MacOSGameClient.mm` и реализованы все необходимые интерфейсные методы.
- [x] **Linker Resolution**: Все неопределенные символы для `MacOSGameClient` разрешены.
- [x] **Header Fixes**: Устранены ошибки в `MEMORY_POOL_GLUE` путем корректной настройки `RTS_DEBUG`.
- [x] **Primitive Drawing**: `drawFillRect` и линии работают в `MacOSDisplay`.
- [x] **Image Rendering**: `drawImage` для UI элементов реализовано через Metal bridge.
- [x] **UI Memory Pools**: Финализирована инициализация пулов памяти для `MacOSDisplayString` и `MacOSGameWindow`.
- [x] **Input Bridge**: `NSEvent` полностью проброшен через `StdMouse` и `StdKeyboard`.
- [x] **Final Linkage**: Исполняемый файл успешно собран (Linked with 925+ objects).



---

## 📝 Key decisions (Architecture)
1. **W3D Bypass:** we decided to return `nullptr` from all `Win32GameEngine` factory methods for W3D.
   - *Rationale:* Allows us to test logic, networking, and memory without being blocked by the massive graphics task.
2. **Resource Strategy:** Transitioning to `GeneralsGamePatch` as the primary source of truth for INI, Window definitions, and Assets.
   - *Rationale:* Fixes numerous legacy bugs in scripts and UI definitions that would otherwise cause crashes (like the Eva.ini issue).
3. **WinMain Integration:** Main entry point remains `WinMain.cpp`, adapted for macOS entry via `win_compat.h`.
3. **Single Window Architecture:** The game will use a single `NSWindow` for both Game World and UI rendering via a unified graphics bridge (Metal/OpenGL).
4. **INI Skip Logic:** Implemented `ini->skipBlock()` for already existing entities. This is critical for macOS/Linux where loose files (`Data_Extracted`) might overlap with internal engine data, causing parser desync.

---

## ⚠️ Hard Constraints for the Agent
1. **No Placeholders:** Generate images/icons if needed via tools.
2. **Always Verify Build:** Never claim a fix works without running `./build_macos.sh`.
3. **Language Consistency:** **RUSSIAN ONLY** for user-facing text.
4. **Platform Parity:** All changes MUST be guarded by preprocessor macros to ensure Windows build compatibility is NEVER broken. No macOS-specific code should leak into Windows builds.
5. **Architectural Isolation:** **DO NOT** refactor or change the original engine architecture. The macOS implementation must be an *integration layer*.
6. **Minimize Core Changes:** Avoid editing original source files in `Core/` and `GeneralsMD/` unless strictly necessary for basic compilation (e.g., adding `#ifdef _WIN32`). Prefer placing macOS-specific logic and stubs in `Platform/MacOS/`.

---

## 📝 Backlog & Optimizations
- [ ] **Metal Ring Buffer (Triple Buffering)**: Реализовать циклическое переключение буферов (или один большой Ring Buffer) для динамических вертексных и индексных буферов, чтобы избежать data corruption и stalls при сбросе оффсета в начале кадра.
- [ ] **Command Encoder Reuse**: Оптимизировать создание `MTLRenderCommandEncoder` в `DrawIndexed` (сейчас создается новый энкодер на каждый вызов отрисовки с `MTLLoadActionLoad`).
- [ ] **Shader Translation Pipeline**: Автоматизировать конвертацию legacy шейдеров в MSL.
- [ ] **Alpha Blending States**: Реализовать маппинг Render States из W3D в Metal Pipeline States.

