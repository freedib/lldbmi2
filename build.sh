#!/bin/sh
# format: build.sh or build.sh opt opt ... opt tag
if [ ! -d "build" ]; then
	mkdir build
fi
if [ ! -d "logs" ]; then
	mkdir logs
fi
cd build
OPT=
TAG=
# separates build.sh args from cmake options and tag which is last
# OPT = -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=1 -DUSE_LIB_PATH=~/sources/llvm-project/build/lib
# TAG = all, clean, lldbmi2, tests, inheritance
for ARG; do OPT="$OPT $TAG"; TAG=$ARG; done
# create cmake tree
echo "cmake" $OPT
cmake .. $OPT
# build
echo "make" $TAG
make $TAG
