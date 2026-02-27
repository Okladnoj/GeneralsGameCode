/**
 * T1: Texture Format Conversion Tests
 * Verifies MetalFormatFromD3D() and BytesPerPixelFromD3D() mappings
 */

// ═══════════════ T1: MetalFormatFromD3D ═══════════════

TEST(T1_1_ARGB8_to_BGRA8) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_A8R8G8B8), MTLPixelFormatBGRA8Unorm);
}

TEST(T1_2_XRGB8_to_BGRA8) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_X8R8G8B8), MTLPixelFormatBGRA8Unorm);
}

TEST(T1_3_R5G6B5_to_BGRA8) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_R5G6B5), MTLPixelFormatBGRA8Unorm);
}

TEST(T1_4_A1R5G5B5_to_BGRA8) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_A1R5G5B5), MTLPixelFormatBGRA8Unorm);
}

TEST(T1_5_A4R4G4B4_to_BGRA8) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_A4R4G4B4), MTLPixelFormatBGRA8Unorm);
}

TEST(T1_6_DXT1_to_BC1) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_DXT1), MTLPixelFormatBC1_RGBA);
}

TEST(T1_7_DXT3_to_BC2) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_DXT3), MTLPixelFormatBC2_RGBA);
}

TEST(T1_8_DXT5_to_BC3) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_DXT5), MTLPixelFormatBC3_RGBA);
}

TEST(T1_9_L8_to_R8) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_L8), MTLPixelFormatR8Unorm);
}

TEST(T1_10_A8L8_to_RG8) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_A8L8), MTLPixelFormatRG8Unorm);
}

TEST(T1_11_A4L4_to_RG8) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_A4L4), MTLPixelFormatRG8Unorm);
}

TEST(T1_12_P8_to_R8) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_P8), MTLPixelFormatR8Unorm);
}

TEST(T1_13_R8G8B8_to_BGRA8) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_R8G8B8), MTLPixelFormatBGRA8Unorm);
}

TEST(T1_14_A8_to_A8Unorm) {
  ASSERT_EQ(MetalFormatFromD3D(D3DFMT_A8), MTLPixelFormatA8Unorm);
}

// ═══════════════ T1: BytesPerPixelFromD3D ═══════════════

TEST(T1_bpp_32bit_is_4) {
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_A8R8G8B8), 4u);
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_X8R8G8B8), 4u);
}

TEST(T1_bpp_16bit_is_2) {
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_R5G6B5), 2u);
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_A1R5G5B5), 2u);
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_A4R4G4B4), 2u);
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_A8L8), 2u);
}

TEST(T1_bpp_8bit_is_1) {
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_L8), 1u);
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_P8), 1u);
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_A4L4), 1u);
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_A8), 1u);
}

TEST(T1_bpp_24bit_is_3) {
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_R8G8B8), 3u);
}

TEST(T1_bpp_DXT1_block) {
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_DXT1), 8u);
}

TEST(T1_bpp_DXT3_5_block) {
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_DXT3), 16u);
  ASSERT_EQ(BytesPerPixelFromD3D(D3DFMT_DXT5), 16u);
}

// ═══════════════ T1: texFormatType ═══════════════

TEST(T1_texFormatType_luminance) {
  ASSERT_EQ(GetTexFormatType(D3DFMT_L8), 1u);
  ASSERT_EQ(GetTexFormatType(D3DFMT_P8), 1u);
}

TEST(T1_texFormatType_luminance_alpha) {
  ASSERT_EQ(GetTexFormatType(D3DFMT_A8L8), 2u);
  ASSERT_EQ(GetTexFormatType(D3DFMT_A4L4), 2u);
  ASSERT_EQ(GetTexFormatType(D3DFMT_A8P8), 2u);
}

TEST(T1_texFormatType_default) {
  ASSERT_EQ(GetTexFormatType(D3DFMT_A8R8G8B8), 0u);
  ASSERT_EQ(GetTexFormatType(D3DFMT_R5G6B5), 0u);
  ASSERT_EQ(GetTexFormatType(D3DFMT_DXT1), 0u);
}
