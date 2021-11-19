#! /bin/sh

# compile.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2021  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

set -e

# just an example
export PKG_CONFIG_PATH="$HOME/crossbuild/local/linux-arm32v7/lib/pkgconfig"
preset="$1"
shift
case "$preset" in
	clang)
		nice scons platform=frt use_llvm=yes use_lto=no \
		  tools=no target=release \
		  frt_arch=arm32v7 frt_cross=auto $*
		;;
	gcc)
		nice scons platform=frt use_llvm=no use_lto=no \
		  tools=no target=debug \
		  frt_arch=arm32v7 frt_cross=auto $*
		;;
	clang-lto)
		nice scons platform=frt use_llvm=yes use_lto=yes \
		  tools=no target=release \
		  frt_arch=arm32v7 frt_cross=auto $*
		;;
	gcc-lto)
		nice scons platform=frt use_llvm=no use_lto=yes \
		  tools=no target=debug \
		  frt_arch=arm32v7 frt_cross=auto $*
		;;
	native)
		nice scons platform=frt use_llvm=yes use_lto=no \
		  tools=no target=release \
		  $*
		;;
	*)
		echo "compile.sh: unknown preset."
		;;
esac
