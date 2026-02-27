/**
 * test_device_caps.cpp — Tests that the Metal bridge reports correct DX8
 * device capabilities.
 *
 * WHY THIS MATTERS:
 *   The W3D engine uses device capabilities to decide texture formats.
 *   If the bridge reports wrong bitdepth, display format, or format support,
 *   textures get silently downgraded (e.g., A8R8G8B8 -> R5G6B5) and alpha
 *   is lost. This causes trees, particles, smoke to render as opaque squares.
 *
 * DEPENDS ON:
 *   captured_textures_data.cpp with HAS_CAPTURED_DEVICE_CAPS define
 */

#ifdef HAS_CAPTURED_DEVICE_CAPS

TEST(caps_display_format_32bit,
     "Display format must be 32-bit")
{
  printf("  Display: %ux%u fmt=%u bits=%u\n",
         captured_display_width, captured_display_height,
         captured_display_format, captured_display_bits);

  ASSERT_TRUE(captured_display_bits >= 32,
              "Display bit depth must be >= 32. "
              "16-bit causes texture format degradation!");

  // D3DFMT_A8R8G8B8 = 21, D3DFMT_X8R8G8B8 = 22
  ASSERT_TRUE(captured_display_format == 21 || captured_display_format == 22,
              "Display format must be A8R8G8B8(21) or X8R8G8B8(22)!");
}

TEST(caps_support_A8R8G8B8, "Must support A8R8G8B8")
{
  printf("  A8R8G8B8 supported: %s\n", captured_support_A8R8G8B8 ? "YES" : "NO");
  ASSERT_TRUE(captured_support_A8R8G8B8, "A8R8G8B8 must be supported!");
}

TEST(caps_support_A4R4G4B4, "Must support A4R4G4B4")
{
  printf("  A4R4G4B4 supported: %s\n", captured_support_A4R4G4B4 ? "YES" : "NO");
  ASSERT_TRUE(captured_support_A4R4G4B4, "A4R4G4B4 must be supported!");
}

TEST(caps_support_A1R5G5B5, "Must support A1R5G5B5")
{
  printf("  A1R5G5B5 supported: %s\n", captured_support_A1R5G5B5 ? "YES" : "NO");
  ASSERT_TRUE(captured_support_A1R5G5B5, "A1R5G5B5 must be supported!");
}

TEST(caps_support_R5G6B5, "Must support R5G6B5")
{
  printf("  R5G6B5 supported: %s\n", captured_support_R5G6B5 ? "YES" : "NO");
  ASSERT_TRUE(captured_support_R5G6B5, "R5G6B5 must be supported!");
}

TEST(caps_support_X8R8G8B8, "Must support X8R8G8B8")
{
  printf("  X8R8G8B8 supported: %s\n", captured_support_X8R8G8B8 ? "YES" : "NO");
  ASSERT_TRUE(captured_support_X8R8G8B8, "X8R8G8B8 must be supported!");
}

TEST(caps_max_texture_size, "Max texture >= 2048")
{
  printf("  Max texture: %ux%u\n", captured_max_tex_width, captured_max_tex_height);
  ASSERT_TRUE(captured_max_tex_width >= 2048, "MaxTextureWidth must be >= 2048!");
  ASSERT_TRUE(captured_max_tex_height >= 2048, "MaxTextureHeight must be >= 2048!");
}

TEST(caps_alpha_format_not_degraded,
     "Alpha format must NOT degrade to R5G6B5")
{
  bool alpha_preserved = false;

  if (captured_display_bits > 16) {
    alpha_preserved = captured_support_A8R8G8B8;
    printf("  32-bit path: A8R8G8B8=%s -> alpha %s\n",
           captured_support_A8R8G8B8 ? "yes" : "no",
           alpha_preserved ? "PRESERVED" : "LOST");
  } else {
    alpha_preserved = captured_support_A4R4G4B4;
    printf("  16-bit path: A4R4G4B4=%s -> alpha %s\n",
           captured_support_A4R4G4B4 ? "yes" : "no",
           alpha_preserved ? "PRESERVED" : "LOST");
  }

  ASSERT_TRUE(alpha_preserved,
              "Alpha format degraded! Trees/particles will be opaque squares.");
}

TEST(caps_captured_alpha_texture_health,
     "Captured textures should include alpha formats")
{
  int fmt21 = 0, fmt25 = 0, fmt26 = 0, fmt23 = 0, other = 0;
  for (size_t i = 0; i < captured_texture_count; i++) {
    switch ((int)captured_textures[i].format) {
      case 21: fmt21++; break;
      case 25: fmt25++; break;
      case 26: fmt26++; break;
      case 23: fmt23++; break;
      default: other++; break;
    }
  }
  int alpha_cap = fmt21 + fmt25 + fmt26;
  printf("  A8R8G8B8=%d A4R4G4B4=%d A1R5G5B5=%d R5G6B5=%d other=%d\n",
         fmt21, fmt26, fmt25, fmt23, other);
  printf("  Alpha-capable: %d / %zu (%.1f%%)\n",
         alpha_cap, captured_texture_count,
         captured_texture_count > 0
             ? (100.0 * alpha_cap / captured_texture_count) : 0.0);

  ASSERT_TRUE(alpha_cap > 0,
              "No alpha textures! Format degradation likely.");
  if (captured_texture_count >= 50) {
    double pct = 100.0 * alpha_cap / captured_texture_count;
    // Most game textures are 24-bit TGA (no alpha) -> R5G6B5 is expected.
    // But engine-generated atlases (trees, terrain, radar) MUST have alpha.
    // A healthy system has at least ~3-5% alpha-capable textures.
    ASSERT_TRUE(pct > 3.0,
                "Less than 3% alpha textures - check tree/terrain/UI atlases.");
  }
}

#else
// Caps data not available — skip gracefully
TEST(caps_not_available, "Device caps (need re-capture)")
{
  printf("  Caps data not in captured file. Re-run:\n");
  printf("  GENERALS_CAPTURE_TEXTURES=1 sh build_run_mac.sh --screenshot\n");
}
#endif
