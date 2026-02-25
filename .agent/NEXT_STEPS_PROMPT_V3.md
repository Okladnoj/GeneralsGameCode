# V3: Stub Implementation Plan — Systematic Completion

## ⚠️ ОБЯЗАТЕЛЬНО прочитать перед началом работы:
- **`Platform/MacOS/docs/STUBS_AUDIT.md`** — полный аудит всех стабов с текущим статусом
- **`Platform/MacOS/docs/RENDERING.md`** — спецификация render pipeline
- **`.agent/image_zh_origin.png`** — оригинальный вид shell map (reference)
- **`.agent/workflows/build-and-run.md`** — как собирать, запускать и делать скриншот

---

## Текущее состояние (2026-02-25, 19:20)

### ✅ Работает
- **Terrain** — текстуры видны! Песок, горы, камни ✅ (D3DXFilterTexture fix)
- **3D модели** — корабль, камни, деревья — текстуры корректные
- **Вода** — анимированная, корректная
- **UI** — кнопки меню, иконки, текст
- **Terrain mipmap генерация** — D3DXFilterTexture реализован через Metal blit encoder
- **Вертолёт** — теперь стабильно рендерится ✅
- **Огонь/вспышки** — корабль стреляет, видны вспышки ✅
- **W3D Shader Pipeline** — terrain shaders, setShroudTex, все фильтры ✅
- **Render-to-texture (RTT)** — init создаёт offscreen render target ✅
- **Screen Filters** — BW, MotionBlur, CrossFade — все через Core ✅
- **SetGammaRamp** — через CGSetDisplayTransferByTable ✅
- **SetLOD/GetLOD** — хранение LOD значения ✅

### ⚠️ Известные визуальные проблемы
- **Terrain blend** — переходы между текстурами (blend tiles) тёмные/неполные
- **Деревья** — частично отсутствуют
- **Следы взрывов (scorch)** — не видны (stub)
- **Лазеры/трейсеры** — не видны (stub)
- **Снег** — не виден на зимних картах (stub)

### Статистика стабов
- **139 ✅ реализовано** / **216 ⚠️ safe stubs** / **0 ❌ dangerous** / **0 🔴 critical**

### 🔑 Ключевое открытие сессии
**`MacOSW3DShaderManager.mm` УДАЛЁН** — содержал 60+ no-op стабов, которые
перекрывали полностью рабочие Core реализации через link order. Удаление одного
файла разблокировало: shroud, RTT, terrain shaders, screen filters, огонь.

---

## 🎯 ОСТАВШИЕСЯ СТАБЫ

### Из 216 оставшихся:
- **~180** — GameSpy/Network/WWDownload/CDManager — **НЕ НУЖНЫ** для offline gameplay
- **~30** — Cosmetic/Windows-shim — работают как есть
- **~6** — **Реально полезные** для gameplay

---

## Фаза 1: Gameplay стабы (6 штук) — ПРИОРИТЕТ

### 1.1 `MacOSGameClient::addScorch()` — Следы взрывов
**Файл:** `Main/MacOSGameClient.mm`
**Статус:** ⚠️ No-op
**Импакт:** Визуальные следы взрывов на земле
**Реализация:**
- Делегировать к `TheTerrainRenderObject->addScorch()` если доступен
- Проверить, определён ли `TheTerrainRenderObject` (extern из W3DTerrainVisual)

### 1.2 `MacOSGameClient::createRayEffectByTemplate()` — Лазеры/трейсеры
**Файл:** `Main/MacOSGameClient.mm`
**Статус:** ⚠️ No-op
**Импакт:** Лазеры, трейсеры, лучевые эффекты
**Реализация:**
- Делегировать к W3DGameClient если есть аналогичная реализация в Core
- Или создать W3D line/billboard objects

### 1.3 `MacOSGameClient::setTeamColor() / setTextureLOD()`
**Файл:** `Main/MacOSGameClient.mm`
**Статус:** ⚠️ No-op
**Импакт:** Цвета фракций, качество текстур
**Реализация:**
- Делегировать к W3DGameClient::setTeamColor() / setTextureLOD()

### 1.4 `MacOSSnowManager` — Снег
**Файл:** `Main/MacOSGameClient.mm`
**Статус:** ⚠️ All no-ops
**Импакт:** Снег на некоторых картах
**Реализация:**
- Делегировать к `W3DSnowManager` если доступен
- Или создать particle system

### 1.5 `MacOSDisplay::takeScreenShot()` — In-game скриншот
**Файл:** `Client/MacOSDisplay.mm`
**Статус:** ⚠️ Empty
**Реализация:**
- Захватить текущий drawable из MetalDevice8
- Сохранить как TGA через `CGImageDestination`

### 1.6 `MacOSFontLibrary::loadFontData()` — Font метрики
**Файл:** `Main/MacOSGameClient.mm`
**Статус:** ⚠️ Sets fontData=nullptr
**Реализация:**
- Считать метрики шрифта через CoreText
- Заполнить fontData структуру

---

## Фаза 2: Cosmetic — Не критичные, но полезные

### 2.1 `StdMouse::setCursor()` — Кастомные курсоры
**Файл:** `Main/StdMouse.mm`
**Статус:** ⚠️ Ограничен 3 курсорами (arrow/crosshair/hand)
**Реализация:**
- Загрузить .ani/.cur файлы, конвертировать в NSCursor

### 2.2 `MetalInterface8::EnumAdapterModes()` — Список разрешений
**Файл:** `Metal/MetalInterface8.mm`
**Статус:** ⚠️ Returns 800×600 only
**Реализация:**
- Query `NSScreen.mainScreen.frame`

### 2.3 `StdMouse::capture() / releaseCapture()` — Захват мыши
**Файл:** `Main/StdMouse.mm`
**Статус:** ⚠️ Empty
**Реализация:**
- `CGAssociateMouseAndMouseCursorPosition(false/true)`

### 2.4 `MacOSAudioManager::getDevice() / getHandleForBink()` — Audio
**Файл:** `Audio/MacOSAudioManager.mm`
**Статус:** ⚠️ Returns nullptr
**Реализация:**
- Dummy handle для Bink video audio

### 2.5 Git Info stubs
**Файл:** `Stubs/GitInfoStubs.cpp`
**Статус:** ⚠️ Hardcoded "MACOS_BUILD_STUB"
**Реализация:**
- CMake `execute_process(COMMAND git rev-parse HEAD ...)`

---

## 🚫 НЕ НУЖНО реализовывать (~180 стабов)

| Категория | Кол-во | Почему |
|-----------|--------|--------|
| GameSpy/Network | ~170 | Онлайн мультиплеер — не актуально |
| WWDownload/Cftp | ~17 | Скачивание патчей через интернет |
| CDManager | 3 | CD проверка — уже обходится |
| windows.h shims | 8 | `GetDesktopWindow`, `GetDC` — маркеры |
| IME Manager | 1 | CJK ввод — не нужен |
| DX8WebBrowser | 4 | EA Browser — не нужен |
| WorkerProcess | 6 | `isDone()=true` — OK |
| MacOSGadgetDraw | 10 | Не используются — W3D рисует |

---

## ✅ УЖЕ РЕАЛИЗОВАНО (ранее были стабы)

| Стаб | Когда | Как |
|------|-------|-----|
| D3DXFilterTexture | 2026-02-25 | Metal `generateMipmapsForTexture` |
| SetLOD/GetLOD | 2026-02-25 | Хранение m_LOD |
| SetGammaRamp | 2026-02-25 | `CGSetDisplayTransferByTable` |
| W3DShaderManager (60+ функций) | 2026-02-25 | **Удалён stub файл** → Core реализации |
| DX8Wrapper::Set_Texture | ранее | Real texture binding |
| All Metal/DX8 rendering | ранее | MetalDevice8, MetalTexture8, etc. |

---

## Порядок выполнения

```
Фаза 1 (Gameplay):    1.1 → 1.2 → 1.3 → 1.4 → 1.5 → 1.6
Фаза 2 (Cosmetic):    2.1 → 2.2 → 2.3 → остальные
```

**Стратегия:** Проверять, есть ли в Core/W3DGameClient готовая реализация,
и делегировать к ней (как с W3DShaderManager). Это может разблокировать
функционал без написания нового кода.

---

## Ключевые файлы

| Файл | Назначение |
|------|-----------|
| `MacOSShaders.metal` | Fragment shader — TSS pipeline, fog, alpha, discard |
| `MetalDevice8.mm` | Metal pipeline — draw calls, uniforms, textures, PSO cache |
| `MetalTexture8.mm` | Texture creation, LockRect/UnlockRect, format conversion |
| `MetalSurface8.mm` | Surface → texture upload with 16→32 bit conversion |
| `D3DXStubs.mm` | D3DX helpers, texture loading, mipmap generation |
| `MacOSGameClient.mm` | Game client factory methods (scorch, ray effects, snow) |
| `dx8wrapper.cpp` | Apply_Render_State_Changes, texture caching |
| `STUBS_AUDIT.md` | Full audit of all stubs — **update after each completion** |

## ⚠️ Правила
- `printf` + `fflush(stdout)` для логов (НЕ `fprintf(stderr)`)
- Не удалять `discard_fragment` для пустых текстур в шейдере
- Тестировать на shell map (горы = `.agent/image_zh_origin.png`)
- Собирать и тестировать: `sh build_run_mac.sh --screenshot`
- **После каждого стаба — обязательный цикл верификации:**
  1. Добавить `printf("[STUB_NAME] called: params=...\n"); fflush(stdout);` в реализацию
  2. `sh build_run_mac.sh --screenshot` — убедиться что билд ОК
  3. `grep "STUB_NAME" Platform/MacOS/Build/Logs/game.log` — проверить что функция вызывается
  4. Скриншот — визуально сравнить с `.agent/image_zh_origin.png`
  5. `grep -i "error\|crash\|assert" Platform/MacOS/Build/Logs/game.log` — нет новых ошибок
- После верификации: обновить `STUBS_AUDIT.md` (⚠️ → ✅)
- Коммитить после каждой фазы или значимого стаба
- **Проверять Core на готовые реализации** перед написанием нового кода (`nm *.o | grep symbol`)
