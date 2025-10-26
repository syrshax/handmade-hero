#!/bin/bash

echo "Compiling..."

clang++ ../src/sld_handmade.cpp -o handmadehero -g `pkg-config --cflags --libs sdl3`

if [ $? -eq 0 ]; then
    echo "Compilation successful. Executing..."
    ./handmadehero
else
    echo "Compilation FAILED. The executable was NOT run."
fi
