import Metal
import Foundation

guard let device = MTLCreateSystemDefaultDevice() else { exit(1) }
let desc = MTLTextureDescriptor()
desc.pixelFormat = .bgr5A1Unorm
desc.width = 256
desc.height = 256
desc.mipmapLevelCount = 3
desc.usage = [.shaderRead, .renderTarget]
if let _ = device.makeTexture(descriptor: desc) {
    print("SUCCESS")
} else {
    print("FAIL")
}
