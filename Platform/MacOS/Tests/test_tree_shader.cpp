/**
 * Test: Tree Vertex Shader Math
 * Validates the Metal tree vertex shader using REAL captured data from game.
 * 
 * This test simulates the Metal vertex_main shader (shaderType==1) and
 * fragment_main TSS path to verify:
 * 1. WVP transformation produces valid clip-space coordinates
 * 2. Sway weight calculation is correct (pos.z - normal.z)
 * 3. Alpha test passes for valid tree pixels
 * 4. TSS MODULATE produces expected color output
 * 5. The dark-pixel discard hack doesn't kill tree pixels
 */

#include <cmath>
#include <cstdio>
#include <cstdint>

// ═══ Captured data from TREE DRAW #1 ═══
// WVP matrix (c4..c7) — these are COLUMNS when used in Metal float4x4
static const float c4[4] = {2.1445f, 0.0000f, 0.0000f, -2280.6274f};
static const float c5[4] = {0.0000f, 2.4678f, 3.2161f, -1117.0780f};
static const float c6[4] = {0.0000f, 0.7940f, -0.6093f, 171.0482f};
static const float c7[4] = {0.0000f, 0.7934f, -0.6088f, 180.9057f};

// Sway vectors
static const float c8_nosway[4] = {0.0f, 0.0f, 0.0f, 0.0f};
static const float c9_sway1[4] = {0.0358f, 0.0207f, -0.0008f, 0.0f};

// Captured vertices: pos(xyz), normal(xyz), diffuse, uv
struct TreeVertex {
  float px, py, pz;
  float nx, ny, nz; // swayType, darkening, groundZ
  uint32_t diffuse;
  float u, v;
};

static const TreeVertex captured_vtx[] = {
  {1384.05f, 698.09f, 53.23f,  3.0f, 1.0f, 18.75f,  0xFFFFF2FE, 0.3527f, 0.0006f},
  {1406.22f, 697.86f, 53.23f,  3.0f, 1.0f, 18.75f,  0xFFFFF2FE, 0.2514f, 0.1236f},
  {1395.33f, 708.85f, 48.20f,  3.0f, 1.0f, 18.75f,  0xFFFFF3FF, 0.3526f, 0.1236f},
};
static const int num_captured_vtx = 3;

// Alpha test params
static const int alpha_test_enable = 1;
static const int alpha_func = 7; // D3DCMP_GREATEREQUAL
static const float alpha_ref = 96.0f / 255.0f; // 0.376

// Helper: dot product of 4-element vectors
static float dot4(const float a[4], const float b[4]) {
  return a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3];
}

// Simulate: out.position = swayedPos * wvpT
// In Metal, float4x4(c4,c5,c6,c7) puts them as COLUMNS
// pos * M = [dot(pos, col0), dot(pos, col1), dot(pos, col2), dot(pos, col3)]
static void transformWVP(const float pos[4], float out[4]) {
  out[0] = dot4(pos, c4); // x
  out[1] = dot4(pos, c5); // y
  out[2] = dot4(pos, c6); // z
  out[3] = dot4(pos, c7); // w
}

// Test: WVP transform produces valid clip-space coordinates
TEST(tree_wvp_transform_valid) {
  printf("\n");
  bool any_visible = false;
  for (int i = 0; i < num_captured_vtx; i++) {
    // Apply sway
    float swayWeight = captured_vtx[i].pz - captured_vtx[i].nz; // height above ground
    int swayType = (int)(captured_vtx[i].nx + 0.5f);
    if (swayType < 1) swayType = 1;
    if (swayType > 8) swayType = 8;
    // Use c9 for sway type 3 -> c[8+3] = c11 (but we only have c9 captured, use it)
    float sx = c9_sway1[0] * swayWeight;
    float sy = c9_sway1[1] * swayWeight;
    float sz = c9_sway1[2] * swayWeight;

    float swayedPos[4] = {
      captured_vtx[i].px + sx,
      captured_vtx[i].py + sy,
      captured_vtx[i].pz + sz,
      1.0f
    };

    float clip[4];
    transformWVP(swayedPos, clip);

    // NDC = clip / clip.w
    float ndc_x = clip[0] / clip[3];
    float ndc_y = clip[1] / clip[3];
    float ndc_z = clip[2] / clip[3];

    printf("  vtx[%d]: clip=[%.2f %.2f %.2f %.2f] NDC=[%.4f %.4f %.4f]",
           i, clip[0], clip[1], clip[2], clip[3], ndc_x, ndc_y, ndc_z);
    
    bool visible = (ndc_x >= -1.0f && ndc_x <= 1.0f &&
                    ndc_y >= -1.0f && ndc_y <= 1.0f &&
                    ndc_z >= 0.0f && ndc_z <= 1.0f);
    printf(" %s\n", visible ? "VISIBLE" : "CLIPPED");
    if (visible) any_visible = true;
  }
  // At least some vertices should be visible
  ASSERT_TRUE(any_visible, "All tree vertices are clipped! WVP transform may be wrong.");
}

// Test: Sway weight is height above ground, not raw groundZ
TEST(tree_sway_weight_is_height_above_ground) {
  for (int i = 0; i < num_captured_vtx; i++) {
    float swayWeight = captured_vtx[i].pz - captured_vtx[i].nz;
    float groundZ = captured_vtx[i].nz;
    printf("  vtx[%d]: pos.z=%.2f, groundZ=%.2f, swayWeight=%.2f\n",
           i, captured_vtx[i].pz, groundZ, swayWeight);
    // Sway weight should be between 0 and ~50 (height of tree)
    ASSERT_TRUE(swayWeight >= 0.0f, "Sway weight negative — tree vertex below ground?");
    ASSERT_TRUE(swayWeight < 100.0f, "Sway weight unreasonable — should be vertex height");
    // The OLD bug would use groundZ (~18.75) as swayWeight directly
    // which is wrong but wouldn't cause invisibility
  }
}

// Test: Alpha test with typical tree texture alpha values
TEST(tree_alpha_test_passes_opaque_pixels) {
  // Tree texture is A8R8G8B8 — opaque tree pixels have alpha = 255 = 1.0
  float opaque_alpha = 1.0f;
  // Alpha func 7 = D3DCMP_GREATEREQUAL, ref = 0.376
  bool passes = (opaque_alpha >= alpha_ref);
  ASSERT_TRUE(passes, "Opaque tree pixel FAILS alpha test!");
  
  // Transparent pixels should fail
  float transparent_alpha = 0.0f;
  bool fails = !(transparent_alpha >= alpha_ref);
  ASSERT_TRUE(fails, "Transparent pixel should NOT pass alpha test!");
  
  // Edge case: alpha = ref
  float edge_alpha = alpha_ref;
  bool edge_passes = (edge_alpha >= alpha_ref);
  ASSERT_TRUE(edge_passes, "Alpha exactly at ref should pass GREATEREQUAL");
}

// Test: TSS MODULATE result with tree diffuse
TEST(tree_tss_modulate_not_black) {
  // diffuse = 0xFFFFF2FE -> ARGB(FF, FF, F2, FE)
  // As normalized: R=1.0, G=0.949, B=0.996, A=1.0 (practically white)
  // Metal reads as BGRA: B=0xFE=0.996, G=0xF2=0.949, R=0xFF=1.0, A=0xFF=1.0
  float diffR = 1.0f, diffG = 0.949f, diffB = 0.996f;
  
  // Assume tree texture pixel is medium green: (0.3, 0.5, 0.2, 1.0)
  float texR = 0.3f, texG = 0.5f, texB = 0.2f, texA = 1.0f;
  
  // MODULATE: result = texture * diffuse
  float resR = texR * diffR;
  float resG = texG * diffG;
  float resB = texB * diffB;
  float resA = texA * 1.0f;
  
  float dotRGB = resR + resG + resB;
  printf("  MODULATE result: [%.3f %.3f %.3f %.3f] dot=%.4f\n", 
         resR, resG, resB, resA, dotRGB);
  
  // The dark pixel discard check: dot(current.rgb, float3(1.0)) < 0.001
  ASSERT_TRUE(dotRGB >= 0.001, "Tree pixel killed by dark-pixel discard hack!");
}

// Test: Dark pixel discard hack analysis
TEST(tree_dark_discard_hack_analysis, "Check if dark discard could kill trees") {
  // Even the darkest possible green tree leaf (0.01, 0.02, 0.01, 1.0)
  // dot = 0.04 > 0.001 — should NOT be discarded
  float darkLeaf = 0.01f + 0.02f + 0.01f;
  ASSERT_TRUE(darkLeaf > 0.001, "Even darkest leaf should survive discard hack");
  
  // BUT: what about tree texture background (transparent area)?
  // If alpha test discards these first, they never reach the hack.
  // alpha test: alpha=0.0 < ref=0.376 → discarded by alpha test ✓
  
  // What about pure black opaque pixels?
  float pureBlack = 0.0f + 0.0f + 0.0f;
  bool blackDiscarded = (pureBlack < 0.001);
  printf("  Pure black opaque pixel would be discarded: %s\n",
         blackDiscarded ? "YES — this is the hack's purpose" : "NO");
  ASSERT_TRUE(blackDiscarded, "Hack should discard pure black");
}

// Test: Clip-space W must be positive (in front of camera)
TEST(tree_clip_w_positive) {
  for (int i = 0; i < num_captured_vtx; i++) {
    float pos[4] = {captured_vtx[i].px, captured_vtx[i].py, captured_vtx[i].pz, 1.0f};
    float clip[4];
    transformWVP(pos, clip);
    printf("  vtx[%d]: w=%.2f %s\n", i, clip[3], clip[3] > 0 ? "OK" : "BEHIND CAMERA!");
    ASSERT_TRUE(clip[3] > 0.0f, "Tree vertex behind camera (w <= 0)!");
  }
}
