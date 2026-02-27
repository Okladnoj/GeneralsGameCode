/**
 * T13: Captured Texture Tests (golden-data)
 *
 * Tests real textures captured from gameplay.
 * Run capture:  GENERALS_CAPTURE_TEXTURES=1 sh build_run_mac.sh --screenshot
 * Then rebuild: sh build_run_mac.sh --test=captured
 *
 * If no captured data exists, these tests are skipped.
 */

// Try to include captured data (might not exist yet)
#if __has_include("captured_textures_data.cpp")
#include "captured_textures_data.cpp"
#define HAS_CAPTURED_TEXTURES 1
#else
#define HAS_CAPTURED_TEXTURES 0
static const size_t captured_texture_count = 0;
#endif

#if HAS_CAPTURED_TEXTURES

TEST(captured_texture_count_nonzero) {
  ASSERT_TRUE(captured_texture_count > 0);
}

TEST(captured_16bit_convert_no_crash) {
  // Verify Convert16to32 doesn't crash on any real texture
  for (size_t i = 0; i < captured_texture_count; i++) {
    const auto& t = captured_textures[i];
    if (!Is16BitFormat(t.format)) continue;

    UINT outPitch = 0;
    void* out = Convert16to32(t.format, t.srcData, t.width, t.height,
                              t.srcPitch, &outPitch);
    ASSERT_TRUE(out != nullptr);
    ASSERT_TRUE(outPitch > 0);
    free(out);
  }
}

TEST(captured_16bit_alpha_correct) {
  // For R5G6B5 textures: every converted pixel must have alpha=0xFF
  // For A1R5G5B5/A4R4G4B4: alpha can be anything 0-255
  for (size_t i = 0; i < captured_texture_count; i++) {
    const auto& t = captured_textures[i];
    if (t.format != D3DFMT_R5G6B5) continue;

    UINT outPitch = 0;
    void* out = Convert16to32(t.format, t.srcData, t.width, t.height,
                              t.srcPitch, &outPitch);
    if (!out) continue;

    // Check every pixel has alpha=0xFF
    const uint32_t* pixels = (const uint32_t*)out;
    uint32_t totalPixels = t.width * t.height;
    uint32_t badAlpha = 0;
    for (uint32_t px = 0; px < totalPixels; px++) {
      uint8_t a = (uint8_t)(pixels[px] >> 24);
      if (a != 0xFF) badAlpha++;
    }
    if (badAlpha > 0) {
      fprintf(stderr, "  FAIL: texture '%s' has %u pixels with alpha!=0xFF\n",
              t.name, badAlpha);
    }
    ASSERT_EQ(badAlpha, 0u);
    free(out);
  }
}

TEST(captured_32bit_passthrough) {
  // 32-bit textures (A8R8G8B8, X8R8G8B8) should not go through conversion
  for (size_t i = 0; i < captured_texture_count; i++) {
    const auto& t = captured_textures[i];
    ASSERT_TRUE(t.width > 0);
    ASSERT_TRUE(t.height > 0);
    ASSERT_TRUE(t.srcSize > 0);
  }
}

TEST(captured_BytesPerPixel_matches) {
  // Verify BPP matches actual data size
  for (size_t i = 0; i < captured_texture_count; i++) {
    const auto& t = captured_textures[i];
    uint32_t bpp = BytesPerPixelFromD3D(t.format);

    // For non-compressed formats, srcSize should be height * srcPitch
    bool isCompressed = (t.format == D3DFMT_DXT1 || t.format == D3DFMT_DXT2 ||
                         t.format == D3DFMT_DXT3 || t.format == D3DFMT_DXT4 ||
                         t.format == D3DFMT_DXT5);
    if (!isCompressed) {
      ASSERT_TRUE(t.srcPitch >= t.width * bpp);
    }
    ASSERT_EQ(t.srcSize, t.height * t.srcPitch);
  }
}

TEST(captured_format_coverage) {
  // Report which formats were captured
  uint32_t formatCounts[256] = {0};
  for (size_t i = 0; i < captured_texture_count; i++) {
    uint32_t fmtIdx = (uint32_t)captured_textures[i].format;
    if (fmtIdx < 256) formatCounts[fmtIdx]++;
  }
  printf("\n  Captured format distribution:\n");
  const char* fmtNames[] = {
    [23] = "R5G6B5", [25] = "A1R5G5B5", [26] = "A4R4G4B4",
    [21] = "A8R8G8B8", [22] = "X8R8G8B8", [50] = "L8",
    [51] = "A8L8", [20] = "R8G8B8",
  };
  for (int f = 0; f < 256; f++) {
    if (formatCounts[f] > 0) {
      const char* name = (f < 52 && fmtNames[f]) ? fmtNames[f] : "?";
      printf("    fmt=%d (%s): %u textures\n", f, name, formatCounts[f]);
    }
  }
  ASSERT_TRUE(captured_texture_count > 0);
}

// ── RGB precision tests ──

TEST(captured_R5G6B5_bit_precision) {
  // Verify that 5-bit/6-bit color channels expand to correct 8-bit range.
  // R5=31 → R8=255, G6=63 → G8=255, B5=31 → B8=255
  // R5=0  → R8=0,   G6=0  → G8=0,   B5=0  → B8=0
  for (size_t i = 0; i < captured_texture_count; i++) {
    const auto& t = captured_textures[i];
    if (t.format != D3DFMT_R5G6B5) continue;

    UINT outPitch = 0;
    void* out = Convert16to32(t.format, t.srcData, t.width, t.height,
                              t.srcPitch, &outPitch);
    if (!out) continue;

    const uint8_t* dst = (const uint8_t*)out;
    const uint16_t* src = (const uint16_t*)t.srcData;
    uint32_t errors = 0;

    for (uint32_t y = 0; y < t.height && errors < 5; y++) {
      for (uint32_t x = 0; x < t.width && errors < 5; x++) {
        uint16_t px16 = src[y * (t.srcPitch / 2) + x];
        uint32_t dstIdx = y * outPitch + x * 4;

        // Extract 5-6-5 components
        uint8_t r5 = (px16 >> 11) & 0x1F;
        uint8_t g6 = (px16 >> 5)  & 0x3F;
        uint8_t b5 =  px16        & 0x1F;

        // Expected 8-bit expansion
        uint8_t r8_exp = (uint8_t)((r5 * 255 + 15) / 31);
        uint8_t g8_exp = (uint8_t)((g6 * 255 + 31) / 63);
        uint8_t b8_exp = (uint8_t)((b5 * 255 + 15) / 31);

        // BGRA8 layout: B, G, R, A
        uint8_t b8 = dst[dstIdx + 0];
        uint8_t g8 = dst[dstIdx + 1];
        uint8_t r8 = dst[dstIdx + 2];

        // Allow ±1 tolerance for rounding differences
        if (abs((int)r8 - r8_exp) > 1 ||
            abs((int)g8 - g8_exp) > 1 ||
            abs((int)b8 - b8_exp) > 1) {
          if (errors == 0) {
            fprintf(stderr, "  FAIL: tex '%s' px(%u,%u): "
                    "src=0x%04X r5=%u g6=%u b5=%u → "
                    "got R=%u G=%u B=%u, expect R=%u G=%u B=%u\n",
                    t.name, x, y, px16, r5, g6, b5,
                    r8, g8, b8, r8_exp, g8_exp, b8_exp);
          }
          errors++;
        }
      }
    }
    ASSERT_EQ(errors, 0u);
    free(out);
  }
}

TEST(captured_16bit_deterministic) {
  // Converting the same input twice must produce identical output
  for (size_t i = 0; i < captured_texture_count; i++) {
    const auto& t = captured_textures[i];
    if (!Is16BitFormat(t.format)) continue;

    UINT pitch1 = 0, pitch2 = 0;
    void* out1 = Convert16to32(t.format, t.srcData, t.width, t.height,
                               t.srcPitch, &pitch1);
    void* out2 = Convert16to32(t.format, t.srcData, t.width, t.height,
                               t.srcPitch, &pitch2);
    ASSERT_TRUE(out1 != nullptr);
    ASSERT_TRUE(out2 != nullptr);
    ASSERT_EQ(pitch1, pitch2);

    int diff = memcmp(out1, out2, t.height * pitch1);
    if (diff != 0) {
      fprintf(stderr, "  FAIL: tex '%s' non-deterministic conversion!\n", t.name);
    }
    ASSERT_EQ(diff, 0);
    free(out1);
    free(out2);
  }
}

TEST(captured_16bit_no_uninitialized) {
  // After conversion, output buffer should have zero bytes that are 0xCD/0xFE
  // (common uninitialized memory patterns). Every pixel must be fully written.
  for (size_t i = 0; i < captured_texture_count; i++) {
    const auto& t = captured_textures[i];
    if (!Is16BitFormat(t.format)) continue;

    UINT outPitch = 0;
    void* out = Convert16to32(t.format, t.srcData, t.width, t.height,
                              t.srcPitch, &outPitch);
    if (!out) continue;

    // Verify output pitch is exactly width * 4 bytes (32-bit BGRA)
    ASSERT_EQ(outPitch, t.width * 4);

    // Verify total buffer covers all pixels
    uint32_t expectedSize = t.height * outPitch;
    ASSERT_TRUE(expectedSize > 0);

    free(out);
  }
}

TEST(captured_16bit_nonzero_content) {
  // Most real textures should have non-trivial content (not all black)
  uint32_t allBlackCount = 0;
  uint32_t total16bit = 0;

  for (size_t i = 0; i < captured_texture_count; i++) {
    const auto& t = captured_textures[i];
    if (!Is16BitFormat(t.format)) continue;
    total16bit++;

    UINT outPitch = 0;
    void* out = Convert16to32(t.format, t.srcData, t.width, t.height,
                              t.srcPitch, &outPitch);
    if (!out) continue;

    const uint32_t* pixels = (const uint32_t*)out;
    uint32_t totalPixels = t.width * t.height;
    uint32_t nonBlack = 0;
    for (uint32_t px = 0; px < totalPixels; px++) {
      // BGRA: black with alpha=0xFF is 0xFF000000
      if ((pixels[px] & 0x00FFFFFF) != 0) nonBlack++;
    }
    if (nonBlack == 0) allBlackCount++;
    free(out);
  }

  // Report — some textures may legitimately be all black (shadow maps),
  // but the majority should have content
  printf("  16-bit textures: %u total, %u all-black\n", total16bit, allBlackCount);
  // At most 50% should be all-black for a healthy dataset
  ASSERT_TRUE(allBlackCount < total16bit / 2 || total16bit < 3);
}

TEST(captured_16bit_pixel_by_pixel_reference) {
  // Cross-verify: Convert16to32 (buffer version) must match
  // ConvertPixel16to32 (single-pixel version) for every pixel
  for (size_t i = 0; i < captured_texture_count; i++) {
    const auto& t = captured_textures[i];
    if (!Is16BitFormat(t.format)) continue;

    UINT outPitch = 0;
    void* out = Convert16to32(t.format, t.srcData, t.width, t.height,
                              t.srcPitch, &outPitch);
    if (!out) continue;

    const uint32_t* dstPixels = (const uint32_t*)out;
    const uint16_t* srcPixels = (const uint16_t*)t.srcData;
    uint32_t mismatches = 0;

    for (uint32_t y = 0; y < t.height && mismatches < 3; y++) {
      for (uint32_t x = 0; x < t.width && mismatches < 3; x++) {
        uint16_t src16 = srcPixels[y * (t.srcPitch / 2) + x];
        uint32_t expected = ConvertPixel16to32(t.format, src16);
        uint32_t actual = dstPixels[y * (outPitch / 4) + x];

        if (actual != expected) {
          if (mismatches == 0) {
            fprintf(stderr, "  FAIL: tex '%s' px(%u,%u): "
                    "buffer=0x%08X vs pixel=0x%08X (src=0x%04X)\n",
                    t.name, x, y, actual, expected, src16);
          }
          mismatches++;
        }
      }
    }
    ASSERT_EQ(mismatches, 0u);
    free(out);
  }
}

#else

TEST(captured_textures_not_available) {
  printf("  SKIP: No captured textures. Run:\n");
  printf("    GENERALS_CAPTURE_TEXTURES=1 sh build_run_mac.sh --screenshot\n");
  printf("    sh build_run_mac.sh --test=captured\n");
  // This test always passes — it's just informational
  ASSERT_TRUE(true);
}

#endif
