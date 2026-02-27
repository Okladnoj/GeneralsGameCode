/**
 * DX8 → Metal Bridge Test Suite
 */

// DX8 type definitions (via macOS shims)
#include <d3d8.h>

// Bridge headers under test
#include "MetalBridgeMappings.h"
#include "MetalFormatConvert.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <string>

// ─────────────────────────────────────────────────────
//  Test Framework
// ─────────────────────────────────────────────────────

struct TestCase {
  const char* name;
  void (*func)();
};

static std::vector<TestCase>& GetTests() {
  static std::vector<TestCase> tests;
  return tests;
}

static int g_testsPassed = 0;
static int g_testsFailed = 0;
static int g_assertsPassed = 0;
static int g_assertsFailed = 0;
static bool g_currentTestFailed = false;

struct TestRegistrar {
  TestRegistrar(const char* name, void (*func)()) {
    GetTests().push_back({name, func});
  }
};

// TEST macro supports: TEST(name) or TEST(name, "description")
#define TEST_1(name) \
  static void test_##name(); \
  static TestRegistrar reg_##name(#name, test_##name); \
  static void test_##name()

#define TEST_2(name, desc) \
  static void test_##name(); \
  static TestRegistrar reg_##name(#name, test_##name); \
  static void test_##name()

#define GET_TEST_MACRO(_1, _2, MACRO, ...) MACRO
#define TEST(...) GET_TEST_MACRO(__VA_ARGS__, TEST_2, TEST_1)(__VA_ARGS__)

#define ASSERT_EQ(a, b) do { \
  auto _a = (a); auto _b = (b); \
  if (_a != _b) { \
    fprintf(stderr, "  FAIL: %s:%d: ASSERT_EQ(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
    fprintf(stderr, "    got:      %lld (0x%llx)\n", (long long)_a, (unsigned long long)_a); \
    fprintf(stderr, "    expected: %lld (0x%llx)\n", (long long)_b, (unsigned long long)_b); \
    g_assertsFailed++; g_currentTestFailed = true; \
  } else { g_assertsPassed++; } \
} while(0)

#define ASSERT_NEQ(a, b) do { \
  auto _a = (a); auto _b = (b); \
  if (_a == _b) { \
    fprintf(stderr, "  FAIL: %s:%d: ASSERT_NEQ(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
    fprintf(stderr, "    both are: %lld (0x%llx)\n", (long long)_a, (unsigned long long)_a); \
    g_assertsFailed++; g_currentTestFailed = true; \
  } else { g_assertsPassed++; } \
} while(0)

#define ASSERT_TRUE_1(expr) do { \
  if (!(expr)) { \
    fprintf(stderr, "  FAIL: %s:%d: ASSERT_TRUE(%s)\n", __FILE__, __LINE__, #expr); \
    g_assertsFailed++; g_currentTestFailed = true; \
  } else { g_assertsPassed++; } \
} while(0)

#define ASSERT_TRUE_2(expr, msg) do { \
  if (!(expr)) { \
    fprintf(stderr, "  FAIL: %s:%d: %s\n", __FILE__, __LINE__, msg); \
    fprintf(stderr, "    assertion: %s\n", #expr); \
    g_assertsFailed++; g_currentTestFailed = true; \
  } else { g_assertsPassed++; } \
} while(0)

#define GET_ASSERT_TRUE_MACRO(_1, _2, MACRO, ...) MACRO
#define ASSERT_TRUE(...) GET_ASSERT_TRUE_MACRO(__VA_ARGS__, ASSERT_TRUE_2, ASSERT_TRUE_1)(__VA_ARGS__)

#define ASSERT_FALSE(expr) do { \
  if ((expr)) { \
    fprintf(stderr, "  FAIL: %s:%d: ASSERT_FALSE(%s)\n", __FILE__, __LINE__, #expr); \
    g_assertsFailed++; g_currentTestFailed = true; \
  } else { g_assertsPassed++; } \
} while(0)

#define ASSERT_NEAR(a, b, eps) do { \
  float _a = (float)(a); float _b = (float)(b); float _e = (float)(eps); \
  if (fabsf(_a - _b) > _e) { \
    fprintf(stderr, "  FAIL: %s:%d: ASSERT_NEAR(%s, %s, %s)\n", __FILE__, __LINE__, #a, #b, #eps); \
    fprintf(stderr, "    got:      %f\n", _a); \
    fprintf(stderr, "    expected: %f (±%f)\n", _b, _e); \
    g_assertsFailed++; g_currentTestFailed = true; \
  } else { g_assertsPassed++; } \
} while(0)

// ─────────────────────────────────────────────────────
//  Test files are included here (they use TEST() macro)
// ─────────────────────────────────────────────────────
#include "test_texture_format.cpp"
#include "test_pixel_conversion.cpp"
#include "test_blend_state.cpp"
#include "test_render_state.cpp"
#include "test_sampler_state.cpp"
#include "test_color_conversion.cpp"
#include "test_uniform_packing.cpp"
#include "test_fvf_layout.cpp"
#include "test_tss_formulas.cpp"
#include "test_fog_formulas.cpp"
#include "test_captured_textures.cpp"
#include "test_device_caps.cpp"
#include "test_tree_shader.cpp"

// ─────────────────────────────────────────────────────
//  Main
// ─────────────────────────────────────────────────────
int main(int argc, char** argv) {
  const char* filter = (argc > 1) ? argv[1] : nullptr;

  printf("═══════════════════════════════════════════════\n");
  printf("  DX8 → Metal Bridge Test Suite\n");
  printf("═══════════════════════════════════════════════\n\n");

  auto& tests = GetTests();
  int skipped = 0;

  for (auto& tc : tests) {
    if (filter && !strstr(tc.name, filter)) {
      skipped++;
      continue;
    }

    g_currentTestFailed = false;
    printf("● %s ... ", tc.name);
    fflush(stdout);
    tc.func();
    if (g_currentTestFailed) {
      printf("FAILED\n");
      g_testsFailed++;
    } else {
      printf("OK\n");
      g_testsPassed++;
    }
  }

  printf("\n═══════════════════════════════════════════════\n");
  printf("  Results: %d passed, %d failed", g_testsPassed, g_testsFailed);
  if (skipped > 0) printf(", %d skipped", skipped);
  printf("\n");
  printf("  Asserts: %d passed, %d failed\n", g_assertsPassed, g_assertsFailed);
  printf("═══════════════════════════════════════════════\n");

  return g_testsFailed > 0 ? 1 : 0;
}
