# V2: Terrain Texture Fix — Next Steps

## ⚠️ ОБЯЗАТЕЛЬНО прочитать перед началом работы:
- **`Platform/MacOS/docs/RENDERING.md`** — спецификация render pipeline (terrain pipeline, texture caching, TSS)
- **`.agent/image_zh_origin.png`** — оригинальный вид shell map (reference)
- **`.agent/workflows/build-and-run.md`** — как собирать, запускать и делать скриншот

---

## Текущее состояние (2026-02-25)

### ✅ Работает
- **3D модели** (корабль, вертолёт, камни, деревья) — текстуры корректные
- **Вода** — анимированная, корректная
- **UI** — кнопки меню, иконки, текст
- **Эффекты** — огонь, взрывы, дым
- **Terrain geometry** — heightmap рендерится, форма гор правильная

### ❌ OPEN BUG: White Terrain (текстура не применяется)

**Симптом**: Terrain heightmap рендерится как **белые горы** без текстур. Геометрия видна и opaque, но macro texture atlas не применяется. Сравни с `.agent/image_zh_origin.png`.

---

## Root Cause: Unconditional Null Texture Re-Apply

`Apply_Render_State_Changes()` в `dx8wrapper.cpp` (строка ~2377) на Apple **безусловно** переприменяет `render_state.Textures[0]` на каждый `Draw()`:

```cpp
#ifdef __APPLE__
    { // безусловный блок — ВСЕГДА входит
#else
    if (render_state_changed & mask) { // условный — только при изменении
#endif
```

Terrain shader (`TerrainShader2Stage::set()` в `W3DShaderManager.cpp`) привязывает текстуру **напрямую** к device:
```cpp
DX8Wrapper::_Get_D3D_Device8()->SetTexture(0, texture->Peek_D3D_Texture());
```
Это обновляет только device cache (`m_Textures[0]`), НЕ wrapper cache (`render_state.Textures[0]`).

`render_state.Textures[0]` = `nullptr` (установлен в `HeightMapRenderObjClass::Render()`). Безусловный re-apply ставит `Apply_Null(0)` → затирает terrain текстуру.

### Три уровня кеширования текстур

```
Level 1: render_state.Textures[i]    — TextureBaseClass*     ← ЗДЕСЬ null
         Updated by: DX8Wrapper::Set_Texture()

Level 2: DX8Wrapper::Textures[i]     — IDirect3DBaseTexture8*
         Updated by: DX8Wrapper::Set_DX8_Texture()

Level 3: MetalDevice8::m_Textures[i]  — IDirect3DBaseTexture8*  ← terrain ставит СЮДА
         Updated by: MetalDevice8::SetTexture()
```

---

## Что пробовали и результат

| # | Подход | Файл | Результат | Почему не сработало |
|:--|:---|:---|:---|:---|
| 1 | Skip `Apply_Null` если `TEXTURE_CHANGED` не set | `dx8wrapper.cpp` | Terrain невидим (transparent) | `alpha = diffuse.a` может быть 0; без null re-apply, alpha blending pass рендерит прозрачно |
| 2 | `Set_Texture()` в `TerrainShader2Stage::set()` | `W3DShaderManager.cpp` | Всё ещё белый | Текстура ставится, но `COLOROP = SELECTARG2` (игнорирует текстуру, берёт только diffuse) |
| 3 | Fix #1 + Fix #2 вместе | оба файла | Всё ещё белый | TSS показывает MODULATE в логах, но визуально белый — возможно текстура содержит белые данные или alpha проблема |

### TSS Backtrace (доказательство)

`COLOROP` переключается между MODULATE(4) и SELECTARG2(3) из-за **UI draws** (мышь, кнопки), а НЕ между terrain tiles:

```
[TSS_DIAG] stage0 COLOROP 4→3 backtrace:
  ShaderClass::Apply()
  Apply_Render_State_Changes()
  Draw() → Render2DClass::Render() → StdMouse::draw()
```

Внутри terrain pass, TSS стабильно MODULATE — проблема только в null текстуре.

---

## Применённые фиксы (уже в коде)

В `MacOSShaders.metal`:

1. **D3DTOP_DISABLE alpha**: При `alphaOp == D3DTOP_DISABLE` alpha сохраняет `current.a` (раньше вычислялось через `evaluateOp` → возвращало texture alpha → terrain становился прозрачным)

2. **Removed early-return hack**: Убран `if (useProjection==1 && hasTexture[0]==0) return diffuse`. Все draws проходят через TSS pipeline.

---

## Рекомендуемые стратегии фикса

### Option A — Не давать Apply_Null затирать device текстуру
В `Apply_Render_State_Changes()`, Apple блок: если `render_state.Textures[i] == null` И `TEXTURE_CHANGED` НЕ set → **пропустить** Apply_Null. НО: нужно также убедиться что alpha правильный (alphaOp=DISABLE → alpha=1.0, alpha blending OFF для pass 0).

**Проблема**: при этом фиксе terrain рендерился прозрачным. Нужно одновременно фиксить alpha blending state.

### Option B — Terrain shader обновляет wrapper cache
В `TerrainShader2Stage::set()` на Apple, после direct device SetTexture, вызывать `DX8Wrapper::Set_Texture(0, textureObj)`. Это обновит `render_state.Textures[0]` → re-apply поставит правильную текстуру.

**Проблема**: Set_Texture() ставит TEXTURE0_CHANGED → Apply повторно вызывает TextureClass::Apply() → может конфликтовать с ShaderClass::Apply() timing.

### Option C — Убрать unconditional re-apply полностью
Удалить `#ifdef __APPLE__` unconditional block. Metal port читает текстуры из `m_Textures[]` при DrawIndexedPrimitive — re-apply из wrapper не нужен.

**Риск**: могут сломаться другие текстуры (модели, UI) которые зависят от re-apply.

### Option D — Terrain shader с TEXTURING_ENABLE
Изменить terrain shader's `m_shaderClass` на `TEXTURING_ENABLE` → ShaderClass::Apply() поставит MODULATE вместо SELECTARG2 → terrain texture будет multiplied с diffuse.

**Риск**: нужно убедиться что terrain shader ставит текстуру ДО ShaderClass::Apply().

---

## Ключевые файлы

| Файл | Назначение |
|------|-----------|
| `MacOSShaders.metal` | Fragment shader — TSS pipeline, fog, alpha |
| `MetalDevice8.mm` | Metal pipeline — draw calls, uniforms, textures |
| `dx8wrapper.cpp` | `Apply_Render_State_Changes()` — unconditional re-apply (строка ~2377) |
| `W3DShaderManager.cpp` | `TerrainShader2Stage::set()` — terrain texture + TSS binding |
| `dx8wrapper.h` | `Set_Texture()`, `Set_DX8_Texture_Stage_State()` — wrapper caching |
| `shader.cpp` | `ShaderClass::Apply()` — TSS from shader settings |
| `HeightMap.cpp` | `HeightMapRenderObjClass::Render()` — terrain draw order |

## ⚠️ Правила
- `printf` + `fflush(stdout)` для логов (НЕ `fprintf(stderr)`)
- Не удалять `discard_fragment` для пустых текстур в шейдере
- Тестировать на shell map (горы = `.agent/image_zh_origin.png`)
- Собирать и тестировать: `sh build_run_mac.sh --screenshot`
