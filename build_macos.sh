#!/bin/bash
set -e

# Configuration
EXECUTABLE="build/macos/GeneralsExecutable"
mkdir -p build/macos

# Clean up old processes
pkill -9 GeneralsExecutable 2>/dev/null || true

# Source files
source Platform/MacOS/Scripts/files.sh

# Include paths
INCLUDES="-ICore/GameEngine/Include \
-IPlatform/MacOS/Include \
-ICore/Libraries/Include \
-ICore/GameEngine/Include \
-IGeneralsMD/Code/GameEngine/Include \
-IGeneralsMD/Code/GameEngineDevice/Include \
-IPartiallyExtract/Code/GameEngine/Include \
-IPartiallyExtract/Code/Libraries/Include \
-IGenerals/Code/GameEngine/Include \
-IDependencies/gamespy/include \
-IDependencies/gamespy/include/gamespy \
-IDependencies/Utility \
-IDependencies/Utility/osdep \
-IDependencies/Utility/Utility \
-ICore/Libraries/Source/WWVegas/WW3D2 \
-ICore/Libraries/Include \
-ICore/Libraries/Source/WWVegas \
-ICore/Libraries/Source/WWVegas/WWSaveLoad \
-ICore/Libraries/Source/WWVegas/WWDebug \
-ICore/Libraries/Source/WWVegas/WWLib \
-IGeneralsMD/Code/Libraries/Source/WWVegas/WWLib \
-ICore/Libraries/Include \
-IGeneralsMD/Code/Libraries/Source \
-IDependencies/Utility \
-IDependencies/Utility/Utility \
-ICore/Libraries/Source/WWVegas \
-IGeneralsMD/Code/Libraries/Source/WWVegas/WW3D2 \
-IPlatform/MacOS/Source/Main \
-IPlatform/MacOS/Source/Metal \
-IGeneralsMD/Code/Libraries/Source/WWVegas \
-ICore/Libraries/Source/WWVegas/WW3D2 \
-IGeneralsMD/Code/Libraries/Source/WWVegas/WW3D2 \
-IGeneralsMD/Code/GameEngine/Include/Precompiled \
-ICore/Libraries/Source/WWVegas/WWMath \
-ICore/Libraries/Source/WWVegas/WWSaveLoad \
-ICore/Libraries/Source/WWVegas/WWDebug \
-ICore/Libraries/Source/WWVegas/WWAudio \
-IGeneralsMD/Code/GameEngineDevice/Include \
-ICore/GameEngineDevice/Include \
-IDependencies/miles \
-IDependencies/miles/mss \
-ICore/Libraries/Source/Compression \
-Igitinfo \
-ICore/Libraries/Source \
-IGeneralsMD/Code/GameEngine/Include/Precompiled \
-ICore/Libraries/Source/WWVegas/WWMath \
-IGeneralsMD/Code/GameEngineDevice/Include \
-IGeneralsMD/Code/GameEngineDevice/Source/W3DDevice/GameClient/GUI/GUICallbacks \
-ICore/Libraries/Source/WWVegas \
-IGeneralsMD/Code/Libraries/Source/WWVegas \
-IPlatform/MacOS/Source/Main \
-IGeneralsMD/Code/Main"

# Compilation flags
# Using -std=c++20 and -fobjc-arc for Objective-C++ files
COMMON_FLAGS="-std=c++20 -O0 -g -D_DEBUG -DRA_MACOS -DRTS_ZEROHOUR -D_LIBCPP_HAS_NO_ASAN -DRTS_DEBUG -DALLOW_DEBUG_UTILS -DDEBUG_CRASHING -Wno-deprecated-declarations -Wno-missing-braces -Wno-c++20-designator -Wno-invalid-offsetof -Wno-pessimizing-move"
OBJC_FLAGS="-fobjc-arc -fobjc-exceptions"

# 1. Compile source files
OBJECTS=()
MAX_JOBS=$(sysctl -n hw.ncpu)
JOB_COUNT=0

for src in "${SOURCES[@]}"; do
  echo "DEBUG: Processing $src"
  if [ ! -f "$src" ]; then
    echo "⚠️ Warning: File not found: $src"
    continue
  fi

  obj_name=$(echo "$src" | sed 's/\//_/g' | sed 's/\./_/g')
  obj="build/macos/${obj_name}.o"
  OBJECTS+=("$obj")

  if [ -f "$obj" ] && [ "$obj" -nt "$src" ]; then
    continue
  fi

  echo "🔨 Compiling $src..."
  echo "DEBUG: obj is [$obj]"
  if [[ "$src" == *.mm ]]; then
    clang++ -x objective-c++ -c "$src" -o "$obj" $INCLUDES $COMMON_FLAGS $OBJC_FLAGS
  elif [[ "$src" == *.c ]]; then
    clang -c "$src" -o "$obj" $INCLUDES ${COMMON_FLAGS//-std=c++20/-std=c11}
  else
    clang++ -c "$src" -o "$obj" -Wno-switch $INCLUDES $COMMON_FLAGS
  fi

  if [ $? -ne 0 ]; then
    echo "❌ Compilation failed for $src"
    exit 1
  fi
done

wait
echo "✅ Compilation finished."

# 3. Link executable
echo "� Linking executable..."
echo "Linking with ${#OBJECTS[@]} objects..."

# Use a response file to avoid command line length limits
printf "%s\n" "${OBJECTS[@]}" > build/macos/objects.txt

clang++ @build/macos/objects.txt -o "$EXECUTABLE" -lz -framework Foundation -framework Cocoa -framework Metal -framework QuartzCore -framework AVFoundation

echo "✅ Build successful!"
echo "📍 Location: $EXECUTABLE"

# 4. Run (Optional, but useful for testing)
if [[ "$1" == "--run" ]]; then
    echo "🚀 Running Generals... (Logging to Platform/MacOS/Build/Logs/run.log)"
    mkdir -p Platform/MacOS/Build/Logs
    ./$EXECUTABLE 2>&1 | tee Platform/MacOS/Build/Logs/run.log
fi
