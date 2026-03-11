# NET-07: Реальный GameSpy SDK вместо стабов

**Статус:** 📋 Запланировано

## Цель

Заменить пустые C-стабы (`GameSpySDKStubs.c`) на **реальную библиотеку GameSpy SDK**,
скомпилированную из форка, чтобы игра могла подключаться к серверам C&C:Online.

## Предпосылки

- SDK уже **компилируется на macOS** (проверено: 100% успех, 0 ошибок, 1 warning)
- Все 1509 символов экспортируются (`libgamespy.a`)
- Все нужные нам символы присутствуют: GP, Peer, QR2, Chat, GStats, ServerBrowsing, глобальные переменные
- Домен C&C:Online (`server.cnc-online.net`) уже задаётся через CMake переменную `GAMESPY_SERVER_NAME`

## Форк

**Репозиторий:** https://github.com/Okladnoj/GamespySDK.git
**Оригинал:** https://github.com/TheAssemblyArmada/GamespySDK.git

## План работ

### Шаг 1: Переключить cmake на форк

**Файл:** `cmake/gamespy.cmake`

```cmake
FetchContent_Declare(
    gamespy
    GIT_REPOSITORY https://github.com/Okladnoj/GamespySDK.git
    GIT_TAG        main
)
```

Параметры сборки (уже заданы):
- `GS_OPENSSL=OFF` — без OpenSSL
- `GS_BUILD_TESTS=OFF` — без тестов
- `GAMESPY_SERVER_NAME="server.cnc-online.net"` — домен C&C:Online

### Шаг 2: Линковка реальной библиотеки

**Файл:** `Platform/MacOS/CMakeLists.txt`

- Добавить `target_link_libraries(generalszh PRIVATE gamespy)` (и для generals)
- Убрать `GameSpySDKStubs.c` из списка исходников

### Шаг 3: Удалить стабы

**Файл:** `Platform/MacOS/Source/Stubs/GameSpySDKStubs.c` — удалить (558 строк)

Все 77+ символов (GP, Peer, QR2, Chat, GStats, SB, глобалы) теперь
предоставляются реальной библиотекой.

### Шаг 4: Разрешить конфликты символов

**Файл:** `Platform/MacOS/Source/Stubs/GameSpyStubs.cpp`

Проверить что в нём не осталось символов, которые дублируют реальный SDK.
Ранее мы уже чистили — убрали 53 дубля. Нужно ещё раз проверить после
линковки с реальной библиотекой.

### Шаг 5: Проверка сборки и линковки

```bash
sh build_run_mac.sh
```

Ожидаемый результат: сборка без ошибок линковки.

### Шаг 6: Тест подключения

1. Запустить игру
2. Нажать "Online" в главном меню
3. Проверить в логах что SDK пытается резолвить `server.cnc-online.net`
4. Проверить что открывается окно логина

## Символы из реальной библиотеки (проверено через nm)

| Группа | Символы | Статус |
|--------|---------|--------|
| GP (Presence) | gpInitialize, gpDestroy, gpProcess, gpConnectA, ... (40+) | ✅ экспортируются |
| Peer | peerInitialize, peerConnectA, peerThink, ... (50+) | ✅ экспортируются |
| QR2 | qr2_buffer_addA, qr2_keybuffer_add, qr2_register_keyA, ... | ✅ экспортируются |
| Chat | chatSetLocalIP, chatConnectA, ... | ✅ экспортируются |
| GStats | InitStatsConnection, GetChallenge, ... | через gcdkey |
| ServerBrowsing | SBServerGetIntValueA, SBServerGetStringValueA, ... | ✅ экспортируются |
| Globals | gcd_gamename, gcd_secret_key, GPConnectionManagerHostname, GPSearchManagerHostname | ✅ экспортируются |

## Зависимости

- NET-06 (стабы слинковались) — ✅ завершён, стабы будут заменены
- Форк GameSpy SDK — ✅ создан

## Риски

1. **Дубли символов** — GameSpyStubs.cpp может содержать функции, которые тоже есть в SDK
2. **Платформозависимый код** — SDK уже поддерживает macOS (`_MACOSX + _UNIX`), но могут быть edge cases
3. **OpenSSL** — отключён, если C&C:Online требует HTTPS — нужно будет включить
