#!/bin/bash
export DYLD_LIBRARY_PATH=$DYLD_LIBRARY_PATH:.
cd GameResources
../build/macos/GeneralsExecutable "$@"
