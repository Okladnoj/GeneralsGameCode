set(UTILITY_SRC
    Utility/compat.h
    Utility/CppMacros.h
    Utility/endian_compat.h
    Utility/fstream_adapter.h
    Utility/hash_map_adapter.h
    Utility/intrin_compat.h
    Utility/iostream_adapter.h
    Utility/malloc_adapter.h
    Utility/mem_compat.h
    Utility/sstream_adapter.h
    Utility/stdint_adapter.h
    Utility/stdio_adapter.h
    Utility/string_compat.h
    Utility/tchar_compat.h
    Utility/thread_compat.h
    Utility/time_compat.h
    Utility/win_compat.h
    Utility/imagehlp_compat.h
    Utility/objbase.h
    Utility/windows.h
    Utility/atlbase.h
    Utility/direct.h
    Utility/winsock.h
    Utility/process.h
    Utility/mmsystem.h
    Utility/io.h
    Utility/wchar_compat.h
    Utility/dsound.h
    Utility/d3d8.h
    Utility/dinput.h
)

## core_utility

add_library(core_utility INTERFACE)
target_include_directories(core_utility INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Utility
)
target_precompile_headers(core_utility INTERFACE
    [["Utility/CppMacros.h"]] # Must be first, to be removed when abandoning VC6
)
target_sources(core_utility PRIVATE ${UTILITY_SRC})

## core_utility_no_pch

add_library(core_utility_no_pch INTERFACE)
target_include_directories(core_utility_no_pch INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/Utility
)
target_sources(core_utility_no_pch PRIVATE ${UTILITY_SRC})

if(NOT WIN32)
    # Define Windows-specific system libraries as empty interface libraries on macOS/Linux.
    # The functions are either provided by system headers or stubbed in win_compat.h.
    add_library(comctl32 INTERFACE)
    add_library(imm32 INTERFACE)
    add_library(vfw32 INTERFACE)
    add_library(winmm INTERFACE)
    add_library(ws2_32 INTERFACE)
    add_library(ole32 INTERFACE)
    add_library(oleaut32 INTERFACE)
    add_library(advapi32 INTERFACE)
    add_library(user32 INTERFACE)
    add_library(gdi32 INTERFACE)
    add_library(shell32 INTERFACE)
    add_library(uuid INTERFACE)
    add_library(version INTERFACE)
    add_library(d3d8 INTERFACE)
    add_library(dinput8 INTERFACE)
    add_library(dsound INTERFACE)
    add_library(dxguid INTERFACE)
    add_library(d3dx8 INTERFACE)
    add_library(d3dx8d INTERFACE)
endif()
