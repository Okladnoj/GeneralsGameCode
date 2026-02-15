#!/bin/bash
export PATH="/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin"
# Set working directory to project root
cd "$(dirname "$0")"

# IMPORTANT: The game resources (Data, Window, Maps, etc.) are in GameResources/
# On Windows, they are in the same folder as the EXE.
# We change to GameResources/ so the game can find its assets.
cd GameResources

echo "Current directory: $(pwd)"
echo "Starting Generals (macOS)..."

# Create log file (in parent dir)
echo "--- Run Start $(date) ---" > ../run_debug.log

# Run executable and capture exit code
../build/macos/GeneralsExecutable "$@" 2>&1 | tee -a ../run_debug.log
REAL_EXIT_CODE=${PIPESTATUS[0]}

echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" >> ../run_debug.log
echo "Exit code: $REAL_EXIT_CODE" >> ../run_debug.log
echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "Exit code: $REAL_EXIT_CODE"
