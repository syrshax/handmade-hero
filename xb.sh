#!/bin/bash

SOURCE_FILE="./src/sld_handmade.cpp"
OUTPUT_DIR="./build"
EXECUTABLE_NAME="handmadehero"

mkdir -p $OUTPUT_DIR

clang++ "$SOURCE_FILE" -o "$OUTPUT_DIR/$EXECUTABLE_NAME" -g -Wall -Wextra $(sdl2-config --cflags --libs)
