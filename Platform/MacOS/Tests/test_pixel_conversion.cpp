/**
 * T2: Pixel Data Conversion Tests (16→32 bit)
 * Verifies ConvertPixel16to32() and Convert16to32() buffer conversion
 *
 * CRITICAL for smoke bug: Tests 2.4-2.9 verify alpha handling
 */

// Helper: extract BGRA bytes from packed uint32_t
static uint8_t BGRA_B(uint32_t c) { return (uint8_t)(c & 0xFF); }
static uint8_t BGRA_G(uint32_t c) { return (uint8_t)((c >> 8) & 0xFF); }
static uint8_t BGRA_R(uint32_t c) { return (uint8_t)((c >> 16) & 0xFF); }
static uint8_t BGRA_A(uint32_t c) { return (uint8_t)((c >> 24) & 0xFF); }

// ═══════════════ T2: R5G6B5 ═══════════════

TEST(T2_1_R5G6B5_white) {
  uint32_t c = ConvertPixel16to32(D3DFMT_R5G6B5, 0xFFFF);
  ASSERT_EQ(BGRA_R(c), 0xFF);
  ASSERT_EQ(BGRA_G(c), 0xFF);
  ASSERT_EQ(BGRA_B(c), 0xFF);
  ASSERT_EQ(BGRA_A(c), 0xFF);
}

TEST(T2_2_R5G6B5_black) {
  uint32_t c = ConvertPixel16to32(D3DFMT_R5G6B5, 0x0000);
  ASSERT_EQ(BGRA_R(c), 0x00);
  ASSERT_EQ(BGRA_G(c), 0x00);
  ASSERT_EQ(BGRA_B(c), 0x00);
  ASSERT_EQ(BGRA_A(c), 0xFF);
}

TEST(T2_3_R5G6B5_pure_red) {
  // R5G6B5: RRRR RGGG GGGB BBBB = 1111 1000 0000 0000 = 0xF800
  uint32_t c = ConvertPixel16to32(D3DFMT_R5G6B5, 0xF800);
  ASSERT_EQ(BGRA_R(c), 0xFF);
  ASSERT_EQ(BGRA_G(c), 0x00);
  ASSERT_EQ(BGRA_B(c), 0x00);
  ASSERT_EQ(BGRA_A(c), 0xFF);
}

TEST(T2_4_R5G6B5_alpha_always_FF) {
  // CRITICAL: R5G6B5 has no alpha channel — must always be 0xFF
  // This is the root cause of the smoke bug if wrong
  for (uint16_t px = 0; px < 256; px += 17) {
    uint32_t c = ConvertPixel16to32(D3DFMT_R5G6B5, px);
    ASSERT_EQ(BGRA_A(c), 0xFF);
  }
}

TEST(T2_R5G6B5_pure_green) {
  // R5G6B5: 0000 0111 1110 0000 = 0x07E0
  uint32_t c = ConvertPixel16to32(D3DFMT_R5G6B5, 0x07E0);
  ASSERT_EQ(BGRA_R(c), 0x00);
  ASSERT_EQ(BGRA_G(c), 0xFF);
  ASSERT_EQ(BGRA_B(c), 0x00);
}

TEST(T2_R5G6B5_pure_blue) {
  // R5G6B5: 0000 0000 0001 1111 = 0x001F
  uint32_t c = ConvertPixel16to32(D3DFMT_R5G6B5, 0x001F);
  ASSERT_EQ(BGRA_R(c), 0x00);
  ASSERT_EQ(BGRA_G(c), 0x00);
  ASSERT_EQ(BGRA_B(c), 0xFF);
}

// ═══════════════ T2: A1R5G5B5 ═══════════════

TEST(T2_5_A1R5G5B5_alpha_1) {
  // A=1, R=31, G=31, B=31 = 0xFFFF
  uint32_t c = ConvertPixel16to32(D3DFMT_A1R5G5B5, 0xFFFF);
  ASSERT_EQ(BGRA_A(c), 0xFF);  // 1-bit alpha=1 → expanded to 255
}

TEST(T2_6_A1R5G5B5_alpha_0) {
  // A=0, R=31, G=31, B=31 = 0x7FFF
  uint32_t c = ConvertPixel16to32(D3DFMT_A1R5G5B5, 0x7FFF);
  ASSERT_EQ(BGRA_A(c), 0x00);  // 1-bit alpha=0 → 0
}

// ═══════════════ T2: A4R4G4B4 ═══════════════

TEST(T2_7_A4R4G4B4_alpha_F) {
  // A=0xF, R=0xF, G=0xF, B=0xF = 0xFFFF
  uint32_t c = ConvertPixel16to32(D3DFMT_A4R4G4B4, 0xFFFF);
  ASSERT_EQ(BGRA_A(c), 0xFF);
}

TEST(T2_8_A4R4G4B4_alpha_8) {
  // A=0x8, R=0xF, G=0xF, B=0xF = 0x8FFF
  uint32_t c = ConvertPixel16to32(D3DFMT_A4R4G4B4, 0x8FFF);
  ASSERT_EQ(BGRA_A(c), (uint8_t)(8 * 255 / 15)); // = 136 = 0x88
}

TEST(T2_9_A4R4G4B4_alpha_0) {
  // A=0x0, R=0xF, G=0xF, B=0xF = 0x0FFF
  uint32_t c = ConvertPixel16to32(D3DFMT_A4R4G4B4, 0x0FFF);
  ASSERT_EQ(BGRA_A(c), 0x00);
}

TEST(T2_A4R4G4B4_color_channels) {
  // A=0xA, R=0x5, G=0x3, B=0x7 = 0xA537
  uint32_t c = ConvertPixel16to32(D3DFMT_A4R4G4B4, 0xA537);
  ASSERT_EQ(BGRA_R(c), (uint8_t)(5 * 255 / 15));
  ASSERT_EQ(BGRA_G(c), (uint8_t)(3 * 255 / 15));
  ASSERT_EQ(BGRA_B(c), (uint8_t)(7 * 255 / 15));
  ASSERT_EQ(BGRA_A(c), (uint8_t)(0xA * 255 / 15));
}

// ═══════════════ T2: Is16BitFormat ═══════════════

TEST(T2_is16bit_true) {
  ASSERT_TRUE(Is16BitFormat(D3DFMT_R5G6B5));
  ASSERT_TRUE(Is16BitFormat(D3DFMT_X1R5G5B5));
  ASSERT_TRUE(Is16BitFormat(D3DFMT_A1R5G5B5));
  ASSERT_TRUE(Is16BitFormat(D3DFMT_A4R4G4B4));
}

TEST(T2_is16bit_false) {
  ASSERT_FALSE(Is16BitFormat(D3DFMT_A8R8G8B8));
  ASSERT_FALSE(Is16BitFormat(D3DFMT_X8R8G8B8));
  ASSERT_FALSE(Is16BitFormat(D3DFMT_L8));
  ASSERT_FALSE(Is16BitFormat(D3DFMT_DXT1));
}

// ═══════════════ T2: Buffer conversion ═══════════════

TEST(T2_convert16to32_buffer) {
  // Create a tiny 2x1 R5G6B5 image: [red, blue]
  uint16_t src[2] = {0xF800, 0x001F};
  UINT outPitch = 0;
  void* dst = Convert16to32(D3DFMT_R5G6B5, src, 2, 1, 4, &outPitch);
  ASSERT_TRUE(dst != nullptr);
  ASSERT_EQ(outPitch, 8u); // 2 pixels * 4 bytes

  uint32_t* pixels = (uint32_t*)dst;
  // Pixel 0: pure red
  ASSERT_EQ(BGRA_R(pixels[0]), 0xFF);
  ASSERT_EQ(BGRA_G(pixels[0]), 0x00);
  ASSERT_EQ(BGRA_B(pixels[0]), 0x00);
  // Pixel 1: pure blue
  ASSERT_EQ(BGRA_R(pixels[1]), 0x00);
  ASSERT_EQ(BGRA_G(pixels[1]), 0x00);
  ASSERT_EQ(BGRA_B(pixels[1]), 0xFF);

  free(dst);
}
