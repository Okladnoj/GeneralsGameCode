# g_googletest and z_googletest compile these sources together with the Core tests.
set(DEPSI_GOOGLETEST_SRC
    GameMath/gmath_test.cpp
    Utility/stringex_test.cpp
)

add_library(depsi_googletest INTERFACE)

target_sources(depsi_googletest INTERFACE ${DEPSI_GOOGLETEST_SRC})

target_compile_definitions(depsi_googletest INTERFACE
    GAMEMATH_SNAPSHOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}/GameMath/Snapshots"
)

if(NOT MSVC)
    # The GameMath snapshots are taken without fused multiply-add, as GameMath itself is built.
    target_compile_options(depsi_googletest INTERFACE -ffp-contract=off)
endif()

target_link_libraries(depsi_googletest INTERFACE
    deps_utility
    gamemath
)
