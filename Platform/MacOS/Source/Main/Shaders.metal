#include <metal_stdlib>
using namespace metal;

struct VertexIn {
    float3 position [[attribute(0)]];
    uchar4 color    [[attribute(1)]];
    float2 texCoord [[attribute(2)]];
};

struct RasterizerData {
    float4 position [[position]];
    float2 texCoord;
    float4 color;
};

struct Uniforms {
    float4x4 world;
    float4x4 view;
    float4x4 projection;
    float2 screenSize;
    int useProjection; // 1 to use 3D (WVP), 0 to use screen coordinates, 2 for pass-through
    int padding;
};

vertex RasterizerData vertexShader(VertexIn in [[stage_in]],
                                   constant Uniforms &uniforms [[buffer(1)]]) {
    RasterizerData out;
    
    float4 pos = float4(in.position, 1.0);
    
    if (uniforms.useProjection == 1) {
        // W3D/DirectX matrices are usually row-major. 
        // In Metal (column-major), M*V with Row-Major data in memory
        // is equivalent to V*M in Row-Major world.
        // We might need to transpose if it's not correct.
        out.position = uniforms.projection * uniforms.view * uniforms.world * pos;
    } else if (uniforms.useProjection == 0) {
        // Screen coordinates: [0, screenWidth] -> [-1, 1], [0, screenHeight] -> [1, -1]
        out.position = float4(
            (pos.x / uniforms.screenSize.x) * 2.0 - 1.0,
            1.0 - (pos.y / uniforms.screenSize.y) * 2.0,
            pos.z,
            1.0
        );
    } else {
        // Pass-through (already in NDC)
        out.position = pos;
    }
    
    out.texCoord = in.texCoord;
    out.color = float4(in.color) / 255.0;
    
    return out;
}

fragment float4 fragmentShader(RasterizerData in [[stage_in]],
                               texture2d<float> colorTexture [[texture(0)]],
                               sampler textureSampler [[sampler(0)]]) {
    float4 texColor = colorTexture.sample(textureSampler, in.texCoord);
    return texColor * in.color;
}
