if [ ! -d "bin" ]; then
    mkdir bin
    echo "Created bin directory"
else
    rm -f bin/*
fi

echo "Compiling interrupts.cpp..."
g++ -g -O0 -I . -o bin/interrupts Interrupts_TimurGrigoryev_RounakMukherjee.cpp

if [ $? -eq 0 ]; then
    echo "Compilation successful!"
else
    echo "Compilation failed!"
    exit 1
fi
