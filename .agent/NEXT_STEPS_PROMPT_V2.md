# V2: Terrain / Ground Texture Layers — Next Steps

## Состояние на 2026-02-25

### ✅ Решено
1. **Модели (юниты, здания, корабль)** — рендерятся с полноценными текстурами.
   - **Корневая причина**: `D3DRS_SPECULARENABLE` по умолчанию `FALSE`, но шейдер
     ВСЕГДА добавлял specular. С `materialPower=0.1` specular вымывал всё в белый.
   - **Фикс**: `specularEnable` в FragmentUniforms, guard в шейдере.

2. **Вода** — анимированная, корректная текстура.

3. **UI** — кнопки меню, иконки, текст работают.

4. **Эффекты** — огонь, взрывы, дым визуально корректны.

### ❌ НЕ решено: Terrain (земля) и горы

**Проблема**: Terrain рисуется МНОГОСЛОЙНО, но overlay/blend текстурные слои
не отображаются. Видна только BASE pass (diffuse vertex color) — бежевый/серый
однородный цвет без текстурных деталей.

**Shell map**: горы — белые блобы (vertex lighting ≈ 1.0 + нет текстурных overlay'ев).
В оригинале горы имеют коричневую скальную текстуру.

**In-game**: земля — однородный цвет без деталей. Дороги видны (отдельный textured
overlay). В оригинале — детальная текстура песка/травы/камня.

---

## Архитектура terrain rendering в Generals

### Как terrain рисуется (DirectX 8 оригинал):

```
Terrain render order:
1. BASE PASS: untextured mesh (diffuse vertex color, lit=0, fvf=0x252)
   → Устанавливает z-buffer, базовый цвет рельефа
   → hasTexture[0]=0

2. BLEND/OVERLAY PASSES: textured quads с alpha blending (множество draw calls)
   → Каждый terrain tile (песок, трава, скала, грязь) рисуется отдельным overlay
   → Vertex alpha контролирует blend factor
   → hasTexture[0]=1, lit=0, fvf=0x252
   → TSS: MODULATE(texture, diffuse), alpha blending ON

3. ROAD PASSES: отдельные textured overlays для дорог
   → Работают ✅

4. MODEL PASSES: W3D mesh объекты (здания, деревья, камни)
   → hasTexture[0]=1, lit=1, fvf=0x112
   → TSS: MODULATE(texture, vertex_color_from_lighting)
```

### Что наблюдаем через debug:

- **EMPTY_TEX**: 20 пустых текстур = все это `MissingTexture` (64x64 BGRA8,
  `HasBeenWritten=false`). Из них 16 = одна текстура `trstrtholecvr.tga` (не найден файл).

- **IDX_DRAW**: MODEL draws все имеют `hasTex0=1` с 128x128 DXT текстурами.
  Данные загружены (`HasBeenWritten=true`). НО горы визуально белые.

- **Debug shader** (split texCoord): на горной карте terrain overlay текстуры
  **ЧАСТИЧНО загружены** — видны синие горы, песок. Но часть overlay'ев = чёрные
  (пустые DXT текстуры).

### Гипотезы почему overlay'и пустые:

1. **Apply_New_Surface race condition**: foreground loading создаёт новый
   MetalTexture8 через `Apply_New_Surface`, но позже thumbnail/background task
   перезаписывает его обратно на пустую thumbnail текстуру.

2. **Terrain texture atlas**: terrain может использовать специальную систему
   текстурных атласов, которая собирает tile-текстуры в большие текстуры.
   Этот процесс может использовать `UpdateTexture` (копирование из sysmem
   в vidmem), который НЕ реализован в Metal адаптере.

3. **Multi-pass blending**: terrain overlay draws используют alpha blending
   с конкретными blend states. Наш адаптер может неправильно устанавливать
   SRC_ALPHA/INV_SRC_ALPHA blend factors.

4. **Текстуры не привязываются**: mesh'и гор могут ссылаться на текстуры,
   которые были созданы, но затем заменены пустыми через lifecycle.

---

## План отладки

### Шаг 1: Проверить `UpdateTexture`
`IDirect3DDevice8::UpdateTexture(src, dst)` копирует данные из system mem
текстуры в video mem. Используется для non-managed текстур. Если наш стаб
— пустой, данные не копируются.
```cpp
// MetalDevice8.mm: проверить реализацию UpdateTexture
```

### Шаг 2: Логировать terrain overlay draws
Увеличить лимит IDX_DRAW лога для unlit textured draws (terrain overlays).
Записать: текстуру pointer, size, HasBeenWritten, blend state.

### Шаг 3: Проверить alpha blending state
Для terrain overlay draws: залогировать `D3DRS_ALPHABLENDENABLE`,
`D3DRS_SRCBLEND`, `D3DRS_DESTBLEND`. Убедиться что они корректно
передаются в Metal pipeline state.

### Шаг 4: Проверить texture lifecycle
Залогировать в каком порядке текстуры создаются, заполняются данными,
и привязываются к draw calls. Искать случаи, где текстура получает данные,
а потом теряет их (перезапись).

---

## Ключевые файлы

| Файл | Назначение |
|------|-----------|
| `MacOSShaders.metal` | Fragment shader — TSS pipeline, fog, specular |
| `MetalDevice8.mm` | Metal pipeline — draw calls, uniforms, textures |
| `MetalTexture8.mm` | Metal texture — LockRect/UnlockRect, data upload |
| `texture.cpp` | TextureClass — Init(), Apply(), texture lifecycle |
| `textureloader.cpp` | TextureLoadTaskClass — Begin_Load, Load, End_Load |
| `dx8wrapper.cpp` | DX8Wrapper — Set_DX8_Texture, draw dispatch |

## ⚠️ Правила
- `printf` + `fflush(stdout)` для логов (НЕ `fprintf(stderr)`)
- Не удалять `discard_fragment` для пустых текстур — без него пустые DXT
  текстуры рендерятся как opaque black
- Тестировать на shell map (горы) И in-game (terrain)
