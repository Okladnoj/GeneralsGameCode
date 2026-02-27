/**
 * T6: TSS Operations (D3DTOP Formulas) - CPU reference tests
 */

static float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

struct float4 { float r, g, b, a; };

static float4 evaluateOp(uint32_t op, float4 arg1, float4 arg2) {
  float4 result = {0,0,0,0};
  switch (op) {
  case D3DTOP_SELECTARG1: result = arg1; break;
  case D3DTOP_SELECTARG2: result = arg2; break;
  case D3DTOP_MODULATE:
    result = {arg1.r*arg2.r, arg1.g*arg2.g, arg1.b*arg2.b, arg1.a*arg2.a}; break;
  case D3DTOP_MODULATE2X:
    result = {clamp01(arg1.r*arg2.r*2), clamp01(arg1.g*arg2.g*2), clamp01(arg1.b*arg2.b*2), clamp01(arg1.a*arg2.a*2)}; break;
  case D3DTOP_ADD:
    result = {clamp01(arg1.r+arg2.r), clamp01(arg1.g+arg2.g), clamp01(arg1.b+arg2.b), clamp01(arg1.a+arg2.a)}; break;
  case D3DTOP_SUBTRACT:
    result = {clamp01(arg1.r-arg2.r), clamp01(arg1.g-arg2.g), clamp01(arg1.b-arg2.b), clamp01(arg1.a-arg2.a)}; break;
  case D3DTOP_ADDSIGNED:
    result = {clamp01(arg1.r+arg2.r-0.5f), clamp01(arg1.g+arg2.g-0.5f), clamp01(arg1.b+arg2.b-0.5f), clamp01(arg1.a+arg2.a-0.5f)}; break;
  case D3DTOP_ADDSMOOTH:
    result = {clamp01(arg1.r+arg2.r-arg1.r*arg2.r), clamp01(arg1.g+arg2.g-arg1.g*arg2.g),
              clamp01(arg1.b+arg2.b-arg1.b*arg2.b), clamp01(arg1.a+arg2.a-arg1.a*arg2.a)}; break;
  default: break;
  }
  return result;
}

TEST(T6_1_SELECTARG1) {
  float4 r = evaluateOp(D3DTOP_SELECTARG1, {1,0,0,1}, {0,1,0,0.5f});
  ASSERT_NEAR(r.r, 1.0f, 0.001f); ASSERT_NEAR(r.a, 1.0f, 0.001f);
}

TEST(T6_2_SELECTARG2) {
  float4 r = evaluateOp(D3DTOP_SELECTARG2, {1,0,0,1}, {0,1,0,0.5f});
  ASSERT_NEAR(r.g, 1.0f, 0.001f); ASSERT_NEAR(r.a, 0.5f, 0.001f);
}

TEST(T6_3_MODULATE) {
  float4 r = evaluateOp(D3DTOP_MODULATE, {0.5f,1,0,1}, {1,0.5f,1,0.5f});
  ASSERT_NEAR(r.r, 0.5f, 0.001f); ASSERT_NEAR(r.g, 0.5f, 0.001f);
  ASSERT_NEAR(r.b, 0.0f, 0.001f); ASSERT_NEAR(r.a, 0.5f, 0.001f);
}

TEST(T6_6_ADD) {
  float4 r = evaluateOp(D3DTOP_ADD, {0.5f,0,0,0.5f}, {0.3f,0,0,0.3f});
  ASSERT_NEAR(r.r, 0.8f, 0.001f); ASSERT_NEAR(r.a, 0.8f, 0.001f);
}

TEST(T6_8_SUBTRACT) {
  float4 r = evaluateOp(D3DTOP_SUBTRACT, {1,0,0,1}, {0.3f,0,0,0.5f});
  ASSERT_NEAR(r.r, 0.7f, 0.001f); ASSERT_NEAR(r.a, 0.5f, 0.001f);
}

TEST(T6_9_ADDSMOOTH) {
  float4 r = evaluateOp(D3DTOP_ADDSMOOTH, {0.5f,0,0,1}, {0.5f,0,0,0});
  ASSERT_NEAR(r.r, 0.75f, 0.001f); ASSERT_NEAR(r.a, 1.0f, 0.001f);
}
