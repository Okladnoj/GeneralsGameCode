set(GM_ENABLE_TESTS OFF CACHE BOOL "Disable GameMath tests" FORCE)

FetchContent_Declare(
    gamemath
    GIT_REPOSITORY https://github.com/OmniBlade/gamemath.git
    GIT_TAG        ce1dc3a2b18e119272aea99c670a3a76d5c78b08
)

FetchContent_MakeAvailable(gamemath)

# Ensure GameMath includes are available to ALL targets
# to prevent one-definition-rule violations and ensure USE_DETERMINISTIC_MATH activates consistently.
include_directories(${gamemath_SOURCE_DIR}/include)
