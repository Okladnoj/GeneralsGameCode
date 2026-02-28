#!/bin/bash

# Run:
#   sh build_run_mac.sh                       # build + run
#   sh build_run_mac.sh --clean               # clean + build + run
#   sh build_run_mac.sh --screenshot          # build + run + screenshot after 12s
#   sh build_run_mac.sh --screenshot=8.5      # build + run + screenshot after 8.5s
#   sh build_run_mac.sh --test                # build + run tests
#   sh build_run_mac.sh --zombie              # run with NSZombieEnabled (ObjC use-after-free)
#   sh build_run_mac.sh --debug               # run with MallocScribble+GuardEdges (heap corruption)

DO_CLEAN=false
DO_SCREENSHOT=false
DO_TEST=false
DO_ZOMBIE=false
DO_DEBUG_MALLOC=false
TEST_FILTER=""
SCREENSHOT_DELAY=""

for arg in "$@"; do
    case "$arg" in
        --clean)
            DO_CLEAN=true
            ;;
        --test)
            DO_TEST=true
            ;;
        --test=*)
            DO_TEST=true
            TEST_FILTER="${arg#--test=}"
            ;;
        --screenshot=*)
            DO_SCREENSHOT=true
            SCREENSHOT_DELAY="${arg#--screenshot=}"
            ;;
        --screenshot)
            DO_SCREENSHOT=true
            ;;
        --zombie)
            DO_ZOMBIE=true
            ;;
        --debug)
            DO_DEBUG_MALLOC=true
            ;;
    esac
done

if [ "$DO_CLEAN" = true ]; then
    echo "Cleaning build directory..."
    rm -rf build
fi

if [ ! -d "build/macos" ]; then
    echo "Configuring CMake preset..."
    cmake --preset macos
    if [ $? -ne 0 ]; then
        exit 1
    fi
fi

echo "Building project..."
cmake --build build/macos
if [ $? -ne 0 ]; then
    exit 1
fi

# ── Test Mode ──
if [ "$DO_TEST" = true ]; then
    mkdir -p Platform/MacOS/Build/Logs
    TEST_LOG="Platform/MacOS/Build/Logs/test_results.log"
    echo ""
    echo "Running DX8→Metal Bridge Tests..."
    echo ""
    if [ -n "$TEST_FILTER" ]; then
        ./build/macos/Platform/MacOS/Tests/metal_bridge_tests "$TEST_FILTER" 2>&1 | tee "$TEST_LOG"
    else
        ./build/macos/Platform/MacOS/Tests/metal_bridge_tests 2>&1 | tee "$TEST_LOG"
    fi
    TEST_EXIT=${PIPESTATUS[0]}
    echo ""
    echo "Test log saved to: $TEST_LOG"
    exit $TEST_EXIT
fi

sleep 1

echo "Killing previous generalszh instance..."
killall generalszh 2>/dev/null

sleep 1

export GENERALS_INSTALL_PATH="/Users/okji/dev/games/Command and Conquer - Generals/Command and Conquer Generals/"

# Metal frame rate control:
# 60 = VSync (default)
# 0  = uncapped
# 30/120/240 = custom
export GENERALS_FPS_LIMIT="${GENERALS_FPS_LIMIT:-60}"

# Screenshot delay (default 12s)
if [ -z "$SCREENSHOT_DELAY" ]; then
    SCREENSHOT_DELAY=12
fi

# ── Debug Environment ──
GAME_ENV=""
if [ "$DO_ZOMBIE" = true ]; then
    GAME_ENV="NSZombieEnabled=YES OBJC_DEBUG_MISSING_POOLS=YES"
    echo "Debug mode: NSZombieEnabled (ObjC zombie detection)"
fi
if [ "$DO_DEBUG_MALLOC" = true ]; then
    GAME_ENV="$GAME_ENV MallocGuardEdges=1 MallocScribble=1"
    echo "Debug mode: MallocScribble + GuardEdges (heap corruption detection)"
fi

GAME_CMD="build/macos/GeneralsMD/generalszh"
if [ -n "$GAME_ENV" ]; then
    GAME_CMD="env $GAME_ENV $GAME_CMD"
fi

echo "Starting game..."
if [ "$DO_SCREENSHOT" = true ]; then
    $GAME_CMD > Platform/MacOS/Build/Logs/game.log 2>&1 &
    GAME_PID=$!
    echo "Waiting ${SCREENSHOT_DELAY}s for shell map to load..."
    sleep ${SCREENSHOT_DELAY}
    python3 Platform/MacOS/Build/screenshot.py
    echo "Killing game (pid=$GAME_PID)..."
    kill $GAME_PID 2>/dev/null
    wait $GAME_PID 2>/dev/null
else
    $GAME_CMD > Platform/MacOS/Build/Logs/game.log 2>&1
fi
