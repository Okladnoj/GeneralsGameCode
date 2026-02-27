/**
 * T10: Fog Factor Computation Tests
 */

static float computeLinearFog(float d, float start, float end) {
  if (end == start) return 1.0f;
  float f = (end - d) / (end - start);
  return f < 0.0f ? 0.0f : (f > 1.0f ? 1.0f : f);
}

static float computeExpFog(float d, float density) {
  return expf(-density * d);
}

static float computeExp2Fog(float d, float density) {
  return expf(-(density * d) * (density * d));
}

TEST(T10_1_linear_fog_mid) {
  ASSERT_NEAR(computeLinearFog(50, 10, 100), 0.5555f, 0.001f);
}
TEST(T10_2_linear_fog_at_start) {
  ASSERT_NEAR(computeLinearFog(10, 10, 100), 1.0f, 0.001f);
}
TEST(T10_3_linear_fog_at_end) {
  ASSERT_NEAR(computeLinearFog(100, 10, 100), 0.0f, 0.001f);
}
TEST(T10_4_linear_fog_beyond) {
  ASSERT_NEAR(computeLinearFog(150, 10, 100), 0.0f, 0.001f);
}
TEST(T10_5_exp_fog) {
  ASSERT_NEAR(computeExpFog(10, 0.1f), 0.3679f, 0.001f);
}
TEST(T10_6_exp2_fog) {
  ASSERT_NEAR(computeExp2Fog(10, 0.1f), 0.3679f, 0.001f);
}
TEST(T10_7_fog_application) {
  float f = 0.5f;
  float objColor = 1.0f, fogColor = 0.0f;
  float result = f * objColor + (1.0f - f) * fogColor;
  ASSERT_NEAR(result, 0.5f, 0.001f);
}
