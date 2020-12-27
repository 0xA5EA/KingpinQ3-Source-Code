#!/bin/sh

#export CC=amd64-mingw32msvc-gcc
#export CCC=amd64-mingw32msvc-g++
#export SLD=amd64-mingw32msvc-ar
#export LD=amd64-mingw32msvc-g++
#export WINDRES=amd64-mingw32msvc-windres

export CC=x86_64-w64-mingw32-gcc
export CCC=x86_64-w64-mingw32-g++
export SLD=x86_64-w64-mingw32-ar
export LD=x86_64-w64-mingw32-ld
export WINDRES=x86_64-w64-mingw32-windres
export PKGCFG=x86_64-w64-mingw32-pkg-config

#export CC=/opt/mingw64/bin/x86_64-w64-mingw32-gcc
#export CCC=/opt/mingw64/bin/x86_64-w64-mingw32-g++
#export SLD=/opt/mingw64/bin/x86_64-w64-mingw32-ar
#export LD=/opt/mingw64/bin/x86_64-w64-mingw32-g++
#export WINDRES=/opt/mingw64/bin/x86_64-w64-mingw32-windres
export PLATFORM=mingw32
export ARCH=x64
exec make $*
