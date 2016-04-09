#!/bin/sh
mkdir build
mkdir logs
cd build
/usr/local/bin/cmake .. $1 $2
make $3