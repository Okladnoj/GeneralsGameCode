# Platform/MacOS — Metal backend, Cocoa integration, platform stubs
# This library provides the macOS platform layer that replaces Win32Device components.

# ── GameNetworkingSockets (Valve P2P transport) ──
include(FetchContent)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5 CACHE STRING "" FORCE)

set(BUILD_SHARED_LIB OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC_LIB ON  CACHE BOOL "" FORCE)
set(BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(BUILD_EXAMPLES   OFF CACHE BOOL "" FORCE)
set(ENABLE_ICE       ON  CACHE BOOL "" FORCE)
set(USE_STEAMWEBRTC  ON  CACHE BOOL "" FORCE)
add_definitions(-DSTEAMWEBRTC_USE_STATIC_LIBS -DSTEAMNETWORKINGSOCKETS_ENABLE_WEBRTC)
set(USE_CRYPTO       "OpenSSL" CACHE STRING "" FORCE)
set(USE_CRYPTO25519  "OpenSSL" CACHE STRING "" FORCE)
set(Protobuf_USE_STATIC_LIBS OFF CACHE BOOL "" FORCE)

list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/openssl@3")
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/protobuf")
list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew/opt/abseil")

set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl@3" CACHE PATH "" FORCE)
set(Protobuf_ROOT "/opt/homebrew/opt/protobuf" CACHE PATH "" FORCE)
set(Protobuf_INCLUDE_DIR "/opt/homebrew/opt/protobuf/include" CACHE PATH "" FORCE)
set(Protobuf_PROTOC_EXECUTABLE "/opt/homebrew/opt/protobuf/bin/protoc" CACHE PATH "" FORCE)

find_package(OpenSSL REQUIRED)
find_package(absl REQUIRED HINTS /opt/homebrew/opt/abseil/lib/cmake/absl)

include_directories(SYSTEM /opt/homebrew/opt/abseil/include)

FetchContent_Declare(
    GameNetworkingSockets
    GIT_REPOSITORY https://github.com/ValveSoftware/GameNetworkingSockets.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
    PATCH_COMMAND  sed -i "" "s/set(absl_FOUND OFF)/#set(absl_FOUND OFF) # patched: use system abseil/" src/external/steamwebrtc/CMakeLists.txt
)
FetchContent_MakeAvailable(GameNetworkingSockets)

FetchContent_GetProperties(GameNetworkingSockets SOURCE_DIR GNS_SOURCE_DIR)
set(GNS_INCLUDE_DIR "${GNS_SOURCE_DIR}/include" CACHE PATH "" FORCE)

target_include_directories(GameNetworkingSockets_s SYSTEM PUBLIC
    /opt/homebrew/opt/abseil/include
)

target_sources(GameNetworkingSockets_s PRIVATE
    ${GNS_SOURCE_DIR}/src/steamnetworkingsockets/clientlib/steamnetworkingsockets_p2p_webrtc.cpp
)
target_compile_definitions(GameNetworkingSockets_s PRIVATE
    STEAMNETWORKINGSOCKETS_ENABLE_WEBRTC
)

# ── Metal rendering backend (DX8 → Metal) ──
set(METAL_SRC
    Source/Metal/MetalDevice8.mm
    Source/Metal/MetalInterface8.mm
    Source/Metal/MetalSurface8.mm
    Source/Metal/MetalTexture8.mm
    Source/Metal/MetalVertexBuffer8.mm
    Source/Metal/MetalIndexBuffer8.mm
)

# ── Main application (game client, window, input, shaders) ──
set(MAIN_SRC
    Source/Main/MacOSGameClient.mm
    Source/Main/MacOSWindowManager.mm
    Source/Main/MacOSGameWindowManager.mm
    Source/Main/MacOSGadgetDraw.mm
    Source/Main/MacOSDisplayManager.mm
    Source/Main/D3DXStubs.mm
    Source/Main/StdKeyboard.mm
    Source/Main/StdMouse.mm
    Source/Main/MacOSOpenURL.mm
    Source/Main/MacOSOnlineLogin.mm
    Source/Main/MacOSOnlineWebSocket.mm
    Source/Main/MacOSOnlineWSBridge.cpp
    Source/Main/MacOSOnlineLobby.mm
    Source/Main/MacOSOnlineP2P.mm
    Source/Main/MacOSWebRTC.mm
    Source/Main/MacOSOnlineStats.mm
    Source/Main/MacOSOnlineSocial.mm
    Source/Main/MacOSOnlineQM.mm
    Source/Main/MacOSMapDownloader.mm
)

# ── Client (display, text rendering) ──
set(CLIENT_SRC
    Source/Client/MacOSDisplay.mm
    Source/Client/MacOSDisplayString.mm
)

# ── Audio ──
set(AUDIO_SRC
    Source/Audio/MacOSAudioManager.mm
)

# ── File system (cross-platform .big + local) ──
set(COMMON_SRC
    Source/Common/StdBIGFile.cpp
    Source/Common/StdBIGFileSystem.cpp
    Source/Common/StdLocalFile.cpp
    Source/Common/StdLocalFileSystem.cpp
)

# ── Stubs (GameSpy, WWDownload — Windows-only libs) ──
# NOTE: LZHLStubs.cpp was REMOVED — real liblzhl is linked via core_compression.
# NOTE: MacOSW3DShaderManager.mm was REMOVED — Core/W3DShaderManager.cpp provides
#       all symbols (setShroudTex, init, setShader, RTT, filters). The stub file
#       was overriding real implementations with no-ops, blocking shroud/fog-of-war,
#       render-to-texture, and screen filter effects.
# NOTE: GitInfoStubs.cpp was REMOVED — real resources target provides gitinfo.
# NOTE: GameSpySDKStubs.c was REMOVED — real libgamespy.a provides all SDK symbols.
set(STUBS_SRC
    Source/Stubs/GameSpyStubs.cpp
    Source/Stubs/WWDownloadStubs.cpp
)

# ── Debug ──
set(DEBUG_SRC
    Source/Debug/MacOSScreenshot.mm
)

# ── Combine all sources ──
add_library(macos_platform STATIC
    ${METAL_SRC}
    ${MAIN_SRC}
    ${CLIENT_SRC}
    ${AUDIO_SRC}
    ${COMMON_SRC}
    ${STUBS_SRC}
    ${DEBUG_SRC}
)

# ── Include directories ──
# Platform/MacOS/Include is already in d3d8lib INTERFACE (set in root CMakeLists.txt),
# but we also need it for our own sources.
target_include_directories(macos_platform PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Include
)

# Private includes for implementation files
target_include_directories(macos_platform PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Metal
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Main
    ${GNS_INCLUDE_DIR}
    # WWVegas include paths — needed because .mm files include game engine headers
    # whose chain reaches always.h, WWCommon.h, wwdebug.h etc.
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWLib
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas
    # WW3D2 headers (dx8indexbuffer.h, dx8vertexbuffer.h, dx8wrapper.h)
    ${CMAKE_SOURCE_DIR}/Generals/Code/Libraries/Source/WWVegas/WW3D2
    # Precompiled header path (PreRTS.h) — use GeneralsMD for Zero Hour
    ${CMAKE_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include/Precompiled
    # W3DDevice headers (W3DShaderManager.h, W3DAssetManager.h, W3DInGameUI.h, etc.)
    # Use GeneralsMD (Zero Hour) versions to match zi_always/zi_gameengine_include
    ${CMAKE_SOURCE_DIR}/GeneralsMD/Code/GameEngine/Include
    ${CMAKE_SOURCE_DIR}/GeneralsMD/Code/GameEngineDevice/Include
    ${CMAKE_SOURCE_DIR}/Generals/Code/Libraries/Source/WWVegas
    # Core WWVegas subdirectories (bare includes from WW3D2 chain)
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWDebug
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWMath
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWSaveLoad
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WW3D2
    ${CMAKE_SOURCE_DIR}/Core/Libraries/Source/WWVegas/WWAudio
    # Core GameEngine/GameEngineDevice headers (W3DShaderManager.h, etc.)
    ${CMAKE_SOURCE_DIR}/Core/GameEngineDevice/Include
    ${CMAKE_SOURCE_DIR}/Core/GameEngine/Include
)

# ── Force-include Carbon compatibility header ──
# This header blocks conflicting Carbon sub-headers (AIFF, Files, IntlResources,
# ToolUtils) and pre-imports Apple frameworks before any game headers.
# Resolves all Carbon↔Game type conflicts (WideChar, ChunkHeader, FileInfo, RGBColor).
target_compile_options(macos_platform PRIVATE
    -include ${CMAKE_CURRENT_SOURCE_DIR}/Include/macos_carbon_compat.h
)
# ── macOS frameworks ──
find_library(METAL_FRAMEWORK Metal REQUIRED)
find_library(METALKIT_FRAMEWORK MetalKit REQUIRED)
find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
find_library(APPKIT_FRAMEWORK AppKit REQUIRED)
find_library(IOKIT_FRAMEWORK IOKit REQUIRED)
find_library(AUDIOTOOLBOX_FRAMEWORK AudioToolbox REQUIRED)
find_library(AVFOUNDATION_FRAMEWORK AVFoundation REQUIRED)

target_link_libraries(macos_platform PUBLIC
    ${METAL_FRAMEWORK}
    ${METALKIT_FRAMEWORK}
    ${QUARTZCORE_FRAMEWORK}
    ${COCOA_FRAMEWORK}
    ${APPKIT_FRAMEWORK}
    ${IOKIT_FRAMEWORK}
    ${AUDIOTOOLBOX_FRAMEWORK}
    ${AVFOUNDATION_FRAMEWORK}
)

# ── Project dependencies ──
target_link_libraries(macos_platform PUBLIC
    d3d8lib
    core_config
    corei_always
    corei_gameengine_include
    zi_libraries_source_wwvegas
    binkstub
    milesstub
    resources
    gamespy
    GameNetworkingSockets_s
)
target_compile_definitions(macos_platform PRIVATE STEAMNETWORKINGSOCKETS_STATIC_LINK)
target_link_directories(macos_platform PUBLIC /opt/homebrew/opt/abseil/lib /opt/homebrew/opt/protobuf/lib)
target_link_libraries(macos_platform PUBLIC
    protobuf
    absl_log_internal_check_op
    absl_log_internal_conditions
    absl_log_internal_message
    absl_log_internal_nullguard
    absl_log_internal_log_sink_set
    absl_log_internal_globals
    absl_log_internal_format
    absl_log_internal_proto
    absl_log_sink
    absl_log_globals
    absl_examine_stack
    absl_raw_logging_internal
    absl_spinlock_wait
    absl_strerror
    absl_str_format_internal
    absl_strings
    absl_strings_internal
    absl_int128
    absl_throw_delegate
    absl_base
    absl_malloc_internal
    absl_time
    absl_time_zone
    absl_synchronization
    absl_kernel_timeout_internal
    absl_graphcycles_internal
    absl_stacktrace
    absl_symbolize
    absl_debugging_internal
    absl_demangle_internal
    absl_demangle_rust
    absl_decode_rust_punycode
    absl_utf8_for_code_point
    OpenSSL::Crypto
)
# Zero Hour includes and defines — PRIVATE to avoid leaking RTS_ZEROHOUR
# to the Generals target that also links macos_platform
target_link_libraries(macos_platform PRIVATE
    zi_always
    zi_gameengine_include
)

# ── Compile Metal shaders ──
# Ensure Metal Toolchain is available (auto-download if missing)
execute_process(
    COMMAND xcrun -sdk macosx metal --version
    RESULT_VARIABLE METAL_CHECK_RESULT
    OUTPUT_QUIET ERROR_QUIET
)
if(NOT METAL_CHECK_RESULT EQUAL 0)
    message(STATUS "Metal Toolchain not found — downloading automatically...")
    execute_process(
        COMMAND xcodebuild -downloadComponent MetalToolchain
        RESULT_VARIABLE METAL_DL_RESULT
    )
    if(NOT METAL_DL_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to download Metal Toolchain. Please run manually:\n  xcodebuild -downloadComponent MetalToolchain")
    endif()
    message(STATUS "Metal Toolchain installed successfully")
endif()

set(METAL_SHADER_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/Source/Main/MacOSShaders.metal)
set(METAL_AIR_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MacOSShaders.air)
set(METAL_LIB_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/default.metallib)

add_custom_command(
    OUTPUT ${METAL_AIR_OUTPUT}
    COMMAND xcrun -sdk macosx metal -c ${METAL_SHADER_SOURCE} -o ${METAL_AIR_OUTPUT}
    DEPENDS ${METAL_SHADER_SOURCE}
    COMMENT "Compiling Metal shaders"
)

add_custom_command(
    OUTPUT ${METAL_LIB_OUTPUT}
    COMMAND xcrun -sdk macosx metallib ${METAL_AIR_OUTPUT} -o ${METAL_LIB_OUTPUT}
    DEPENDS ${METAL_AIR_OUTPUT}
    COMMENT "Linking Metal shader library"
)

add_custom_target(metal_shaders ALL DEPENDS ${METAL_LIB_OUTPUT})
add_dependencies(macos_platform metal_shaders)

# ── ObjC++ settings ──
# Enable ARC for all ObjC++ files in this target.
target_compile_options(macos_platform PRIVATE
    $<$<COMPILE_LANGUAGE:OBJCXX>:-fobjc-arc>
)

# The inherited PCH (from core_utility INTERFACE) was compiled without ARC,
# which conflicts with our -fobjc-arc flag for OBJCXX files.
# By specifying our own PCH with the same content, CMake generates a new PCH
# binary for this target. Since this target has -fobjc-arc, the OBJCXX PCH
# variant will include the ARC flag, avoiding the mismatch.
target_precompile_headers(macos_platform PRIVATE
    [["Utility/CppMacros.h"]]
)

# ── Integration Tests ──
add_subdirectory(Tests)

# ── Install mod resources (.big) into ZH game directory ──
set(MACOS_RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Resources")
if(DEFINED ENV{GENERALS_INSTALL_PATH})
    set(ZH_GAME_DIR "$ENV{GENERALS_INSTALL_PATH}/Command and Conquer Generals Zero Hour")
    file(GLOB MACOS_BIG_FILES "${MACOS_RESOURCES_DIR}/*.big")
    foreach(BIG_FILE ${MACOS_BIG_FILES})
        get_filename_component(BIG_NAME "${BIG_FILE}" NAME)
        set(BIG_DEST "${ZH_GAME_DIR}/${BIG_NAME}")
        if(NOT EXISTS "${BIG_DEST}")
            message(STATUS "Linking mod: ${BIG_NAME}")
            file(CREATE_LINK "${BIG_FILE}" "${BIG_DEST}" SYMBOLIC)
        endif()
    endforeach()
endif()
