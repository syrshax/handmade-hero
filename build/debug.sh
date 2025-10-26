#!/bin/bash

echo "Compiling..."

clang++ ../src/sdl_handmade.cpp -o handmadehero -g `pkg-config --cflags --libs sdl3`

if [ $? -eq 0 ]; then
    echo "Compilation successful"
else
    echo "Compilation FAILED. The executable was NOT run."
fi

echo "Opening Debugger for Executable at $TARGET_LOCATION..."

../../gdb-frontend/gdbfrontend 
