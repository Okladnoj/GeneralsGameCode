#!/bin/bash
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:.
./build/macos/GeneralsExecutable "$@"
