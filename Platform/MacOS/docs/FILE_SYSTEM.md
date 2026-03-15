# Файловая система macOS порта (C&C Generals Zero Hour)

## Общая архитектура

Файловая система состоит из двух абстракций, работающих в связке:

| Компонент | Класс | Назначение |
|-----------|-------|------------|
| `TheLocalFileSystem` | `StdLocalFileSystem` | Loose-файлы на диске (INI, скрипты, текстуры) |
| `TheArchiveFileSystem` | `StdBIGFileSystem` | Файлы внутри `.big` архивов |

При вызове `TheFileSystem->openFile(path)`:
1. Сначала ищется **loose-файл** через `TheLocalFileSystem->openFile`
2. Если не найден — ищется в **`.big` архивах** через `TheArchiveFileSystem->openFile`

> [!IMPORTANT]
> Loose-файлы **всегда** имеют приоритет над `.big` архивами.

## Переменная окружения `GENERALS_INSTALL_PATH`

На Windows игра использует реестр (`InstallPath`). На macOS — переменную окружения.

```bash
# build_run_mac.sh
export GENERALS_INSTALL_PATH="/Users/okji/dev/games/Command and Conquer - Generals"
```

Ожидаемая структура внутри:
```
Command and Conquer - Generals/
├── Command and Conquer Generals Zero Hour/   ← ZH
│   ├── *.big
│   └── Data/Scripts/SkirmishScripts.scb
└── Command and Conquer Generals/             ← Base
    ├── *.big
    └── Data/...
```

Имена подпапок **захардкожены** в `StdBIGFileSystem::init()`.

## Инициализация (`StdBIGFileSystem::init`)

Порядок инициализации при запуске (macOS, `RTS_ZEROHOUR`):

### 1. Регистрация search paths для loose-файлов

```
TheLocalFileSystem->addSearchPath(zhPath)       ← первый
TheLocalFileSystem->addSearchPath(genBasePath)   ← второй
```

### 2. Загрузка `.big` архивов

```
loadBigFilesFromDirectory("", "*.big")           ← CWD (dev mods)
loadBigFilesFromDirectory(zhPath, "*.big")        ← Zero Hour
loadBigFilesFromDirectory(genBasePath, "*.big")   ← Base Game
```

## Приоритеты

### Loose-файлы (`resolveWithSearchPaths`)

| # | Где ищет | Пример |
|---|---------|--------|
| 1 | CWD (папка исходников) | `./Data/Scripts/foo.scb` |
| 2 | ZH path | `.../Zero Hour/Data/Scripts/foo.scb` |
| 3 | Base path | `.../Generals/Data/Scripts/foo.scb` |

### `.big` архивы (первый загруженный побеждает)

| # | Источник | Приоритет |
|---|---------|-----------|
| 1 | CWD `.big` | Наивысший (dev-моды, кастомный UI) |
| 2 | ZH `.big` | Средний |
| 3 | Base `.big` | Низший |

### Итоговый приоритет при `openFile`

```
Loose CWD > Loose ZH > Loose Base > BIG CWD > BIG ZH > BIG Base
```

## Дуальность Core / Platform

В проекте существуют **два** `StdLocalFileSystem`:

| Файл | Расположение |
|------|-------------|
| `Core/GameEngineDevice/.../StdLocalFileSystem.h/.cpp` | Core (общий, компилируется всегда) |
| `Platform/MacOS/.../StdLocalFileSystem.h/.cpp` | Platform (macOS-специфичная версия) |

> [!WARNING]
> **Линкер использует Core-версию.** Platform/MacOS версия компилируется, но
> её символы перезатираются Core версией (или не линкуются вовсе).
> Поэтому все изменения `StdLocalFileSystem` (search paths, `addSearchPath`,
> `resolveWithSearchPaths`) **должны вноситься в Core-версию**.

Аналогичная ситуация с `StdBIGFileSystem`:

| Файл | Расположение |
|------|-------------|
| `Core/GameEngineDevice/.../StdBIGFileSystem.cpp` | Core (используется линкером) |
| `Platform/MacOS/.../StdBIGFileSystem.cpp` | Platform (может не линковаться) |

## Патчи для macOS

### Case-Insensitive поиск файлов

`fixFilenameFromWindowsPath` обходит директории через `std::filesystem::directory_iterator`
и сравнивает имена через `strcasecmp`. Это нужно потому что:
- Игра запрашивает файлы в Windows-стиле: `Data\INI\GameData.ini`
- macOS может иметь case-sensitive файловую систему
- Реальный файл может иметь другой регистр

### Фильтрация дубликата INIZH.big

В `loadBigFilesFromDirectory` пропускается `INIZH.big` из подпапки `Data/INI/`,
потому что многие цифровые версии игры содержат дубликат этого файла,
что приводит к конфликту CRC в сетевой игре.

### Конвертация путей

Все обратные слеши `\` в путях автоматически заменяются на прямые `/` на этапе
открытия файла.

## Критические loose-файлы

Эти файлы **не запакованы** в `.big` и ищутся через search paths:

| Файл | Зачем |
|------|-------|
| `Data/Scripts/SkirmishScripts.scb` | AI скрипты для скирмиша (строительство, атаки) |
| `Data/Scripts/MultiplayerScripts.scb` | Скрипты для мультиплеера |
| `Data/INI/*.ini` | Конфигурация юнитов, оружия, зданий |

> [!CAUTION]
> Если `SkirmishScripts.scb` не найден, бот в скирмише **не строит и не атакует**.
> Это была основная проблема до внедрения `addSearchPath` в Core `StdLocalFileSystem`.

## Кастомные карты (Map Transfer)

### Источники карт

Кастомные карты могут появиться у игрока тремя способами:

| Способ | Механизм | Статус macOS |
|--------|----------|-------------|
| **P2P Transfer** | Хост передаёт карту через `TheNetwork->sendFile()` | ✅ Пути исправлены (PATH_SEP) |
| **GenPatcher Download** | Загрузка ranked карт через HTTP | ✅ `MacOSMapDownloader.mm` |
| **Ручная установка** | Игрок копирует файлы в папку карт | ✅ Работает если путь правильный |

### Windows: где хранятся карты

На Windows кастомные карты пишутся в:
```
%USERPROFILE%\My Documents\Command and Conquer Generals Zero Hour Data\Maps\<mapname>\
```

Этот путь определяется через `portableMapPathToRealMapPath()` в `GameState.cpp`.

### macOS: хранение карт

На macOS кастомные карты сохраняются в `getUserMapDir()` (аналог Windows My Documents):

```
~/Library/Application Support/Generals Zero Hour/Maps/
```

### P2P Map Transfer — поток данных

```
DoAnyMapTransfers() → FileTransfer.cpp
  ├─ Хост: sendFileAnnounce(map, mask) → sendFile(map, mask)
  └─ Клиент: processFile() → TheFileSystem->openFile(path, CREATE|WRITE)
```

Передаются файлы по `mapContentsMask` (побитово):
- `& 2` → preview.tga
- `& 4` → map.ini
- `& 8` → map.str
- `& 16` → solo.ini
- `& 32` → assetusage.txt
- `& 64` → readme.txt
- Всегда → .map файл

### P2P Transfer — проблемы macOS

1. **Backslash пути**: `FileTransfer.cpp::GetBasePathFromPath()` использует `\\`.
   На macOS нужен `/`.

2. **Writable location**: `processFile()` в `ConnectionManager.cpp:745` вызывает
   `TheFileSystem->openFile(realFileName, CREATE)`. Если `realFileName` указывает
   в read-only location — запись невозможна.

3. **Security patch**: `ParseAsciiStringToGameInfo()` (GameInfo.cpp:1124) вызывает
   `portableMapPathToRealMapPath()`. Для неизвестных карт возвращает пустую строку →
   security patch отклоняет SL string → слоты не парсятся → player kicked.

### GenPatcher / Server Map Download

GenTool/GenPatcher на Windows загружает ranked карты с `gentool.net/download/`.
На macOS реализован аналогичный механизм:

- **`MacOSMapDownloader.mm`** — нативное NSWindow с прогресс-баром
- Ranked: скачивает `Maps_All_Ranked_ZH.zip` с `gentool.net/download/`
- Custom: скачивает из `github.com/TheSuperHackers/GeneralsRankedMaps` (backlog/)
- Распаковывает в `getUserMapDir()` (`~/Library/Application Support/Generals Zero Hour/Maps/`)
- Доступен через macOS menu bar: **Tools → Download Ranked Maps** (⌘M) / **Download Custom Maps**
- Работает из любого состояния игры (главное меню, скирмиш, LAN, онлайн)

### Mod Resources (.big)

Кастомные `.big` моды (например, ControlBar HD) хранятся в `Platform/MacOS/Resources/`.
CMake автоматически создаёт симлинки из `Resources/*.big` в ZH install path при конфигурации.

Источники данных GenTool:
- `gentool.net/download/Maps_All_Ranked_ZH.zip` — полный пак ranked карт
- `gentool.net/download/Maps_1v1_ZH.zip` / `Maps_2v2_ZH.zip` / `Maps_3v3_ZH.zip` — по категориям
- `gentool.net/download/maps/` — архивы 2024 года
- `.dat` файлы (`patch_maps_list_zh.dat`) — обфусцированные бинарники GenPatcher (closed source)

В GeneralsOnlineServices (сервер) **нет** Map Download endpoint.
Для кастомных карт используется P2P transfer (Phase 1-3).

### Связанная задача

Подробный план реализации: [`.agent/tasks/net-15-map-transfer.md`](.agent/tasks/net-15-map-transfer.md)

### Участвующие файлы

| Файл | Роль |
|------|------|
| `Core/.../FileTransfer.cpp` | `DoAnyMapTransfers()` — оркестратор P2P transfer |
| `Core/.../ConnectionManager.cpp:688-805` | `processFile/processFileAnnounce` — recv/write |
| `Core/.../GameInfo.cpp:224` | `setMapAvailability(hasMap)` |
| `Core/.../GameInfo.cpp:981` | SL string: `slot->hasMap()?'T':'F'` |
| `Core/.../NetCommandMsg.h:453` | `getRealFilename()` — portable → real path |
| `Platform/MacOS/.../MacOSOnlineLobby.mm:178` | `has_map: YES` → GenOnline API |
| `Platform/MacOS/.../MacOSMapDownloader.mm` | Ranked maps HTTP download + unzip |
| `Platform/MacOS/.../MacOSWindowManager.mm:153-181` | macOS menu bar с Tools → Download Maps |

