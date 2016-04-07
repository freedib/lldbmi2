#!/bin/bash
mkdir build
cd build
/usr/local/bin/cmake .. $1 $2
make $3
