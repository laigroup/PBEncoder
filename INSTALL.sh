#!/bin/bash

if [ ! -f "./Encoder" ] 
then
    echo "file not exisit"
else 
    rm Encoder 
fi

mkdir -p build && cd build && cmake .. && make -f Makefile && cp Encoder ..
