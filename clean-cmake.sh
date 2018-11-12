#!/bin/bash

find . -name Makefile| xargs rm
rm CMakeCache.txt
find . -name CMakeFiles|xargs rm -Rf {}
find . -name cmake_install.cmake|xargs rm -Rf {}
find . -name CTestTestfile.cmake|xargs rm -Rf {}
find . -name Debug -type d |xargs rm -Rf {}
find . -name Release -type d |xargs rm -Rf {}
find . -name '*.dir' -type d |xargs rm -Rf {}
find . -name '*.vcxproj*' |xargs rm -Rf {}
find . -name '*.sln' |xargs rm -Rf {}

rm lib/*
rm -Rf bin/*
rm bin/vegapp
rm -Rf build/Abstract
rm -Rf build/Aster
rm -Rf build/lib
rm -Rf Testing


echo "Debug: cd ./build/x86_64;cmake -DCMAKE_BUILD_TYPE=Debug ../.."
echo "Release: cd ./build/x86_64;cmake -DCMAKE_BUILD_TYPE=Release ../.."
