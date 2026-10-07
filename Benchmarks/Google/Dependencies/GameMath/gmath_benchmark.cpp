/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2026 TheSuperHackers
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * Times GameMath against the system math library, function by function.
 *
 * For every function there are four benchmarks: the GameMath double entry
 * point (BM_gm_sin), the system double one (BM_sin), and the same pair in float
 * (BM_gm_sinf, BM_sinf). Each iteration calls the function once per input.
 *
 * On 32-bit x86 every benchmark runs twice, with the x87 precision control set
 * to _PC_24 (pc:24) and to _PC_53 (pc:53), since the game build sets _PC_24 in
 * setFPMode(). Only two x87 instructions change speed with that setting, fdiv
 * and fsqrt, so the difference is expected to sit in the functions that divide
 * or take roots. Elsewhere there is no x87 precision control and each
 * benchmark runs once.
 */

#include <benchmark/benchmark.h>
#include <math.h>

#ifdef _WIN32
#include <float.h>
#endif

#include "gmath.h"

#define NV 6

/* ---------- input sets ---------- */

/* general values */
static const double v_gen[]  = { 0.5, -0.5, 2.7, 187.66, 0.000123, 100.5 };
/* positive only */
static const double v_pos[]  = { 0.5, 1.0, 2.7, 187.66, 0.000123, 100.5 };
/* inside [-1, 1] */
static const double v_unit[] = { -0.9, -0.5, 0.0, 0.25, 0.5, 0.9 };
/* one and above */
static const double v_ge1[]  = { 1.0, 1.5, 2.7, 10.0, 187.66, 1000.0 };

static const double v_pair[][2] = {
    { 0.4, 1.3 }, { 2.7, 11.9 }, { 187.66, -59.13 },
    { 0.000123, 9999.5 }, { 1.0, -1.0 }, { -3.5, 2.0 }
};

/*
 * The timed expressions read their inputs from here rather than from the
 * constant arrays above. Without this the compiler folds calls to the system
 * functions at compile time, since it knows what sin(0.5) is, while the
 * GameMath calls stay opaque and the comparison becomes meaningless.
 */
static volatile double vin[NV];
static volatile double vinx[NV];
static volatile double viny[NV];

static void load1(const double *s)
{
    int i;
    for (i = 0; i < NV; ++i) vin[i] = s[i];
}

static void load2(void)
{
    int i;
    for (i = 0; i < NV; ++i) { vinx[i] = v_pair[i][0]; viny[i] = v_pair[i][1]; }
}

/* ---------- x87 precision control ---------- */

/* x87 precision control only exists on 32-bit x86. */
#if defined(_WIN32) && (defined(_M_IX86) || defined(__i386__))

class X87Precision
{
public:
    explicit X87Precision(const benchmark::State &state)
    {
        unsigned int unused;
        _controlfp_s(&m_previous, 0, 0);
        _controlfp_s(&unused, state.range(0) == 24 ? _PC_24 : _PC_53, _MCW_PC);
    }

    ~X87Precision()
    {
        unsigned int unused;
        _controlfp_s(&unused, m_previous & _MCW_PC, _MCW_PC);
    }

private:
    unsigned int m_previous;
};

#define PRECISIONS ->ArgName("pc")->Arg(24)->Arg(53)

#else

class X87Precision
{
public:
    explicit X87Precision(const benchmark::State &) {}
};

#define PRECISIONS

#endif

/* ---------- benchmarks ---------- */

#define TIME_D(name, load, expr)                                            \
    static void name(benchmark::State &state)                               \
    {                                                                       \
        X87Precision precision(state);                                      \
        load;                                                               \
        for (auto _ : state) {                                              \
            double acc = 0.0;                                               \
            for (int i = 0; i < NV; ++i) acc += (expr);                     \
            benchmark::DoNotOptimize(acc);                                  \
        }                                                                   \
        state.SetItemsProcessed(state.iterations() * NV);                   \
    }                                                                       \
    BENCHMARK(name) PRECISIONS

#define TIME_F(name, load, expr)                                            \
    static void name(benchmark::State &state)                               \
    {                                                                       \
        X87Precision precision(state);                                      \
        load;                                                               \
        for (auto _ : state) {                                              \
            float acc = 0.0f;                                               \
            for (int i = 0; i < NV; ++i) acc += (expr);                     \
            benchmark::DoNotOptimize(acc);                                  \
        }                                                                   \
        state.SetItemsProcessed(state.iterations() * NV);                   \
    }                                                                       \
    BENCHMARK(name) PRECISIONS

#define BENCH1(base, set)                                                   \
    TIME_D(BM_gm_##base, load1(set), gm_##base(vin[i]));                    \
    TIME_D(BM_##base, load1(set), base(vin[i]));                            \
    TIME_F(BM_gm_##base##f, load1(set), gm_##base##f((float)vin[i]));       \
    TIME_F(BM_##base##f, load1(set), base##f((float)vin[i]))

#define BENCH2(base)                                                        \
    TIME_D(BM_gm_##base, load2(), gm_##base(vinx[i], viny[i]));             \
    TIME_D(BM_##base, load2(), base(vinx[i], viny[i]));                     \
    TIME_F(BM_gm_##base##f, load2(),                                        \
           gm_##base##f((float)vinx[i], (float)viny[i]));                   \
    TIME_F(BM_##base##f, load2(), base##f((float)vinx[i], (float)viny[i]))

BENCH1(sin,   v_gen);
BENCH1(cos,   v_gen);
BENCH1(tan,   v_gen);
BENCH1(asin,  v_unit);
BENCH1(acos,  v_unit);
BENCH1(atan,  v_gen);
BENCH2(atan2);

BENCH1(sinh,  v_gen);
BENCH1(cosh,  v_gen);
BENCH1(tanh,  v_gen);
BENCH1(asinh, v_gen);
BENCH1(acosh, v_ge1);
BENCH1(atanh, v_unit);

BENCH1(exp,   v_gen);
BENCH1(exp2,  v_gen);
BENCH1(expm1, v_gen);
BENCH1(log,   v_pos);
BENCH1(log10, v_pos);
BENCH1(log1p, v_pos);
BENCH1(logb,  v_pos);
BENCH2(pow);

BENCH1(sqrt,  v_pos);
BENCH1(cbrt,  v_gen);
BENCH2(hypot);

BENCH1(ceil,  v_gen);
BENCH1(floor, v_gen);
BENCH1(trunc, v_gen);
BENCH1(round, v_gen);
BENCH1(rint,  v_gen);

BENCH1(fabs,  v_gen);
BENCH2(fmod);
BENCH2(remainder);
BENCH2(copysign);
BENCH2(fmax);
BENCH2(fmin);

BENCH1(erf,   v_unit);
BENCH1(erfc,  v_unit);
BENCH1(lgamma, v_pos);

/* j0 and y0 are left out: macOS libm has no j0f or y0f to compare against. */
