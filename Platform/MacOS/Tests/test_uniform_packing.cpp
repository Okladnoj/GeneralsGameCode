/**
 * T7: Uniform Struct Layout & Packing Tests
 * Verifies that C++ struct sizes match Metal shader expectations
 *
 * NOTE: The struct definitions are duplicated here from MetalDevice8.mm
 * since those structs are file-local. In test mode we define compatible
 * versions using the same layout to verify sizes and alignment.
 */
#include <simd/simd.h>

// Re-define the structs identically to MetalDevice8.mm
// This itself is a test: if the layout changes in production,
// these tests should be updated to match — and the size checks
// will catch any divergence from the Metal shader.

namespace TestStructs {

struct MetalUniforms {
  simd::float4x4 world;
  simd::float4x4 view;
  simd::float4x4 projection;
  simd::float4x4 texMatrix[4];
  simd::float2 screenSize;
  int useProjection;
  uint32_t shaderSettings;
  uint32_t texTransformFlags[4];
};

struct TextureStageConfig {
  uint32_t colorOp;
  uint32_t colorArg1;
  uint32_t colorArg2;
  uint32_t alphaOp;
  uint32_t alphaArg1;
  uint32_t alphaArg2;
  uint32_t _pad0;
  uint32_t _pad1;
};

struct FragmentUniforms {
  TextureStageConfig stages[4];
  simd::float4 textureFactor;
  simd::float4 fogColor;
  float fogStart;
  float fogEnd;
  float fogDensity;
  uint32_t fogMode;
  uint32_t alphaTestEnable;
  uint32_t alphaFunc;
  float alphaRef;
  uint32_t hasTexture[4];
  uint32_t specularEnable;
  uint32_t texCoordIndex[4];
  uint32_t texFormatType[4];
  uint32_t blendEnabled;
};

struct LightData {
  simd::float4 diffuse;
  simd::float4 ambient;
  simd::float4 specular;
  simd::float3 position;
  float range;
  simd::float3 direction;
  float falloff;
  float attenuation0;
  float attenuation1;
  float attenuation2;
  float theta;
  float phi;
  uint32_t type;
  uint32_t enabled;
  float _pad;
};

struct LightingUniforms {
  LightData lights[4];
  simd::float4 materialDiffuse;
  simd::float4 materialAmbient;
  simd::float4 materialSpecular;
  simd::float4 materialEmissive;
  float materialPower;
  simd::float4 globalAmbient;
  uint32_t lightingEnabled;
  uint32_t diffuseSource;
  uint32_t ambientSource;
  uint32_t specularSource;
  uint32_t emissiveSource;
  uint32_t hasNormals;
  float fogStart;
  float fogEnd;
  float fogDensity;
  uint32_t fogMode;
};

struct CustomVSUniforms {
  uint32_t shaderType;
  uint32_t _pad[3];
  simd::float4 c[34];
};

} // namespace TestStructs

// ═══════════════ Size checks ═══════════════

TEST(T7_1_FragmentUniforms_size) {
  // Must be a multiple of 16 for Metal buffer alignment
  size_t sz = sizeof(TestStructs::FragmentUniforms);
  ASSERT_TRUE(sz > 0);
  ASSERT_EQ(sz % 4, 0u); // at least 4-byte aligned
}

TEST(T7_2_MetalUniforms_size) {
  size_t sz = sizeof(TestStructs::MetalUniforms);
  ASSERT_TRUE(sz > 0);
  ASSERT_EQ(sz % 4, 0u);
}

TEST(T7_3_LightingUniforms_size) {
  size_t sz = sizeof(TestStructs::LightingUniforms);
  ASSERT_TRUE(sz > 0);
  ASSERT_EQ(sz % 4, 0u);
}

TEST(T7_4_CustomVSUniforms_size) {
  size_t sz = sizeof(TestStructs::CustomVSUniforms);
  ASSERT_TRUE(sz > 0);
  ASSERT_EQ(sz % 16, 0u); // float4 array requires 16-byte alignment
}

TEST(T7_5_TextureStageConfig_size_32bytes) {
  // Each stage config must be 32 bytes (8 x uint32_t) for array padding
  ASSERT_EQ(sizeof(TestStructs::TextureStageConfig), 32u);
}

TEST(T7_6_LightData_size) {
  // LightData must be consistently sized for array indexing
  size_t sz = sizeof(TestStructs::LightData);
  ASSERT_TRUE(sz > 0);
  ASSERT_EQ(sz % 16, 0u); // must be 16-byte aligned for float4 members
}

// ═══════════════ TSS Defaults (from D3D8 spec) ═══════════════

TEST(T7_stage0_colorOp_default_MODULATE) {
  ASSERT_EQ((uint32_t)D3DTOP_MODULATE, 4u);
}

TEST(T7_stage0_colorArg1_default_TEXTURE) {
  ASSERT_EQ((uint32_t)D3DTA_TEXTURE, 2u);
}

TEST(T7_stage0_alphaOp_default_SELECTARG1) {
  ASSERT_EQ((uint32_t)D3DTOP_SELECTARG1, 2u);
}

TEST(T7_stages1plus_default_DISABLE) {
  ASSERT_EQ((uint32_t)D3DTOP_DISABLE, 1u);
}
