/**
 * T8: Color Conversion Tests
 * Tests D3DCOLOR packing and BGRA byte order
 */

// D3DCOLOR_ARGB macro (from DX8 spec, not always in our stub)
#ifndef D3DCOLOR_ARGB
#define D3DCOLOR_ARGB(a,r,g,b) \
  ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif

#ifndef D3DCOLOR_RGBA
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
#endif

TEST(T8_1_D3DCOLOR_ARGB_red) {
  D3DCOLOR c = D3DCOLOR_ARGB(255, 255, 0, 0);
  ASSERT_EQ(c, (D3DCOLOR)0xFFFF0000);
}

TEST(T8_2_D3DCOLOR_to_BGRA_bytes) {
  // D3DCOLOR 0xFFFF0000 = A=0xFF, R=0xFF, G=0x00, B=0x00
  // In memory (little-endian): byte[0]=0x00(B), byte[1]=0x00(G), byte[2]=0xFF(R), byte[3]=0xFF(A)
  D3DCOLOR c = 0xFFFF0000;
  uint8_t* bytes = (uint8_t*)&c;
  ASSERT_EQ(bytes[0], 0x00); // B
  ASSERT_EQ(bytes[1], 0x00); // G
  ASSERT_EQ(bytes[2], 0xFF); // R
  ASSERT_EQ(bytes[3], 0xFF); // A
}

TEST(T8_3_D3DCOLOR_BGRA_byte_order) {
  // D3DCOLOR 0xFF804020 = A=0xFF, R=0x80, G=0x40, B=0x20
  D3DCOLOR c = 0xFF804020;
  uint8_t* bytes = (uint8_t*)&c;
  // On little-endian (which macOS/ARM is):
  // bytes[0] = LSB = 0x20 (B)
  // bytes[1] = 0x40 (G)
  // bytes[2] = 0x80 (R)
  // bytes[3] = MSB = 0xFF (A)
  ASSERT_EQ(bytes[0], 0x20); // B
  ASSERT_EQ(bytes[1], 0x40); // G
  ASSERT_EQ(bytes[2], 0x80); // R
  ASSERT_EQ(bytes[3], 0xFF); // A
}

TEST(T8_4_D3DCOLOR_shader_channels) {
  // MTLVertexFormatUChar4Normalized_BGRA reads {B,G,R,A} bytes
  // and maps them to shader (R,G,B,A).
  // For D3DCOLOR 0xFF804020:
  //   memory bytes: B=0x20, G=0x40, R=0x80, A=0xFF
  //   shader sees: R=0x80/255, G=0x40/255, B=0x20/255, A=1.0
  D3DCOLOR c = 0xFF804020;
  uint8_t* bytes = (uint8_t*)&c;
  
  // Shader R channel = memory byte for R = bytes[2] = 0x80
  float shaderR = bytes[2] / 255.0f;
  float shaderG = bytes[1] / 255.0f;
  float shaderB = bytes[0] / 255.0f;
  float shaderA = bytes[3] / 255.0f;
  
  ASSERT_NEAR(shaderR, 0x80 / 255.0f, 0.001f);
  ASSERT_NEAR(shaderG, 0x40 / 255.0f, 0.001f);
  ASSERT_NEAR(shaderB, 0x20 / 255.0f, 0.001f);
  ASSERT_NEAR(shaderA, 1.0f, 0.001f);
}

TEST(T8_ARGB_packing_green) {
  D3DCOLOR c = D3DCOLOR_ARGB(128, 0, 255, 0);
  ASSERT_EQ(c, (D3DCOLOR)0x8000FF00);
}

TEST(T8_ARGB_packing_blue) {
  D3DCOLOR c = D3DCOLOR_ARGB(64, 0, 0, 255);
  ASSERT_EQ(c, (D3DCOLOR)0x400000FF);
}
