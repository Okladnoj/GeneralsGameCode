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

Every function is also fed into a small expression, because a result that is
merely stored may be rounded correctly while the same result kept in a register
and used in arithmetic is not. `r1`, `r2` and `r3` are the function applied to
three consecutive inputs, in float and in double:

| Row | Meaning |
| :--- | :--- |
| `.mul.d` / `.mul.f` | `r1 * r2` |
| `.inv.d` / `.inv.f` | `1 / r1` |
| `.mad.d` / `.mad.f` | `r1 * r2 + r3` |

The `.mad` rows are the interesting ones: a three term expression gives the
compiler the most room to keep an intermediate at a wider precision than the
type asks for.

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

Windows, from an ordinary shell — no Developer Command Prompt needed, the script
finds the toolchain itself:

```
powershell -ExecutionPolicy Bypass -File tests\run_verify_game_math.ps1
```

That walks the whole matrix — x86 and x64, `/fp:precise` and `/fp:strict` — and
writes all six dumps to `tests/`. Each build happens in its own temporary
directory outside the repository and is deleted afterwards, so no object files,
import libraries or executables are left behind. A configuration that fails to
build does not stop the others; the failures are listed at the end and the
script exits non-zero. `-Arch x86` or `-Fp precise` narrows the matrix.

The 64-bit half needs a 64-bit GameMath, and nothing in the repository builds
one: the game is x86 only and there is no x64 preset. The script therefore
configures GameMath directly from the sources the 32-bit build tree already
fetched, into `build\win64-gamemath`, with the same options `cmake/gamemath.cmake`
uses and the same runtime library as the win32 tree, so the two libraries differ
in architecture and nothing else. It is built once and reused; `-RebuildX64`
forces it again.

The same by hand, if the script is in the way. win32 x86, from an x86 Developer
Command Prompt:

```
cl /O2 /fp:precise /MD tests\verify_game_math.c ^
   /I build\win32\_deps\gamemath-src\include ^
   build\win32\_deps\gamemath-build\Release\gm.lib ^
   /Fe:verify_game_math.exe
verify_game_math.exe
```

Same thing with strict floating point — only the flag and the output name change:

```
cl /O2 /fp:strict /MD tests\verify_game_math.c ^
   /I build\win32\_deps\gamemath-src\include ^
   build\win32\_deps\gamemath-build\Release\gm.lib ^
   /Fe:verify_game_math_strict.exe
verify_game_math_strict.exe
```

`/MD` is not optional: `gm.lib` is built against the dynamic CRT, and the `cl`
default drags in the static one. The mix fails to link on `__imp__fesetround`
and `__except_handler4_common`.

win64, from an x64 Developer Command Prompt. Build GameMath for x64 first — the
headers come from the existing 32-bit tree, only the library is architecture
specific:

```
cmake -S build\win32\_deps\gamemath-src -B build\win64-gamemath ^
      -G "Ninja Multi-Config" -DGM_ENABLE_TESTS=OFF ^
      -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
cmake --build build\win64-gamemath --config Release
```

Without Ninja, drop the `-G` and pass `-A x64` instead; either way the library
lands in `build\win64-gamemath\Release\gm.lib`. Then, both ways round:

```
cl /O2 /fp:precise /MD tests\verify_game_math.c ^
   /I build\win32\_deps\gamemath-src\include ^
   build\win64-gamemath\Release\gm.lib ^
   /Fe:verify_game_math64.exe
verify_game_math64.exe

cl /O2 /fp:strict /MD tests\verify_game_math.c ^
   /I build\win32\_deps\gamemath-src\include ^
   build\win64-gamemath\Release\gm.lib ^
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
dir /s /b build\*\_deps\gamemath-build\*.lib
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

`compare_math.sh` takes the macOS dump as the baseline and compares every other
`math-*.txt` against it:

```
sh compare_math.sh
```

It writes two files. `math-diff.txt` holds a legend and, per comparison, a
breakdown by row kind followed by the differing lines. `math-summary.txt` holds a
single table with the row kinds down the side and every compared configuration
across the top, for reading the modes against each other at a glance.

Both pick up whatever dumps are present, so adding a configuration needs no
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
