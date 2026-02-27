#!/bin/bash

# Run:
#   sh build_run_mac.sh                       # build + run
#   sh build_run_mac.sh --clean               # clean + build + run
#   sh build_run_mac.sh --screenshot          # build + run + screenshot after 15s
#   sh build_run_mac.sh --screenshot=8.5      # build + run + screenshot after 8.5s
# Run: sh build_run_mac.sh --test                # build + run tests

DO_CLEAN=false
DO_SCREENSHOT=false
DO_TEST=false
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

# Screenshot delay (default 15s)
if [ -z "$SCREENSHOT_DELAY" ]; then
    SCREENSHOT_DELAY=15
fi

echo "Starting game..."
if [ "$DO_SCREENSHOT" = true ]; then
    build/macos/GeneralsMD/generalszh > Platform/MacOS/Build/Logs/game.log 2>&1 &
    GAME_PID=$!
    echo "Waiting ${SCREENSHOT_DELAY}s for shell map to load..."
    sleep ${SCREENSHOT_DELAY}
    python3 Platform/MacOS/Build/screenshot.py
    echo "Killing game (pid=$GAME_PID)..."
    kill $GAME_PID 2>/dev/null
    wait $GAME_PID 2>/dev/null
else
    build/macos/GeneralsMD/generalszh > Platform/MacOS/Build/Logs/game.log 2>&1
fi
