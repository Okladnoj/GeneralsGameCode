# V3: Stub Implementation Plan — Systematic Completion

## ⚠️ ОБЯЗАТЕЛЬНО прочитать перед началом работы:
- **`Platform/MacOS/docs/STUBS_AUDIT.md`** — полный аудит всех стабов с текущим статусом
- **`Platform/MacOS/docs/RENDERING.md`** — спецификация render pipeline
- **`.agent/image_zh_origin.png`** — оригинальный вид shell map (reference)
- **`.agent/workflows/build-and-run.md`** — как собирать, запускать и делать скриншот

---

## Текущее состояние (2026-02-25)

### ✅ Работает
- **Terrain** — текстуры видны! Песок, горы, камни ✅ (D3DXFilterTexture fix)
- **3D модели** — корабль, камни, деревья — текстуры корректные
- **Вода** — анимированная, корректная
- **UI** — кнопки меню, иконки, текст
- **Terrain mipmap генерация** — D3DXFilterTexture реализован через Metal blit encoder

### ⚠️ Известные визуальные проблемы
- **Вертолёт** — иногда не виден (discard_fragment для пустых DXT1 блоков)
- **Terrain blend** — переходы между текстурами (blend tiles) тёмные/неполные
- **Эффекты** — огонь/вспышки выстрелов не всегда видны
- **Деревья** — частично отсутствуют
- **Shroud/Fog of war** — не реализован (stub)

### Статистика стабов
- **124 ✅ реализовано** / **231 ⚠️ safe stubs** / **0 ❌ dangerous** / **0 🔴 critical**

---

## 🎯 ПЕРВООЧЕРЕДНАЯ ЗАДАЧА: Реализация стабов

Стабы организованы по **фазам от наибольшего визуального/gameplay импакта** к наименьшему.

---

## Фаза 1: Рендеринг — Прямой визуальный эффект

### 1.1 `D3DXLoadSurfaceFromMemory()` — Surface pixel copy
**Файл:** `Main/D3DXStubs.mm`
**Статус:** ⚠️ Частичная реализация (только surface→surface, не memory→surface)
**Импакт:** Используется для динамических текстур (курсор, shroud mask)
**Реализация:**
- Конвертировать source данные из `SrcFormat` в формат destination surface
- Поддержать RECT-based копирование (sub-region)
- Учитывать `ColorKey` для transparency

### 1.2 `D3DXSaveTextureToFileA()` — Texture screenshot
**Файл:** `Main/D3DXStubs.mm`
**Статус:** Не вызывается в gameplay, но полезен для отладки
**Реализация:**
- Считать данные из Metal текстуры через `getBytes`
- Записать как TGA/PNG файл

### 1.3 `MetalTexture8::SetLOD() / GetLOD()` — Texture LOD bias
**Файл:** `Metal/MetalTexture8.mm`
**Статус:** ⚠️ Возвращает 0
**Импакт:** Terrain texture reduction (качество текстур при низком LOD)
**Реализация:**
- Хранить LOD значение в `m_LOD`
- При SetLOD вызывать Metal API для bias (или пересоздавать texture view с mipmap range)

### 1.4 `MetalDevice8::SetGammaRamp()` — Gamma correction
**Файл:** `Metal/MetalDevice8.mm`
**Статус:** ⚠️ No-op
**Импакт:** Яркость/контраст в настройках игры
**Реализация:**
- Использовать `CGSetDisplayTransferByTable` или post-process pass в шейдере
- Хранить gamma ramp, применять при Present

### 1.5 `MacOSDisplay::takeScreenShot()` — In-game screenshot
**Файл:** `Client/MacOSDisplay.mm`
**Статус:** ⚠️ Empty
**Реализация:**
- Захватить текущий drawable из MetalDevice8
- Сохранить как PNG/TGA через `CGImageDestination`

---

## Фаза 2: Gameplay — Влияет на игровой процесс

### 2.1 `W3DShaderManager::setShroudTex()` — Fog of War texture
**Файл:** `Stubs/MacOSW3DShaderManager.mm`
**Статус:** ⚠️ Returns TRUE, stub
**Импакт:** 🔴 **Высокий** — без shroud нет fog of war в gameplay
**Реализация:**
- Получить shroud текстуру из `W3DShroud`
- Привязать к stage 1 или 2 через `DX8Wrapper::Set_Texture()`
- Настроить TSS для мультипликативного блендинга (MODULATE с текстурой)

### 2.2 `W3DShaderManager::startRenderToTexture() / endRenderToTexture()` — RTT
**Файл:** `Stubs/MacOSW3DShaderManager.mm`
**Статус:** ⚠️ No-op / returns nullptr
**Импакт:** Нужен для minimap, water reflections, screen effects
**Реализация:**
- Создать offscreen MTLTexture (render target)
- В `startRenderToTexture()` сохранить текущий render target, переключить encoder
- В `endRenderToTexture()` восстановить render target, вернуть offscreen MTLTexture
- Потребует `MetalDevice8` поддержки смены render target mid-frame

### 2.3 `MacOSGameClient::addScorch()` — Scorched earth marks
**Файл:** `Main/MacOSGameClient.mm`
**Статус:** ⚠️ No-op
**Импакт:** Визуальные следы взрывов на земле
**Реализация:**
- Делегировать к `TheTerrainRenderObject->addScorch()` если доступен
- `TheTerrainRenderObject` = `W3DTerrainVisual::getTerrainRenderObject()`

### 2.4 `MacOSGameClient::createRayEffectByTemplate()` — Laser/tracer effects
**Файл:** `Main/MacOSGameClient.mm`
**Статус:** ⚠️ No-op
**Импакт:** Лазеры, трейсеры, лучевые эффекты
**Реализация:**
- Создать W3D line/billboard objects для ray effects
- Нужен доступ к W3D scene

### 2.5 `MacOSSnowManager` — Weather effects
**Файл:** `Main/MacOSGameClient.mm`
**Статус:** ⚠️ All no-ops
**Импакт:** Снег на некоторых картах
**Реализация:**
- Создать particle system для снежинок
- Или делегировать к `W3DSnowManager` если доступен

---

## Фаза 3: Post-Processing / Filters — Визуальные эффекты

### 3.1 `ScreenBWFilter` — Black & White effect (Nuclear bomb)
**Файл:** `Stubs/MacOSW3DShaderManager.mm`
**Статус:** ⚠️ All no-ops
**Импакт:** Эффект ядерного удара (BW flash)
**Реализация:**
- Post-process pass: render fullscreen quad с Convert-to-luminance shader
- Fade от BW → color через `m_curFadeValue`

### 3.2 `ScreenMotionBlurFilter` — Motion blur
**Файл:** `Stubs/MacOSW3DShaderManager.mm`
**Статус:** ⚠️ All no-ops
**Импакт:** Blur при camera zoom/rotate
**Реализация:**
- Accumulation buffer или velocity-based blur
- Требует RTT (зависит от 2.2)

### 3.3 `ScreenCrossFadeFilter` — Cross-fade transitions
**Файл:** `Stubs/MacOSW3DShaderManager.mm`
**Статус:** ⚠️ All no-ops
**Импакт:** Плавные переходы между сценами
**Реализация:**
- Сохранить предыдущий кадр
- Blend между старым и новым через `m_curFadeValue`

### 3.4 `W3DShaderManager::drawViewport()` — Viewport overlay
**Файл:** `Stubs/MacOSW3DShaderManager.mm`
**Статус:** ⚠️ No-op
**Реализация:**
- Render fullscreen quad с заданным цветом (используется для screen overlays)

---

## Фаза 4: Cosmetic / Не критичные

### 4.1 `MetalInterface8::EnumAdapterModes()` — Screen resolution list
**Файл:** `Metal/MetalInterface8.mm`
**Статус:** ⚠️ Returns 800×600 only
**Реализация:**
- Query `NSScreen.mainScreen.frame` и вернуть реальные доступные разрешения
- `GetAdapterModeCount()` → вернуть количество режимов

### 4.2 `StdMouse::setCursor()` — Custom cursor images
**Файл:** `Main/StdMouse.mm`
**Статус:** ⚠️ Limited (arrow/crosshair/hand only)
**Реализация:**
- Загрузить .ani/.cur файлы из .big архивов
- Конвертировать в NSCursor с анимацией

### 4.3 `StdMouse::capture() / releaseCapture()` — Mouse capture
**Файл:** `Main/StdMouse.mm`
**Статус:** ⚠️ Empty
**Реализация:**
- `CGAssociateMouseAndMouseCursorPosition(false/true)` для захвата
- Или `[NSEvent addLocalMonitorForEventsMatchingMask:]`

### 4.4 `MacOSFontLibrary::loadFontData()` — Font metrics
**Файл:** `Main/MacOSGameClient.mm`
**Статус:** ⚠️ Sets fontData=nullptr
**Реализация:**
- Считать метрики шрифта через CoreText
- Заполнить `fontData` структуру с height, ascent, descent

### 4.5 `MacOSAudioManager::getDevice() / getHandleForBink()` — Audio handles
**Файл:** `Audio/MacOSAudioManager.mm`
**Статус:** ⚠️ Returns nullptr
**Реализация:**
- Вернуть dummy handle (не nullptr) если Bink video playback нуждается в audio device
- Или реализовать через AVAudioEngine

### 4.6 `CDManagerStub` — CD check bypass
**Файл:** `Main/MacOSMain.mm`
**Статус:** ⚠️ Returns nullptr
**Импакт:** Уже работает — `driveCount()` returns 0, CD check skipped

### 4.7 Git Info stubs
**Файл:** `Stubs/GitInfoStubs.cpp`
**Статус:** ⚠️ Hardcoded "MACOS_BUILD_STUB"
**Реализация:**
- Читать git info из CMake-сгенерированного файла
- Или `git rev-parse HEAD` при сборке

---

## Порядок выполнения

```
Фаза 1 (Рендеринг):     1.3 → 1.4 → 1.1 → 1.5 → 1.2
Фаза 2 (Gameplay):       2.1 → 2.2 → 2.3 → 2.4 → 2.5
Фаза 3 (Post-process):   3.1 → 3.4 → 3.3 → 3.2
Фаза 4 (Cosmetic):       4.1 → 4.2 → 4.4 → остальные
```

**Приоритет #1:** Shroud (2.1) — без него нет fog of war → нельзя играть.
**Приоритет #2:** RTT (2.2) — нужен для minimap и многих эффектов.
**Приоритет #3:** LOD (1.3) + Gamma (1.4) — качество рендеринга.

---

## Ключевые файлы

| Файл | Назначение |
|------|-----------|
| `MacOSShaders.metal` | Fragment shader — TSS pipeline, fog, alpha, discard |
| `MetalDevice8.mm` | Metal pipeline — draw calls, uniforms, textures, PSO cache |
| `MetalTexture8.mm` | Texture creation, LockRect/UnlockRect, format conversion |
| `MetalSurface8.mm` | Surface → texture upload with 16→32 bit conversion |
| `D3DXStubs.mm` | D3DX helpers, texture loading, mipmap generation |
| `MacOSW3DShaderManager.mm` | W3D shader/filter stubs |
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
