/**
 * T4: FVF → Vertex Descriptor Layout Tests
 */

TEST(T4_1_XYZ_DIFFUSE_TEX1) {
  DWORD fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
  FVFLayoutResult r = ComputeFVFLayout(fvf);
  ASSERT_TRUE(r.position.present);
  ASSERT_EQ(r.position.format, (uint32_t)MTLVertexFormatFloat3);
  ASSERT_EQ(r.position.offset, 0u);
  ASSERT_EQ(r.diffuse.offset, 12u);
  ASSERT_EQ(r.texCoord0.offset, 16u);
  ASSERT_EQ(r.computedStride, 24u);
}

TEST(T4_2_terrain_XYZ_NORMAL_DIFFUSE_TEX2) {
  DWORD fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2;
  FVFLayoutResult r = ComputeFVFLayout(fvf);
  ASSERT_EQ(r.position.offset, 0u);
  ASSERT_EQ(r.normal.offset, 12u);
  ASSERT_EQ(r.diffuse.offset, 24u);
  ASSERT_EQ(r.texCoord0.offset, 28u);
  ASSERT_EQ(r.texCoord1.offset, 36u);
  ASSERT_EQ(r.computedStride, 44u);
}

TEST(T4_3_2D_XYZRHW_DIFFUSE_TEX1) {
  DWORD fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1;
  FVFLayoutResult r = ComputeFVFLayout(fvf);
  ASSERT_EQ(r.position.format, (uint32_t)MTLVertexFormatFloat4);
  ASSERT_EQ(r.position.offset, 0u);
  ASSERT_EQ(r.diffuse.offset, 16u);
  ASSERT_EQ(r.texCoord0.offset, 20u);
  ASSERT_EQ(r.computedStride, 28u);
}

TEST(T4_4_particles_XYZRHW_DIFFUSE_TEX2) {
  DWORD fvf = D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX2;
  FVFLayoutResult r = ComputeFVFLayout(fvf);
  ASSERT_EQ(r.position.offset, 0u);
  ASSERT_EQ(r.diffuse.offset, 16u);
  ASSERT_EQ(r.texCoord0.offset, 20u);
  ASSERT_EQ(r.texCoord1.offset, 28u);
  ASSERT_EQ(r.computedStride, 36u);
}

TEST(T4_5_FVF_memory_order) {
  DWORD fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | 0x080 | D3DFVF_TEX1;
  FVFLayoutResult r = ComputeFVFLayout(fvf);
  ASSERT_TRUE(r.position.offset < r.normal.offset);
  ASSERT_TRUE(r.normal.offset < r.diffuse.offset);
  ASSERT_TRUE(r.diffuse.offset < r.specular.offset);
  ASSERT_TRUE(r.specular.offset < r.texCoord0.offset);
}

TEST(T4_6_missing_diffuse_buffer30) {
  DWORD fvf = D3DFVF_XYZ | D3DFVF_TEX1;
  FVFLayoutResult r = ComputeFVFLayout(fvf);
  ASSERT_FALSE(r.diffuse.present);
  ASSERT_EQ(r.diffuse.bufIndex, 30u);
}
