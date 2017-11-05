# vega

[![Build Status](https://travis-ci.org/Alneos/vega.svg?branch=master)](https://travis-ci.org/Alneos/vega)

Finite element format converter

apt-get install cmake cmake-curses-gui gcc-4.8 valgrind

apt-get install libmedc1 libmedc-dev libboost-all-dev 

apt-get install ccache distcc distcc-pump graphviz

ECLIPSE:

install plugin: C/C++ Unit Testing support

included in standard repositories.

Window -> Preferences -> C/C++ -> Build -> Settings -> Discovery -> CDT GCC Build-in Compiler Settings

in the text box entitled Command to get compiler specs append -std=c++11

Generate eclipse project files

cmake -G"Eclipse CDT4 - Unix Makefiles" -D CMAKE_BUILD_TYPE=Debug ..

Then import generated directory to eclipse as standard eclipse project. Right click project and open

Properties -> C/C++ General -> Preprocessor Include Paths, Marcos etc. -> Providers

enable CDT GCC Build-in Compiler Settings and move it higher than Contributed PathEntry Containers (This is important)

Right Click over "[Targets]" and "[Subprojects]", mark both as "derived" in properties 

recompile, regenerate Project ->C/C++ Index and restart Eclipse.

LINUX:

cd /"wherever vegapp was downloaded"/vega

mkdir -p ./build/x86_64
cd ./build/x86_64

Build release (static linking):

cmake -DCMAKE_BUILD_TYPE=SRelease ../..

make -j 4

ctest .

Build debug (default, dynamic linking):

cmake -DCMAKE_BUILD_TYPE=DDebug  ../..

make -j 4

ctest .
