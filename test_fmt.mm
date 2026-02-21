#import <Metal/Metal.h>
#include <cstdio>
int main() {
    id<MTLDevice> device = MTLCreateSystemDefaultDevice();
    printf("B5G6R5: %d\n", [device supportsTextureSampleCount:1] ? 1 : 0); // just to check compile
    MTLTextureDescriptor *desc = [[MTLTextureDescriptor alloc] init];
    desc.pixelFormat = MTLPixelFormatB5G6R5Unorm;
    desc.width = 16;
    desc.height = 16;
    id<MTLTexture> tex1 = [device newTextureWithDescriptor:desc];
    printf("B5G6R5: %p\n", tex1);
    desc.pixelFormat = MTLPixelFormatBGR5A1Unorm;
    id<MTLTexture> tex2 = [device newTextureWithDescriptor:desc];
    printf("BGR5A1: %p\n", tex2);
    return 0;
}
