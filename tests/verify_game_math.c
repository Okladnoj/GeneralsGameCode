/*
 * Dumps the raw bits returned by every GameMath function over a fixed input set.
 *
 * On Windows it runs the whole set twice, once under _PC_24 and once under
 * _PC_53, writing math-win-PC24.txt and math-win-PC53.txt. On macOS it runs
 * once and writes math-mac.txt, since there is no x87 precision control there.
 *
 * Files are written to the working directory. The output is line oriented and
 * stable, so the runs can be compared with a plain diff.
 *
 * macOS:
 *   cc -O2 -ffp-contract=off verify_game_math.c \
 *      -I build/macos/_deps/gamemath-src/include \
 *      build/macos/_deps/gamemath-build/libgm.a -o verify_game_math
 *   ./verify_game_math
 *
 * win32 x86, from a Developer Command Prompt:
 *   cl /O2 /fp:precise verify_game_math.c ^
 *      /I build\win32\_deps\gamemath-src\include ^
 *      build\win32\_deps\gamemath-build\Release\gm.lib ^
 *      /Fe:verify_game_math.exe
 *   verify_game_math.exe
 *
 * Note: 16 functions declared in gmath.h are not present in the built library
 * and are therefore skipped here: gm_fdim, gm_fdimf, gm_j1, gm_jn, gm_ldexp,
 * gm_ldexpf, gm_llround, gm_nearbyint, gm_nearbyintf, gm_nextafterf,
 * gm_remquof, gm_scalbln, gm_scalblnf, gm_tgamma, gm_y1, gm_yn.
 */

#include <stdio.h>
#include <string.h>
#include "gmath.h"

#ifdef _WIN32
#include <float.h>
#endif

/* ---------- raw bit output ---------- */

/* Written next to the executable, one file per precision mode. */
static FILE *g_out;

static void put_d(const char *name, const char *arg, double v)
{
    unsigned long long u;
    memcpy(&u, &v, sizeof u);
    fprintf(g_out, "%-16s %-34s %016llX\n", name, arg, u);
}

static void put_f(const char *name, const char *arg, float v)
{
    unsigned int u;
    memcpy(&u, &v, sizeof u);
    fprintf(g_out, "%-16s %-34s %08X\n", name, arg, u);
}

static void put_i(const char *name, const char *arg, long long v)
{
    fprintf(g_out, "%-16s %-34s %lld\n", name, arg, v);
}

static void section(const char *title)
{
    fprintf(g_out, "---- %s ----\n", title);
}

static int open_out(const char *path, const char *title)
{
    g_out = fopen(path, "w");
    if (!g_out) {
        printf("cannot write %s\n", path);
        return 0;
    }
    fprintf(g_out, "================ %s ================\n", title);
    printf("writing %s\n", path);
    return 1;
}

/* ---------- input sets ---------- */

/* general values */
static const double d_gen[] = { 0.5, -0.5, 2.7, 187.66, 0.000123, 100.5 };
/* positive only: log, sqrt, gamma, bessel y */
static const double d_pos[] = { 0.5, 1.0, 2.7, 187.66, 0.000123, 100.5 };
/* inside [-1, 1]: acos, asin, atanh, erf */
static const double d_unit[] = { -0.9, -0.5, 0.0, 0.25, 0.5, 0.9 };
/* one and above: acosh */
static const double d_ge1[] = { 1.0, 1.5, 2.7, 10.0, 187.66, 1000.0 };

static const float f_gen[] = { 0.5f, -0.5f, 2.7f, 187.66f, 0.000123f, 100.5f };
static const float f_pos[] = { 0.5f, 1.0f, 2.7f, 187.66f, 0.000123f, 100.5f };
static const float f_unit[] = { -0.9f, -0.5f, 0.0f, 0.25f, 0.5f, 0.9f };
static const float f_ge1[] = { 1.0f, 1.5f, 2.7f, 10.0f, 187.66f, 1000.0f };

/* pairs for the two argument functions */
static const double d_pair[][2] = {
    { 0.4, 1.3 }, { 2.7, 11.9 }, { 187.66, -59.13 },
    { 0.000123, 9999.5 }, { 1.0, -1.0 }, { -3.5, 2.0 }
};
static const float f_pair[][2] = {
    { 0.4f, 1.3f }, { 2.7f, 11.9f }, { 187.66f, -59.13f },
    { 0.000123f, 9999.5f }, { 1.0f, -1.0f }, { -3.5f, 2.0f }
};

#define NGEN 6

/* ---------- drivers ---------- */

#define RUN_D1(fn, set)                                            \
    do {                                                           \
        int i;                                                     \
        for (i = 0; i < NGEN; ++i) {                               \
            char a[40];                                            \
            snprintf(a, sizeof a, "%.17g", set[i]);                \
            put_d(#fn, a, fn(set[i]));                             \
        }                                                          \
    } while (0)

#define RUN_F1(fn, set)                                            \
    do {                                                           \
        int i;                                                     \
        for (i = 0; i < NGEN; ++i) {                               \
            char a[40];                                            \
            snprintf(a, sizeof a, "%.9g", set[i]);                 \
            put_f(#fn, a, fn(set[i]));                             \
        }                                                          \
    } while (0)

#define RUN_D2(fn)                                                 \
    do {                                                           \
        int i;                                                     \
        for (i = 0; i < NGEN; ++i) {                               \
            char a[80];                                            \
            snprintf(a, sizeof a, "%.17g, %.17g",                  \
                     d_pair[i][0], d_pair[i][1]);                  \
            put_d(#fn, a, fn(d_pair[i][0], d_pair[i][1]));         \
        }                                                          \
    } while (0)

#define RUN_F2(fn)                                                 \
    do {                                                           \
        int i;                                                     \
        for (i = 0; i < NGEN; ++i) {                               \
            char a[80];                                            \
            snprintf(a, sizeof a, "%.9g, %.9g",                    \
                     f_pair[i][0], f_pair[i][1]);                  \
            put_f(#fn, a, fn(f_pair[i][0], f_pair[i][1]));         \
        }                                                          \
    } while (0)

#define RUN_ID1(fn, set)                                           \
    do {                                                           \
        int i;                                                     \
        for (i = 0; i < NGEN; ++i) {                               \
            char a[40];                                            \
            snprintf(a, sizeof a, "%.17g", set[i]);                \
            put_i(#fn, a, (long long)fn(set[i]));                  \
        }                                                          \
    } while (0)

#define RUN_IF1(fn, set)                                           \
    do {                                                           \
        int i;                                                     \
        for (i = 0; i < NGEN; ++i) {                               \
            char a[40];                                            \
            snprintf(a, sizeof a, "%.9g", set[i]);                 \
            put_i(#fn, a, (long long)fn(set[i]));                  \
        }                                                          \
    } while (0)

/* ---------- the run ---------- */

static void run_all(void)
{
    int i;

    section("double, trigonometric");
    RUN_D1(gm_sin, d_gen);
    RUN_D1(gm_cos, d_gen);
    RUN_D1(gm_tan, d_gen);
    RUN_D1(gm_asin, d_unit);
    RUN_D1(gm_acos, d_unit);
    RUN_D1(gm_atan, d_gen);
    RUN_D2(gm_atan2);

    section("double, hyperbolic");
    RUN_D1(gm_sinh, d_gen);
    RUN_D1(gm_cosh, d_gen);
    RUN_D1(gm_tanh, d_gen);
    RUN_D1(gm_asinh, d_gen);
    RUN_D1(gm_acosh, d_ge1);
    RUN_D1(gm_atanh, d_unit);

    section("double, exponential and logarithmic");
    RUN_D1(gm_exp, d_gen);
    RUN_D1(gm_exp2, d_gen);
    RUN_D1(gm_expm1, d_gen);
    RUN_D1(gm_log, d_pos);
    RUN_D1(gm_log10, d_pos);
    RUN_D1(gm_log1p, d_pos);
    RUN_D1(gm_logb, d_pos);
    RUN_D2(gm_pow);

    section("double, roots");
    RUN_D1(gm_sqrt, d_pos);
    RUN_D1(gm_cbrt, d_gen);
    RUN_D2(gm_hypot);

    section("double, rounding");
    RUN_D1(gm_ceil, d_gen);
    RUN_D1(gm_floor, d_gen);
    RUN_D1(gm_trunc, d_gen);
    RUN_D1(gm_round, d_gen);
    RUN_D1(gm_rint, d_gen);
    RUN_ID1(gm_lrint, d_gen);
    RUN_ID1(gm_lround, d_gen);
    RUN_ID1(gm_llrint, d_gen);

    section("double, sign and remainder");
    RUN_D1(gm_fabs, d_gen);
    RUN_D2(gm_fmod);
    RUN_D2(gm_remainder);
    RUN_D2(gm_drem);
    RUN_D2(gm_copysign);
    RUN_D2(gm_fmax);
    RUN_D2(gm_fmin);
    RUN_D2(gm_nextafter);

    section("double, special");
    RUN_D1(gm_erf, d_unit);
    RUN_D1(gm_erfc, d_unit);
    RUN_D1(gm_lgamma, d_pos);
    RUN_D1(gm_gamma, d_pos);
    RUN_D1(gm_significand, d_pos);
    RUN_D1(gm_j0, d_pos);
    RUN_D1(gm_y0, d_pos);
    RUN_ID1(gm_ilogb, d_pos);
    RUN_ID1(gm_isfinite, d_gen);
    RUN_ID1(gm_isnormal, d_gen);
    RUN_ID1(gm_finite, d_gen);

    section("double, out parameters and integer arguments");
    for (i = 0; i < NGEN; ++i) {
        char a[60];
        int e = 0;
        double m = gm_frexp(d_pos[i], &e);
        snprintf(a, sizeof a, "%.17g", d_pos[i]);
        put_d("gm_frexp.m", a, m);
        put_i("gm_frexp.e", a, e);
    }
    for (i = 0; i < NGEN; ++i) {
        char a[60];
        double ip = 0.0;
        double fr = gm_modf(d_pos[i], &ip);
        snprintf(a, sizeof a, "%.17g", d_pos[i]);
        put_d("gm_modf.fr", a, fr);
        put_d("gm_modf.ip", a, ip);
    }
    for (i = 0; i < NGEN; ++i) {
        char a[80];
        int q = 0;
        double r = gm_remquo(d_pair[i][0], d_pair[i][1], &q);
        snprintf(a, sizeof a, "%.17g, %.17g", d_pair[i][0], d_pair[i][1]);
        put_d("gm_remquo.r", a, r);
        put_i("gm_remquo.q", a, q);
    }
    for (i = 0; i < NGEN; ++i) {
        char a[60];
        snprintf(a, sizeof a, "%.17g, %d", d_pos[i], i - 3);
        put_d("gm_scalbn", a, gm_scalbn(d_pos[i], i - 3));
    }
    for (i = 0; i < NGEN; ++i) {
        char a[80];
        snprintf(a, sizeof a, "%.17g, %.17g", d_pair[i][0], d_pair[i][1]);
        put_d("gm_scalb", a, gm_scalb(d_pair[i][0], d_pair[i][1]));
    }
    for (i = 0; i < NGEN; ++i) {
        char a[80];
        snprintf(a, sizeof a, "%.17g, %.17g, %.17g",
                 d_pair[i][0], d_pair[i][1], d_gen[i]);
        put_d("gm_fma", a, gm_fma(d_pair[i][0], d_pair[i][1], d_gen[i]));
    }

    section("float, trigonometric");
    RUN_F1(gm_sinf, f_gen);
    RUN_F1(gm_cosf, f_gen);
    RUN_F1(gm_tanf, f_gen);
    RUN_F1(gm_asinf, f_unit);
    RUN_F1(gm_acosf, f_unit);
    RUN_F1(gm_atanf, f_gen);
    RUN_F2(gm_atan2f);

    section("float, hyperbolic");
    RUN_F1(gm_sinhf, f_gen);
    RUN_F1(gm_coshf, f_gen);
    RUN_F1(gm_tanhf, f_gen);
    RUN_F1(gm_asinhf, f_gen);
    RUN_F1(gm_acoshf, f_ge1);
    RUN_F1(gm_atanhf, f_unit);

    section("float, exponential and logarithmic");
    RUN_F1(gm_expf, f_gen);
    RUN_F1(gm_exp2f, f_gen);
    RUN_F1(gm_expm1f, f_gen);
    RUN_F1(gm_logf, f_pos);
    RUN_F1(gm_log10f, f_pos);
    RUN_F1(gm_log1pf, f_pos);
    RUN_F1(gm_logbf, f_pos);
    RUN_F2(gm_powf);

    section("float, roots");
    RUN_F1(gm_sqrtf, f_pos);
    RUN_F1(gm_cbrtf, f_gen);
    RUN_F2(gm_hypotf);

    section("float, rounding");
    RUN_F1(gm_ceilf, f_gen);
    RUN_F1(gm_floorf, f_gen);
    RUN_F1(gm_truncf, f_gen);
    RUN_F1(gm_roundf, f_gen);
    RUN_F1(gm_rintf, f_gen);
    RUN_IF1(gm_lrintf, f_gen);
    RUN_IF1(gm_lroundf, f_gen);
    RUN_IF1(gm_llrintf, f_gen);
    RUN_IF1(gm_llroundf, f_gen);

    section("float, sign and remainder");
    RUN_F1(gm_fabsf, f_gen);
    RUN_F2(gm_fmodf);
    RUN_F2(gm_remainderf);
    RUN_F2(gm_dremf);
    RUN_F2(gm_copysignf);
    RUN_F2(gm_fmaxf);
    RUN_F2(gm_fminf);

    section("float, special");
    RUN_F1(gm_erff, f_unit);
    RUN_F1(gm_erfcf, f_unit);
    RUN_F1(gm_lgammaf, f_pos);
    RUN_F1(gm_gammaf, f_pos);
    RUN_F1(gm_significandf, f_pos);
    RUN_F1(gm_j0f, f_pos);
    RUN_F1(gm_j1f, f_pos);
    RUN_F1(gm_y0f, f_pos);
    RUN_F1(gm_y1f, f_pos);
    RUN_IF1(gm_ilogbf, f_pos);
    RUN_IF1(gm_isfinitef, f_gen);
    RUN_IF1(gm_isnormalf, f_gen);
    RUN_IF1(gm_finitef, f_gen);
    RUN_IF1(gm_isnanf, f_gen);

    section("float, out parameters and integer arguments");
    for (i = 0; i < NGEN; ++i) {
        char a[60];
        int e = 0;
        float m = gm_frexpf(f_pos[i], &e);
        snprintf(a, sizeof a, "%.9g", f_pos[i]);
        put_f("gm_frexpf.m", a, m);
        put_i("gm_frexpf.e", a, e);
    }
    for (i = 0; i < NGEN; ++i) {
        char a[60];
        float ip = 0.0f;
        float fr = gm_modff(f_pos[i], &ip);
        snprintf(a, sizeof a, "%.9g", f_pos[i]);
        put_f("gm_modff.fr", a, fr);
        put_f("gm_modff.ip", a, ip);
    }
    for (i = 0; i < NGEN; ++i) {
        char a[60];
        snprintf(a, sizeof a, "%.9g, %d", f_pos[i], i - 3);
        put_f("gm_scalbnf", a, gm_scalbnf(f_pos[i], i - 3));
    }
    for (i = 0; i < NGEN; ++i) {
        char a[80];
        snprintf(a, sizeof a, "%.9g, %.9g", f_pair[i][0], f_pair[i][1]);
        put_f("gm_scalbf", a, gm_scalbf(f_pair[i][0], f_pair[i][1]));
    }
    for (i = 0; i < NGEN; ++i) {
        char a[60];
        snprintf(a, sizeof a, "%d, %.9g", i, f_pos[i]);
        put_f("gm_jnf", a, gm_jnf(i, f_pos[i]));
        put_f("gm_ynf", a, gm_ynf(i, f_pos[i]));
    }
    for (i = 0; i < NGEN; ++i) {
        char a[80];
        snprintf(a, sizeof a, "%.9g, %.9g, %.9g",
                 f_pair[i][0], f_pair[i][1], f_gen[i]);
        put_f("gm_fmaf", a, gm_fmaf(f_pair[i][0], f_pair[i][1], f_gen[i]));
    }
}

int main(void)
{
#ifdef _WIN32
    unsigned int cw;

    if (open_out("math-win-PC24.txt", "win32 x86, _PC_24")) {
        _controlfp_s(&cw, _PC_24, _MCW_PC);
        run_all();
        fclose(g_out);
    }

    if (open_out("math-win-PC53.txt", "win32 x86, _PC_53")) {
        _controlfp_s(&cw, _PC_53, _MCW_PC);
        run_all();
        fclose(g_out);
    }
#else
    if (open_out("math-mac.txt", "macOS ARM64")) {
        run_all();
        fclose(g_out);
    }
#endif
    puts("done");
    return 0;
}
