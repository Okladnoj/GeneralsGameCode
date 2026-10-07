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
 * Cast matrix for GameMath, run on the same input value four ways:
 *
 *   .d     double function, double result          gm_f(x)
 *   .f     float function, float result            gm_ff((float)x)
 *   .f2d   float function, result widened          (double)gm_ff((float)x)
 *   .d2f   double function, result narrowed        (float)gm_f(x)
 *
 * The raw bits of every result are compared with the snapshot of the platform
 * the test runs on, in Snapshots/math-<platform>.txt. A snapshot records the
 * results as they are, not as they should be: the platforms differ from each
 * other, and a failing test means that the results on this platform changed.
 *
 * On 32-bit x86 the whole matrix runs twice, once with the x87 precision
 * control set to _PC_24 and once with _PC_53, against a snapshot for each.
 *
 * A failing test writes the results it got to math-<platform>.txt in the
 * working directory, in the snapshot format, so the snapshot can be replaced
 * with it once the change is understood.
 */

#include <gtest/gtest.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <float.h>
#endif

#include "gmath.h"

/* ---------- output ---------- */

static std::vector<std::string> g_lines;

static void put_d(const char *name, const char *arg, double v)
{
    unsigned long long u;
    char line[128];
    memcpy(&u, &v, sizeof u);
    snprintf(line, sizeof line, "%-20s %-46s %016llX", name, arg, u);
    g_lines.push_back(line);
}

static void put_f(const char *name, const char *arg, float v)
{
    unsigned int u;
    char line[128];
    memcpy(&u, &v, sizeof u);
    snprintf(line, sizeof line, "%-20s %-46s %08X", name, arg, u);
    g_lines.push_back(line);
}

static void section(const char *title)
{
    char line[128];
    snprintf(line, sizeof line, "---- %s ----", title);
    g_lines.push_back(line);
}

/* ---------- input sets ---------- */

/* general values */
static const double v_gen[] = { 0.5, -0.5, 2.7, 187.66, 0.000123, 100.5 };
/* positive only: log, sqrt, gamma, bessel y */
static const double v_pos[] = { 0.5, 1.0, 2.7, 187.66, 0.000123, 100.5 };
/* inside [-1, 1]: asin, acos, atanh, erf */
static const double v_unit[] = { -0.9, -0.5, 0.0, 0.25, 0.5, 0.9 };
/* one and above: acosh */
static const double v_ge1[] = { 1.0, 1.5, 2.7, 10.0, 187.66, 1000.0 };

static const double v_pair[][2] = {
    { 0.4, 1.3 }, { 2.7, 11.9 }, { 187.66, -59.13 },
    { 0.000123, 9999.5 }, { 1.0, -1.0 }, { -3.5, 2.0 }
};

#define NV 6

/* ---------- one argument ---------- */

#define MATRIX1(base, set)                                                  \
    do {                                                                    \
        int i;                                                              \
        for (i = 0; i < NV; ++i) {                                          \
            double x = set[i];                                              \
            float  xf = (float)x;                                           \
            char a[48];                                                     \
            snprintf(a, sizeof a, "%.17g", x);                              \
            put_d(#base ".d",   a, gm_##base(x));                           \
            put_f(#base ".f",   a, gm_##base##f(xf));                       \
            put_d(#base ".f2d", a, (double)gm_##base##f(xf));               \
            put_f(#base ".d2f", a, (float)gm_##base(x));                    \
        }                                                                   \
        for (i = 0; i < NV; ++i) {                                          \
            double d1 = gm_##base(set[i]);                                  \
            double d2 = gm_##base(set[(i + 1) % NV]);                       \
            double d3 = gm_##base(set[(i + 2) % NV]);                       \
            float  s1 = gm_##base##f((float)set[i]);                        \
            float  s2 = gm_##base##f((float)set[(i + 1) % NV]);             \
            float  s3 = gm_##base##f((float)set[(i + 2) % NV]);             \
            char a[48];                                                     \
            snprintf(a, sizeof a, "%.17g", set[i]);                         \
            put_d(#base ".mul.d", a, d1 * d2);                              \
            put_f(#base ".mul.f", a, s1 * s2);                              \
            put_d(#base ".inv.d", a, 1.0 / d1);                             \
            put_f(#base ".inv.f", a, 1.0f / s1);                            \
            put_d(#base ".mad.d", a, d1 * d2 + d3);                         \
            put_f(#base ".mad.f", a, s1 * s2 + s3);                         \
        }                                                                   \
    } while (0)

/* ---------- two arguments ----------
 *
 * An argument may reach the call as a full double or as a value that has
 * already been through a float, so both arguments are varied independently,
 * and every combination is recorded with the result kept as double and with
 * the result narrowed to float:
 *
 *   .dd     gm_f(x, y)
 *   .dd2f   (float)gm_f(x, y)
 *   .df     gm_f(x, (double)(float)y)
 *   .df2f   (float)gm_f(x, (double)(float)y)
 *   .fd     gm_f((double)(float)x, y)
 *   .fd2f   (float)gm_f((double)(float)x, y)
 *   .ff     gm_f((double)(float)x, (double)(float)y)
 *   .ff2f   (float)gm_f((double)(float)x, (double)(float)y)
 *   .f      gm_ff((float)x, (float)y)
 *   .f2d    (double)gm_ff((float)x, (float)y)
 */

#define MATRIX2(base)                                                       \
    do {                                                                    \
        int i;                                                              \
        for (i = 0; i < NV; ++i) {                                          \
            double x = v_pair[i][0], y = v_pair[i][1];                      \
            float  xf = (float)x, yf = (float)y;                            \
            double xd = (double)xf, yd = (double)yf;                        \
            char a[64];                                                     \
            snprintf(a, sizeof a, "%.17g, %.17g", x, y);                    \
            put_d(#base ".dd",   a, gm_##base(x, y));                       \
            put_f(#base ".dd2f", a, (float)gm_##base(x, y));                \
            put_d(#base ".df",   a, gm_##base(x, yd));                      \
            put_f(#base ".df2f", a, (float)gm_##base(x, yd));               \
            put_d(#base ".fd",   a, gm_##base(xd, y));                      \
            put_f(#base ".fd2f", a, (float)gm_##base(xd, y));               \
            put_d(#base ".ff",   a, gm_##base(xd, yd));                     \
            put_f(#base ".ff2f", a, (float)gm_##base(xd, yd));              \
            put_f(#base ".f",    a, gm_##base##f(xf, yf));                  \
            put_d(#base ".f2d",  a, (double)gm_##base##f(xf, yf));          \
        }                                                                   \
        for (i = 0; i < NV; ++i) {                                          \
            int j = (i + 1) % NV, k = (i + 2) % NV;                         \
            double d1 = gm_##base(v_pair[i][0], v_pair[i][1]);              \
            double d2 = gm_##base(v_pair[j][0], v_pair[j][1]);              \
            double d3 = gm_##base(v_pair[k][0], v_pair[k][1]);              \
            float  s1 = gm_##base##f((float)v_pair[i][0],                   \
                                     (float)v_pair[i][1]);                  \
            float  s2 = gm_##base##f((float)v_pair[j][0],                   \
                                     (float)v_pair[j][1]);                  \
            float  s3 = gm_##base##f((float)v_pair[k][0],                   \
                                     (float)v_pair[k][1]);                  \
            char a[64];                                                     \
            snprintf(a, sizeof a, "%.17g, %.17g",                           \
                     v_pair[i][0], v_pair[i][1]);                           \
            put_d(#base ".mul.d", a, d1 * d2);                              \
            put_f(#base ".mul.f", a, s1 * s2);                              \
            put_d(#base ".inv.d", a, 1.0 / d1);                             \
            put_f(#base ".inv.f", a, 1.0f / s1);                            \
            put_d(#base ".mad.d", a, d1 * d2 + d3);                         \
            put_f(#base ".mad.f", a, s1 * s2 + s3);                         \
        }                                                                   \
    } while (0)

/* ---------- the run ---------- */

static void run_all(void)
{
    section("trigonometric");
    MATRIX1(sin, v_gen);
    MATRIX1(cos, v_gen);
    MATRIX1(tan, v_gen);
    MATRIX1(asin, v_unit);
    MATRIX1(acos, v_unit);
    MATRIX1(atan, v_gen);
    MATRIX2(atan2);

    section("hyperbolic");
    MATRIX1(sinh, v_gen);
    MATRIX1(cosh, v_gen);
    MATRIX1(tanh, v_gen);
    MATRIX1(asinh, v_gen);
    MATRIX1(acosh, v_ge1);
    MATRIX1(atanh, v_unit);

    section("exponential and logarithmic");
    MATRIX1(exp, v_gen);
    MATRIX1(exp2, v_gen);
    MATRIX1(expm1, v_gen);
    MATRIX1(log, v_pos);
    MATRIX1(log10, v_pos);
    MATRIX1(log1p, v_pos);
    MATRIX1(logb, v_pos);
    MATRIX2(pow);

    section("roots");
    MATRIX1(sqrt, v_pos);
    MATRIX1(cbrt, v_gen);
    MATRIX2(hypot);

    section("rounding");
    MATRIX1(ceil, v_gen);
    MATRIX1(floor, v_gen);
    MATRIX1(trunc, v_gen);
    MATRIX1(round, v_gen);
    MATRIX1(rint, v_gen);

    section("sign and remainder");
    MATRIX1(fabs, v_gen);
    MATRIX2(fmod);
    MATRIX2(remainder);
    MATRIX2(drem);
    MATRIX2(copysign);
    MATRIX2(fmax);
    MATRIX2(fmin);

    section("special");
    MATRIX1(erf, v_unit);
    MATRIX1(erfc, v_unit);
    MATRIX1(lgamma, v_pos);
    MATRIX1(gamma, v_pos);
    MATRIX1(significand, v_pos);
    MATRIX1(j0, v_pos);
    MATRIX1(y0, v_pos);
}

/* ---------- what we are running on ----------
 *
 * The snapshot name carries the configuration, so each platform is compared
 * with its own results.
 */

#if defined(_M_IX86) || defined(__i386__)
#define ARCH_TAG "x86"
#elif defined(_M_X64) || defined(_M_AMD64) || defined(__x86_64__)
#define ARCH_TAG "x64"
#elif defined(_M_ARM64) || defined(__aarch64__) || defined(__arm64__)
#define ARCH_TAG "arm64"
#else
#define ARCH_TAG "unknown"
#endif

#ifdef _WIN32
#if defined(_M_FP_STRICT)
#define FP_TAG "strict"
#elif defined(_M_FP_FAST)
#define FP_TAG "fast"
#elif defined(_M_FP_PRECISE)
#define FP_TAG "precise"
#else
#define FP_TAG "fpdefault"
#endif
#define PLAT_TAG "win"
#else
#define FP_TAG "clang"
#define PLAT_TAG "mac"
#endif

#define BASE_TAG PLAT_TAG "-" ARCH_TAG "-" FP_TAG

/* ---------- snapshots ---------- */

#define MAX_SHOWN_DIFFERENCES 20

static bool read_snapshot(const std::string &path, std::vector<std::string> &lines)
{
    std::ifstream file(path.c_str());
    if (!file) {
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        if (line.compare(0, 16, "================") == 0) {
            continue;
        }
        lines.push_back(line);
    }
    return true;
}

static void write_results(const char *name, const char *title)
{
    FILE *out = fopen(name, "wb");
    if (!out) {
        return;
    }

    fprintf(out, "================ %s ================\n", title);
    for (size_t i = 0; i < g_lines.size(); ++i) {
        fprintf(out, "%s\n", g_lines[i].c_str());
    }
    fclose(out);
}

static std::string describe_differences(const std::vector<std::string> &snapshot)
{
    std::string text;
    size_t differing = 0;

    for (size_t i = 0; i < snapshot.size(); ++i) {
        if (snapshot[i] == g_lines[i]) {
            continue;
        }
        if (differing < MAX_SHOWN_DIFFERENCES) {
            text += "  snapshot: " + snapshot[i] + "\n";
            text += "  result:   " + g_lines[i] + "\n";
        }
        ++differing;
    }
    return std::to_string(differing) + " of " + std::to_string(snapshot.size()) + " rows differ\n" + text;
}

static void expect_snapshot(const char *name, const char *title)
{
    const std::string path = std::string(GAMEMATH_SNAPSHOT_DIR) + "/" + name;
    std::vector<std::string> snapshot;

    g_lines.clear();
    run_all();

    const bool found = read_snapshot(path, snapshot);
    if (found && g_lines == snapshot) {
        return;
    }

    write_results(name, title);
    ASSERT_TRUE(found) << "No snapshot at " << path << "; the results were written to " << name;
    ASSERT_EQ(g_lines.size(), snapshot.size()) << "Row count differs from " << path << "; the results were written to " << name;
    ADD_FAILURE() << "Results differ from " << path << "; they were written to " << name << "\n" << describe_differences(snapshot);
}

/* ---------- the tests ---------- */

/* x87 precision control only exists on 32-bit x86. */
#if defined(_WIN32) && (defined(_M_IX86) || defined(__i386__))

class X87Precision
{
public:
    explicit X87Precision(unsigned int precision)
    {
        unsigned int unused;
        _controlfp_s(&m_previous, 0, 0);
        _controlfp_s(&unused, precision, _MCW_PC);
    }

    ~X87Precision()
    {
        unsigned int unused;
        _controlfp_s(&unused, m_previous & _MCW_PC, _MCW_PC);
    }

private:
    unsigned int m_previous;
};

TEST(GameMath, MatchesSnapshotWithPrecision24)
{
    X87Precision precision(_PC_24);
    expect_snapshot("math-" BASE_TAG "-PC24.txt", BASE_TAG ", _PC_24");
}

TEST(GameMath, MatchesSnapshotWithPrecision53)
{
    X87Precision precision(_PC_53);
    expect_snapshot("math-" BASE_TAG "-PC53.txt", BASE_TAG ", _PC_53");
}

#else

TEST(GameMath, MatchesSnapshot)
{
    expect_snapshot("math-" BASE_TAG ".txt", BASE_TAG);
}

#endif
