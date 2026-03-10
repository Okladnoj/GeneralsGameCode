#!/bin/bash

# Run:
#   sh build_run_mac.sh                       # build + run
#   sh build_run_mac.sh --clean               # clean + build + run
#   sh build_run_mac.sh --screenshot          # build + run + screenshot after 12s
#   sh build_run_mac.sh --screenshot=8.5      # build + run + screenshot after 8.5s
#   sh build_run_mac.sh --test                # build + run tests

# ── Game Command-Line Flags ──
# Toggle on/off: true = pass to game, false = skip
GAME_FLAG_NOSHELLMAP=false
GAME_FLAG_QUICKSTART=false
GAME_FLAG_NOAUDIO=false
GAME_FLAG_WIN=false
GAME_FLAG_XRES=""       # e.g. "1024"
GAME_FLAG_YRES=""       # e.g. "768"

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

export GENERALS_INSTALL_PATH="/Users/okji/dev/games/Command and Conquer - Generals"

# Metal frame rate control:
# 60 = VSync (default)
# 0  = uncapped
# 30/120/240 = custom
export GENERALS_FPS_LIMIT="${GENERALS_FPS_LIMIT:-60}"

# Screenshot delay (default 12s)
if [ -z "$SCREENSHOT_DELAY" ]; then
    SCREENSHOT_DELAY=12
fi

# ── Build Game Args ──
GAME_ARGS=""
[ "$GAME_FLAG_NOSHELLMAP" = true ] && GAME_ARGS="$GAME_ARGS -noshellmap"
[ "$GAME_FLAG_QUICKSTART" = true ] && GAME_ARGS="$GAME_ARGS -quickstart"
[ "$GAME_FLAG_NOAUDIO" = true ]    && GAME_ARGS="$GAME_ARGS -noaudio"
[ "$GAME_FLAG_WIN" = true ]        && GAME_ARGS="$GAME_ARGS -win"
[ -n "$GAME_FLAG_XRES" ]           && GAME_ARGS="$GAME_ARGS -xRes $GAME_FLAG_XRES"
[ -n "$GAME_FLAG_YRES" ]           && GAME_ARGS="$GAME_ARGS -yRes $GAME_FLAG_YRES"

GAME_CMD="build/macos/GeneralsMD/generalszh"

if [ -n "$GAME_ARGS" ]; then
    echo "Game args:$GAME_ARGS"
fi

echo "Starting game..."
if [ "$DO_SCREENSHOT" = true ]; then
    $GAME_CMD $GAME_ARGS > Platform/MacOS/Build/Logs/game.log 2>&1 &
    GAME_PID=$!
    echo "Waiting ${SCREENSHOT_DELAY}s for game to load..."
    sleep ${SCREENSHOT_DELAY}
    python3 Platform/MacOS/Build/screenshot.py
    echo "Killing game (pid=$GAME_PID)..."
    kill $GAME_PID 2>/dev/null
    wait $GAME_PID 2>/dev/null
else
    $GAME_CMD $GAME_ARGS > Platform/MacOS/Build/Logs/game.log 2>&1
fi
