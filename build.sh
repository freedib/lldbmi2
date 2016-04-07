#!/bin/bash
mkdir build
cd build
/usr/local/bin/cmake ..
make $1
