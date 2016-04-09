#!/bin/sh
mkdir build
mkdir logs
cd build
# separates cmake options from make tag which is last
# OPT = -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=1 -DUSE_LIB_PATH=/lldb_tree/debug/lib  ...
# TAG = all, clean
OPT=
TAG=
for F; do OPT="$OPT $TAG"; TAG=$F; done
# create cmake tree
/usr/local/bin/cmake .. $OPT
# build
make $TAG