#!/bin/bash
revisioncount=`git log --oneline | wc -l | tr -d ' '`
projectversion=`git describe --tags --long`
cleanversion=${projectversion%%-*}
echo "#define LLDBMI2_VERSION \"$cleanversion.$revisioncount\""
