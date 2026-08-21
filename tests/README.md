# GameMath cross-platform check

`verify_game_math.c` calls every function the built GameMath library exports over
a fixed input set and writes the raw bits of each result to a file. Comparing the
files from two platforms shows exactly which calls disagree.

On Windows the whole set runs twice, once with the x87 precision control set to
`_PC_24` and once with `_PC_53`, since the 32-bit game build sets `_PC_24` in
`setFPMode()`. On macOS there is no x87 precision control, so it runs once.

## Build and run

macOS:

```
cc -O2 -ffp-contract=off tests/verify_game_math.c \
   -I build/macos/_deps/gamemath-src/include \
   build/macos/_deps/gamemath-build/libgm.a -o verify_game_math
./verify_game_math
```

win32 x86, from a Developer Command Prompt:

```
cl /O2 /fp:precise tests\verify_game_math.c ^
   /I build\win32\_deps\gamemath-src\include ^
   build\win32\_deps\gamemath-build\Release\gm.lib ^
   /Fe:verify_game_math.exe
verify_game_math.exe
```

If `gm.lib` is somewhere else:

```
dir /s /b build\win32\_deps\gamemath-build\*.lib
```

## Output

Files are written to the working directory:

| File | Produced on |
| :--- | :--- |
| `math-mac.txt` | macOS ARM64 |
| `math-win-PC24.txt` | win32 x86 under `_PC_24` |
| `math-win-PC53.txt` | win32 x86 under `_PC_53` |

Each line is `function`, `arguments`, `result bits`. Doubles print as 16 hex
digits, floats as 8, integer results as decimal. Functions with out parameters
print both the return value and the parameter.

Compare with a plain diff. Windows writes CRLF, so normalise first:

```
diff <(tr -d '\r' < math-mac.txt) <(tr -d '\r' < math-win-PC24.txt)
```

## Inputs

Inputs are grouped by domain so that out-of-range calls do not fill the output
with NaN, whose bit patterns can legitimately differ between platforms: a general
set, a positive-only set for logarithms and roots, `[-1, 1]` for the inverse
trigonometric functions, and `>= 1` for `acosh`.

## Not covered

These 16 functions are declared in `gmath.h` but are missing from the built
library, so they are skipped:

`gm_fdim`, `gm_fdimf`, `gm_j1`, `gm_jn`, `gm_ldexp`, `gm_ldexpf`, `gm_llround`,
`gm_nearbyint`, `gm_nearbyintf`, `gm_nextafterf`, `gm_remquof`, `gm_scalbln`,
`gm_scalblnf`, `gm_tgamma`, `gm_y1`, `gm_yn`.
