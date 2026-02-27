# Platform/MacOS/Tests/CMakeLists.txt
# DX8 → Metal Bridge Integration Tests
#
# Builds: metal_bridge_tests executable
# Run:    sh build_run_mac.sh --test

add_executable(metal_bridge_tests
    test_main.cpp
)

# METAL_BRIDGE_TEST_MODE replaces Metal types with plain integers
# so tests compile without Metal framework
target_compile_definitions(metal_bridge_tests PRIVATE
    METAL_BRIDGE_TEST_MODE
)

# Include paths:
# - d3d8lib provides Platform/MacOS/Include (d3d8_stub.h, windows.h shim, etc.)
# - Metal bridge headers (MetalBridgeMappings.h, MetalFormatConvert.h)
target_include_directories(metal_bridge_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/Platform/MacOS/Source/Metal
)

# d3d8lib provides DX8 types and include paths
target_link_libraries(metal_bridge_tests PRIVATE
    d3d8lib
)

# C++17 for auto in ASSERT macros
target_compile_features(metal_bridge_tests PRIVATE cxx_std_17)

# Suppress warnings for test code
target_compile_options(metal_bridge_tests PRIVATE
    -Wno-unused-function
    -Wno-c99-extensions
)
