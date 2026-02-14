#include <metal_stdlib>
using namespace metal;

struct Uniforms {
    float4x4 world;
    float4x4 view;
    float4x4 projection;
    float2 screenSize;
    int useProjection;
    uint shaderSettings;
};

struct VertexIn {
    float3 position [[attribute(0)]];
    float4 color    [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
};

struct VertexOut {
    float4 position [[position]];
    float4 color;
    float2 texCoord;
    float fogFactor;
};

// Shader Settings Bit Positions (from shader.h)
#define SHIFT_DEPTHCOMPARE      0
#define SHIFT_DEPTHMASK         3
#define SHIFT_COLORMASK         4
#define SHIFT_DSTBLEND          5
#define SHIFT_FOG               8
#define SHIFT_PRIGRADIENT       10
#define SHIFT_SECGRADIENT       13
#define SHIFT_SRCBLEND          14
#define SHIFT_TEXTURING         16
#define SHIFT_ALPHATEST         18
#define SHIFT_POSTDETAILCOLOR   20
#define SHIFT_POSTDETAILALPHA   24

vertex VertexOut vertex_main(VertexIn in [[stage_in]],
                            constant Uniforms &uniforms [[buffer(1)]]) {
    VertexOut out;
    
    float4 pos = float4(in.position, 1.0);
    
    if (uniforms.useProjection == 1) {
        // 3D Mode
        out.position = uniforms.projection * uniforms.view * uniforms.world * pos;
    } else if (uniforms.useProjection == 2) {
        // 2D Mode (Screen Space)
        float2 screenPos = (in.position.xy / uniforms.screenSize) * 2.0 - 1.0;
        out.position = float4(screenPos.x, -screenPos.y, 0.0, 1.0);
    } else {
        out.position = pos;
    }
    
    out.color = in.color;
    out.texCoord = in.texCoord;
    
    // Simple fog calculation
    float4 viewPos = uniforms.view * uniforms.world * pos;
    out.fogFactor = clamp(-viewPos.z / 500.0, 0.0, 1.0); // Dummy fog logic
    
    return out;
}

fragment float4 fragment_main(VertexOut in [[stage_in]],
                             constant Uniforms &uniforms [[buffer(1)]],
                             texture2d<float> tex0 [[texture(0)]],
                             sampler sampler0 [[sampler(0)]]) {
    uint settings = uniforms.shaderSettings;
    
    float4 texColor = float4(1.0);
    if ((settings >> SHIFT_TEXTURING) & 0x1) {
        texColor = tex0.sample(sampler0, in.texCoord);
    }
    
    float4 finalColor = texColor * in.color;
    
    // Alpha Test
    if ((settings >> SHIFT_ALPHATEST) & 0x1) {
        if (finalColor.a < 0.375) { // 0x60/0xFF ~ 0.375
            discard_fragment();
        }
    }
    
    // Primary Gradient (Modulate/Add)
    uint priGrad = (settings >> SHIFT_PRIGRADIENT) & 0x7;
    if (priGrad == 1) { // GRADIENT_MODULATE
        // Already handled by texColor * in.color
    } else if (priGrad == 2) { // GRADIENT_ADD
        finalColor.rgb += in.color.rgb;
    }
    
    // Fog
    uint fogFunc = (settings >> SHIFT_FOG) & 0x3;
    if (fogFunc != 0) { // FOG_DISABLE
        float3 fogColor = float3(0.5, 0.5, 0.5); // Should come from uniforms
        finalColor.rgb = mix(finalColor.rgb, fogColor, in.fogFactor);
    }
    
    return finalColor;
}
