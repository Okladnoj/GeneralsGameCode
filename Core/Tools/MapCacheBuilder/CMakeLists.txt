set(MAPCACHEBUILDER_SRC
    "Include/WinMain.h"
    "Resource/Resource.h"
    "Source/WinMain.cpp"
)

add_library(corei_mapcachebuilder INTERFACE)

target_sources(corei_mapcachebuilder INTERFACE ${MAPCACHEBUILDER_SRC})

target_include_directories(corei_mapcachebuilder INTERFACE
    Include
    Resource
)
#
target_link_libraries(corei_mapcachebuilder INTERFACE
    $<$<PLATFORM_ID:Windows>:comctl32>
    core_debug
    core_profile
    $<$<PLATFORM_ID:Windows>:imm32>
    $<$<PLATFORM_ID:Windows>:vfw32>
    $<$<PLATFORM_ID:Windows>:winmm>
    $<$<PLATFORM_ID:Windows>:ole32>
)

if(WIN32 OR "${CMAKE_SYSTEM}" MATCHES "Windows")
    target_sources(corei_mapcachebuilder INTERFACE Resource/MapCacheBuilder.rc)
endif()
