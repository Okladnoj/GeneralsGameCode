# Задача: прогнать GameMath на винде в четырёх конфигурациях

Привет. Я работаю на маке, ты на винде. Нужны твои дампы, чтобы закрыть спор в
PR [#2670](https://github.com/TheSuperHackers/GeneralsGameCode/pull/2670) о том,
оставлять ли `_PC_24` или переходить на `_PC_53`.

Ветка: `okji/test/deterministic-math-v2.2.8`. Всё нужное лежит в `tests/`.

## Что уже есть

| Файл | Откуда |
|---|---|
| `math-mac-arm64-clang.txt` | mac ARM64, сделал я |
| `math-win-x86-precise-PC24.txt` | win32 x86, `/fp:precise` |
| `math-win-x86-precise-PC53.txt` | win32 x86, `/fp:precise` |

## Что нужно от тебя

Четыре прогона. Программа сама определяет разрядность и режим `/fp` и называет
файл соответственно — руками имена задавать не надо.

**1. x86, `/fp:strict`.** Из x86 Developer Command Prompt:

```
cl /O2 /fp:strict tests\verify_game_math.c ^
   /I build\win32\_deps\gamemath-src\include ^
   build\win32\_deps\gamemath-build\Release\gm.lib ^
   /Fe:verify_game_math_strict.exe
verify_game_math_strict.exe
```

Даст `math-win-x86-strict-PC24.txt` и `math-win-x86-strict-PC53.txt`.

**2. x64, `/fp:precise` и `/fp:strict`.** Тут сложнее: нужен 64-битный GameMath,
его сначала надо собрать. Путь `build\win64\...` в README я написал наугад —
**подставь реальный и поправь README**, чтобы за тобой не переделывали.

Найти собранную библиотеку:

```
dir /s /b build\*\_deps\gamemath-build\*.lib
```

Дальше:

```
cl /O2 /fp:precise tests\verify_game_math.c ^
   /I <путь>\gamemath-src\include ^
   <путь>\gamemath-build\Release\gm.lib ^
   /Fe:verify_game_math64.exe
verify_game_math64.exe

cl /O2 /fp:strict tests\verify_game_math.c ^
   /I <путь>\gamemath-src\include ^
   <путь>\gamemath-build\Release\gm.lib ^
   /Fe:verify_game_math64_strict.exe
verify_game_math64_strict.exe
```

Даст `math-win-x64-precise.txt` и `math-win-x64-strict.txt`. Пары `PC24`/`PC53`
там не будет: управление точностью относится к блоку x87, 64-битный код его для
вещественной математики не использует, и MSVC этих значений на x64 не принимает.

## Проверь при первом запуске

Если в имени файла окажется `fpdefault` вместо `precise` или `strict`, значит
MSVC не определил ни один из `_M_FP_*`. Скажи — поправлю определение в
`verify_game_math.c`.

## Что сделать с результатами

1. Положить дампы в `tests/`.
2. Закоммитить и запушить в `okji/test/deterministic-math-v2.2.8`.
3. Дописать отчёт в конец этого файла — что собралось, что нет, какие пути
   реально использовал, что показалось странным.

Сравнение я сделаю сам, скрипт `compare_math.sh` подхватит все найденные дампы.

## Зачем это

Омни (мейнтейнер) спрашивает две вещи, на которые сам ответить не может, потому
что у него только x86:

- совпадает ли 64-битная винда с маком — если да, значит дело в 32-битном ABI, а
  не в библиотеке;
- меняет ли `/fp:strict` картину — по его разбору в godbolt на `/O2` MSVC
  перемножает результаты через x87 вместо SSE, отсюда завышенная промежуточная
  точность.

---

## Отчёт

<!-- заполни здесь -->
