/**
 * T5: Render State Mapping Tests
 * Verifies cull mode, compare function, stencil op, and color write mask mappings
 */

// ═══════════════ Cull Mode ═══════════════

TEST(T5_1_CULL_NONE) {
  ASSERT_EQ(MapD3DCullToMTL(D3DCULL_NONE), MTLCullModeNone);
}

TEST(T5_2_CULL_CW_to_Front) {
  ASSERT_EQ(MapD3DCullToMTL(D3DCULL_CW), MTLCullModeFront);
}

TEST(T5_3_CULL_CCW_to_Back) {
  ASSERT_EQ(MapD3DCullToMTL(D3DCULL_CCW), MTLCullModeBack);
}

// ═══════════════ Compare Function ═══════════════

TEST(T5_4_CMP_NEVER) {
  ASSERT_EQ(MapD3DCmpToMTL(D3DCMP_NEVER), MTLCompareFunctionNever);
}

TEST(T5_4_CMP_LESS) {
  ASSERT_EQ(MapD3DCmpToMTL(D3DCMP_LESS), MTLCompareFunctionLess);
}

TEST(T5_4_CMP_EQUAL) {
  ASSERT_EQ(MapD3DCmpToMTL(D3DCMP_EQUAL), MTLCompareFunctionEqual);
}

TEST(T5_4_CMP_LESSEQUAL) {
  ASSERT_EQ(MapD3DCmpToMTL(D3DCMP_LESSEQUAL), MTLCompareFunctionLessEqual);
}

TEST(T5_4_CMP_GREATER) {
  ASSERT_EQ(MapD3DCmpToMTL(D3DCMP_GREATER), MTLCompareFunctionGreater);
}

TEST(T5_4_CMP_NOTEQUAL) {
  ASSERT_EQ(MapD3DCmpToMTL(D3DCMP_NOTEQUAL), MTLCompareFunctionNotEqual);
}

TEST(T5_4_CMP_GREATEREQUAL) {
  ASSERT_EQ(MapD3DCmpToMTL(D3DCMP_GREATEREQUAL), MTLCompareFunctionGreaterEqual);
}

TEST(T5_4_CMP_ALWAYS) {
  ASSERT_EQ(MapD3DCmpToMTL(D3DCMP_ALWAYS), MTLCompareFunctionAlways);
}

TEST(T5_4_CMP_default) {
  ASSERT_EQ(MapD3DCmpToMTL(999), MTLCompareFunctionLessEqual);
}

// ═══════════════ Stencil Operations ═══════════════

TEST(T5_stencil_KEEP) {
  ASSERT_EQ(MapD3DStencilOpToMTL(D3DSTENCILOP_KEEP), MTLStencilOperationKeep);
}

TEST(T5_stencil_ZERO) {
  ASSERT_EQ(MapD3DStencilOpToMTL(D3DSTENCILOP_ZERO), MTLStencilOperationZero);
}

TEST(T5_stencil_REPLACE) {
  ASSERT_EQ(MapD3DStencilOpToMTL(D3DSTENCILOP_REPLACE), MTLStencilOperationReplace);
}

TEST(T5_stencil_INCRSAT) {
  ASSERT_EQ(MapD3DStencilOpToMTL(D3DSTENCILOP_INCRSAT), MTLStencilOperationIncrementClamp);
}

TEST(T5_stencil_DECRSAT) {
  ASSERT_EQ(MapD3DStencilOpToMTL(D3DSTENCILOP_DECRSAT), MTLStencilOperationDecrementClamp);
}

TEST(T5_stencil_INVERT) {
  ASSERT_EQ(MapD3DStencilOpToMTL(D3DSTENCILOP_INVERT), MTLStencilOperationInvert);
}

TEST(T5_stencil_INCR) {
  ASSERT_EQ(MapD3DStencilOpToMTL(D3DSTENCILOP_INCR), MTLStencilOperationIncrementWrap);
}

TEST(T5_stencil_DECR) {
  ASSERT_EQ(MapD3DStencilOpToMTL(D3DSTENCILOP_DECR), MTLStencilOperationDecrementWrap);
}

// ═══════════════ Color Write Mask ═══════════════

TEST(T5_6_colorwrite_all) {
  ASSERT_EQ(MapD3DColorWriteToMTL(0xF), MTLColorWriteMaskAll);
}

TEST(T5_7_colorwrite_RGB_only) {
  MTLColorWriteMask expected = MTLColorWriteMaskRed | MTLColorWriteMaskGreen | MTLColorWriteMaskBlue;
  ASSERT_EQ(MapD3DColorWriteToMTL(0x7), expected);
}

TEST(T5_colorwrite_zero_defaults_to_all) {
  // cwMask=0 should default to writing all channels
  ASSERT_EQ(MapD3DColorWriteToMTL(0x0), MTLColorWriteMaskAll);
}

TEST(T5_colorwrite_alpha_only) {
  ASSERT_EQ(MapD3DColorWriteToMTL(0x8), MTLColorWriteMaskAlpha);
}
