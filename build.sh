#!/bin/bash

# Create bin directory if it doesn't exist
if [ ! -d "bin" ]; then
    mkdir bin
    echo "Created bin directory"
else
    rm -f bin/*
    echo "Cleaned bin directory"
fi

# Create output_files directory if it doesn't exist
if [ ! -d "output_files" ]; then
    mkdir output_files
    echo "Created output_files directory"
fi

# Compile the program
echo "Compiling interrupts.cpp..."
g++ -g -O0 -I . -o bin/interrupts interrupts.cpp

# Check if compilation was successful
if [ $? -eq 0 ]; then
    echo "Compilation successful!"
    echo ""
    echo "To run the program, use:"
    echo "./bin/interrupts input_files/trace.txt vector_table.txt device_table.txt external_files.txt"
else
    echo "Compilation failed!"
    exit 1
fi
