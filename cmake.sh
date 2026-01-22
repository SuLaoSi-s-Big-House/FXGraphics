#!/bin/bash

if [ ! -d build ]; then
    mkdir build
fi

cd ./build
#cmake -G "Visual Studio 17 2022" ..
#cmake -G "MinGW Makefiles" ..
cmake ..

read -p "Press Enter to continue:"
