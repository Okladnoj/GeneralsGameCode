# GameMath cross-platform check

`verify_game_math.c` runs each GameMath function on the same input value four
different ways and writes the raw bits of every result to a file. Comparing the
files from two platforms shows which calls disagree, and the four rows show
whether the disagreement comes from the function itself or from a conversion
around it.

One argument functions:

| Row | Meaning |
| :--- | :--- |
| `.d` | double function, double result — `gm_f(x)` |
| `.f` | float function, float result — `gm_ff((float)x)` |
| `.f2d` | float function, result widened — `(double)gm_ff((float)x)` |
| `.d2f` | double function, result narrowed — `(float)gm_f(x)` |

Two argument functions also vary each argument independently, since in real code
a value may arrive as a full double or as one that has already been through a
float. Every argument combination is recorded with the result kept as double and
with it narrowed to float:

| Row | Meaning |
| :--- | :--- |
| `.dd` | `gm_f(x, y)` |
| `.dd2f` | `(float)gm_f(x, y)` |
| `.df` | `gm_f(x, (double)(float)y)` |
| `.df2f` | `(float)gm_f(x, (double)(float)y)` |
| `.fd` | `gm_f((double)(float)x, y)` |
| `.fd2f` | `(float)gm_f((double)(float)x, y)` |
| `.ff` | `gm_f((double)(float)x, (double)(float)y)` |
| `.ff2f` | `(float)gm_f((double)(float)x, (double)(float)y)` |
| `.f` | `gm_ff((float)x, (float)y)` |
| `.f2d` | `(double)gm_ff((float)x, (float)y)` |

The `.f2d` row is what the `WWMath` wrappers do, so it is the one that matters
for the game.

These rows are not interchangeable even on a single platform. On macOS ARM64,
`atan2(187.66, -59.13)` gives four different doubles depending on which argument
went through a float, and `.f` and `.d2f` differ by one ULP.

On Windows the whole matrix runs twice, once with the x87 precision control set
to `_PC_24` and once with `_PC_53`, since the 32-bit game build sets `_PC_24` in
`setFPMode()`. On macOS there is no x87 precision control, so it runs once.

## Build and run

Each build names its own output after the platform, the architecture, the `/fp`
model and, on 32-bit x86, the x87 precision control. Several configurations can
therefore be dumped side by side without overwriting each other.

macOS:

```
cc -O2 -ffp-contract=off tests/verify_game_math.c \
   -I build/macos/_deps/gamemath-src/include \
   build/macos/_deps/gamemath-build/libgm.a -o verify_game_math
./verify_game_math
```

win32 x86, from an x86 Developer Command Prompt:

```
cl /O2 /fp:precise tests\verify_game_math.c ^
   /I build\win32\_deps\gamemath-src\include ^
   build\win32\_deps\gamemath-build\Release\gm.lib ^
   /Fe:verify_game_math.exe
verify_game_math.exe
```

Same thing with strict floating point — only the flag and the output name change:

```
cl /O2 /fp:strict tests\verify_game_math.c ^
   /I build\win32\_deps\gamemath-src\include ^
   build\win32\_deps\gamemath-build\Release\gm.lib ^
   /Fe:verify_game_math_strict.exe
verify_game_math_strict.exe
```

win64, from an x64 Developer Command Prompt. This needs a 64-bit build of
GameMath. `/fp` still matters here, so build it both ways:

```
cl /O2 /fp:precise tests\verify_game_math.c ^
   /I build\win64\_deps\gamemath-src\include ^
   build\win64\_deps\gamemath-build\Release\gm.lib ^
   /Fe:verify_game_math64.exe
verify_game_math64.exe

cl /O2 /fp:strict tests\verify_game_math.c ^
   /I build\win64\_deps\gamemath-src\include ^
   build\win64\_deps\gamemath-build\Release\gm.lib ^
   /Fe:verify_game_math64_strict.exe
verify_game_math64_strict.exe
```

There is no `_PC_24` / `_PC_53` pair on x64. The precision control belongs to the
x87 unit, which 64-bit code does not use for floating point, and MSVC does not
accept those values there. SSE2 has no equivalent knob: every operation is
computed at the precision it was declared with. So an x64 build produces one
file per `/fp` model instead of two.

If `gm.lib` is somewhere else:

```
dir /s /b build\win32\_deps\gamemath-build\*.lib
```

## Output

Files are written to the working directory and named
`math-<platform>-<arch>-<fp model>[-PC24|-PC53].txt`:

| File | Produced on |
| :--- | :--- |
| `math-mac-arm64-clang.txt` | macOS ARM64 |
| `math-win-x86-precise-PC24.txt` | win32 x86, `/fp:precise`, `_PC_24` |
| `math-win-x86-precise-PC53.txt` | win32 x86, `/fp:precise`, `_PC_53` |
| `math-win-x86-strict-PC24.txt` | win32 x86, `/fp:strict`, `_PC_24` |
| `math-win-x86-strict-PC53.txt` | win32 x86, `/fp:strict`, `_PC_53` |
| `math-win-x64-precise.txt` | win64, `/fp:precise` |
| `math-win-x64-strict.txt` | win64, `/fp:strict` |

Each line is `function.row`, `arguments`, `result bits`. Doubles print as 16 hex
digits, floats as 8. The argument column is padded to a fixed width but never
truncated, so a long argument list simply pushes the result column right.

`compare_math.sh` takes the macOS dump as the baseline, compares every other
`math-*.txt` against it and writes `math-diff.txt` with the differing lines and a
legend:

```
sh compare_math.sh
```

It picks up whatever dumps are present, so adding a configuration needs no
change to the script.

## Inputs

Inputs are grouped by domain so that out-of-range calls do not fill the output
with NaN, whose bit patterns can legitimately differ between platforms: a general
set, a positive-only set for logarithms and roots, `[-1, 1]` for the inverse
trigonometric functions, and `>= 1` for `acosh`.

## Not covered

Only functions that exist in both a double and a float form are in the matrix,
since the whole point is comparing the conversions. That leaves out the integer
returning ones (`lrint`, `lround`, `llrint`, `ilogb`), the ones with out
parameters (`frexp`, `modf`, `remquo`) and the ones taking an integer argument
(`ldexp`, `scalbn`, `jn`, `yn`).

These 16 are declared in `gmath.h` but are missing from the built library
altogether:

`gm_fdim`, `gm_fdimf`, `gm_j1`, `gm_jn`, `gm_ldexp`, `gm_ldexpf`, `gm_llround`,
`gm_nearbyint`, `gm_nearbyintf`, `gm_nextafterf`, `gm_remquof`, `gm_scalbln`,
`gm_scalblnf`, `gm_tgamma`, `gm_y1`, `gm_yn`.
