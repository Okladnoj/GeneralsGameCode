# V3: Stub Implementation Plan — Systematic Completion

## ⚠️ ОБЯЗАТЕЛЬНО прочитать перед началом работы:
- **`Platform/MacOS/docs/STUBS_AUDIT.md`** — полный аудит всех стабов с текущим статусом
- **`Platform/MacOS/docs/RENDERING.md`** — спецификация render pipeline
- **`.agent/image_zh_origin.png`** — оригинальный вид shell map (reference)
- **`.agent/workflows/build-and-run.md`** — как собирать, запускать и делать скриншот

---

## Текущее состояние (2026-02-25, 20:45)

### ✅ Работает
- **Terrain** — текстуры, песок, горы, камни ✅
- **3D модели** — корабль, камни, деревья ✅
- **Вода** — анимированная ✅
- **UI** — кнопки меню, иконки, текст ✅
- **Вертолёт** — стабильно рендерится ✅
- **Огонь/вспышки** — видны ✅
- **Следы взрывов (scorch)** — видны на берегу! ✅ (addScorch → TheTerrainRenderObject)
- **W3D Shader Pipeline** — terrain shaders, setShroudTex, все фильтры ✅
- **Render-to-texture (RTT)** — offscreen render target ✅
- **Screen Filters** — BW, MotionBlur, CrossFade ✅
- **SetGammaRamp** — через CGSetDisplayTransferByTable ✅
- **SetLOD/GetLOD** — хранение LOD значения ✅
- **W3DSnowManager** — снег через Core ✅
- **Mipmap генерация** — D3DXFilterTexture через Metal blit encoder ✅
- **Cursor (2D)** — TGA текстуры загружаются через WW3DAssetManager, анимация, 8 направлений скролла ✅
- **takeScreenShot** — наследуется от W3DDisplay ✅
- **toggleMovieCapture** — наследуется от W3DDisplay ✅

### ⚠️ Известные визуальные проблемы
- **Terrain blend** — переходы между текстурами тёмные/неполные
- **Деревья** — частично отсутствуют
- **Лазеры/трейсеры** — не видны (stub)
- **Cursor RM_W3D** — 3D модели курсоров (зелёные прицелы, красные круги атаки) не реализованы. Текущий 2D fallback покрывает базовый геймплей
- **Краш при выходе** — `freeDisplayString` SIGSEGV при shutdown (не критичен)

### Статистика стабов
- **144 ✅ реализовано** / **215 ⚠️ safe stubs** / **0 ❌ dangerous** / **0 🔴 critical**

### 🔑 Ключевые решения сессии
1. **`MacOSW3DShaderManager.mm` УДАЛЁН** — 60+ no-op стабов перекрывали Core
2. **`addScorch()` реализован** — делегация к `TheTerrainRenderObject->addScorch()`
3. **`W3DSnowManager` из Core** — заменил пустой `MacOSSnowManager`
4. **`MacOSTerrainVisual` удалён** — `W3DTerrainVisual` используется напрямую

---

## 🎯 ОСТАВШИЕСЯ СТАБЫ

### Из 215 оставшихся:
- **~180** — GameSpy/Network/WWDownload/CDManager — **НЕ НУЖНЫ** для offline
- **~30** — Cosmetic/Windows-shim — работают как есть
- **~5** — Можно реализовать для полноты

---

## Фаза 1: Оставшиеся gameplay стабы (✅ ВЫПОЛНЕНА)

| Стаб | Статус |
|------|--------|
| `addScorch()` | ✅ Делегация к TheTerrainRenderObject |
| `createSnowManager()` | ✅ W3DSnowManager из Core |
| `notifyTerrainObjectMoved()` | ✅ Safe no-op |
| `releaseShadows/allocateShadows` | ✅ Делегация к GameClient |
| `createRayEffectByTemplate()` | ⚠️ Logged stub |
| `setTeamColor/setTextureLOD` | ⚠️ Logged stubs |

---

## Фаза 2: Cosmetic — Не критичные, но полезные

### 2.1 `MacOSDisplay::takeScreenShot()` — ✅ ГОТОВО
- Убран пустой override → используется W3DDisplay::takeScreenShot()

### 2.2 `StdMouse::draw()` — ✅ ГОТОВО (2D), ⚠️ RM_W3D TODO
- **Реализовано:** TGA текстуры через WW3DAssetManager + Render2DClass
- **Поддержка:** многокадровая анимация (FPS), 8-direction scroll, hotspot
- **TODO:** RM_W3D 3D модели курсоров (targeting crosshairs, attack circles, move arrows). Требует: W3D model loading, ortho camera, WW3D::Render() per-frame

### 2.3 `MetalInterface8::EnumAdapterModes()` — Список разрешений
**Файл:** `Metal/MetalInterface8.mm`
**Статус:** ⚠️ Returns 800×600 only
**Реализация:**
- Query `NSScreen.mainScreen.frame`

### 2.4 `StdMouse::capture() / releaseCapture()` — Захват мыши
**Файл:** `Main/StdMouse.mm`
**Статус:** ⚠️ Empty
**Реализация:**
- `CGAssociateMouseAndMouseCursorPosition(false/true)`

### 2.5 `MacOSAudioManager::getDevice() / getHandleForBink()` — Audio
**Файл:** `Audio/MacOSAudioManager.mm`
**Статус:** ⚠️ Returns nullptr
**Реализация:**
- Dummy handle для Bink video audio

### 2.6 Git Info stubs
**Файл:** `Stubs/GitInfoStubs.cpp`
**Реализация:**
- CMake `execute_process(COMMAND git rev-parse HEAD ...)`

---

## 🐛 Известные баги

### Краш при выходе (SIGSEGV)
```
MacOSDisplayStringManager::freeDisplayString → SIGSEGV
W3DDisplay::~W3DDisplay → MacOSDisplay::~MacOSDisplay
```
**Причина:** DisplayString уже освобождён или невалидный указатель при деструкции.
**Импакт:** Только при выходе, не влияет на gameplay.

---

## 🚫 НЕ НУЖНО реализовывать (~180 стабов)

| Категория | Кол-во | Почему |
|-----------|--------|--------|
| GameSpy/Network | ~170 | Онлайн мультиплеер |
| WWDownload/Cftp | ~17 | Скачивание патчей |
| CDManager | 3 | CD проверка обходится |
| windows.h shims | 8 | Маркеры, callers проверяют |
| IME/WebBrowser/Worker | ~11 | Не используются |
| MacOSGadgetDraw | 10 | W3D рисует |

---

## Ключевые файлы

| Файл | Назначение |
|------|-----------|
| `MacOSShaders.metal` | Fragment shader — TSS pipeline, fog, alpha |
| `MetalDevice8.mm` | Metal pipeline — draw calls, textures, PSO cache |
| `MetalTexture8.mm` | Texture creation, format conversion |
| `D3DXStubs.mm` | D3DX helpers, mipmap generation |
| `MacOSGameClient.mm` | Game client — factories, addScorch, snow |
| `dx8wrapper.cpp` | Render state, texture caching |
| `STUBS_AUDIT.md` | Full audit — **update after each change** |

## ⚠️ Правила
- `printf` + `fflush(stdout)` для логов
- Тестировать: `sh build_run_mac.sh --screenshot`
- **Проверять Core на готовые реализации** (`nm *.o | grep symbol`)
- После верификации: обновить `STUBS_AUDIT.md` (⚠️ → ✅)
- Коммитить после каждой фазы
