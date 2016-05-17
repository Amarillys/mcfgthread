#!/bin/sh

set -e

prefix="$(pwd)/release/mingw64"
mkdir -p "$prefix"

builddir="$(pwd)/build_x86_64"
mkdir -p "$builddir"

build=x86_64-w64-mingw32

echo Building shared library...
mkdir -p m4
autoreconf -i
(cd "$builddir" &&
  CFLAGS='-O3' CPPFLAGS='-DNDEBUG' LDFLAGS='-Wl,-s' ../configure --disable-static --enable-shared --build="$build" --host="$build" --prefix="$prefix" &&
  make -j4)

echo Testing...
("$build"-gcc -std=c11 -Wall -Wextra -Wpedantic -Werror test/test.c -Isrc/env -L"$builddir/lib/.libs" -lmcfgthread -o test.exe &&
  PATH=$PATH:"$builddir/lib/.libs" ./test.exe)

(cd "$builddir" &&
  make install)
