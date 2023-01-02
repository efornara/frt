#! /bin/sh
set -e

# compile.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2023  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

# just an example

usage() {
	cat <<EOT

usage: compile.sh <arch> <preset> [extra_scons_options]

where:
  arch: arm32v7 arm64v8 native
  preset: clang gcc clang-lto gcc-lto

example:
  compile.sh arm32v7 clang use_static_cpp=yes -j4

EOT
	exit 1
}

[ $# -ge 2 ] || usage

arch="$1"
preset="$2"
shift 2

case "$arch" in
	arm32v7|arm64v8)
		export PKG_CONFIG_PATH="$HOME/crossbuild/local/linux-$arch/lib/pkgconfig"
		cross_opts="frt_arch=$arch frt_cross=auto"
		;;
	native)
		cross_opts=""
		;;
	*)
		echo "compile.sh: unknwon <arch>: $arch"
		usage
		;;
esac

case "$preset" in
	clang)
		nice scons platform=frt use_llvm=yes use_lto=no \
		  tools=no target=release \
		  $cross_opts $*
		;;
	gcc)
		nice scons platform=frt use_llvm=no use_lto=no \
		  tools=no target=debug \
		  $cross_opts $*
		;;
	clang-lto)
		nice scons platform=frt use_llvm=yes use_lto=yes \
		  tools=no target=release \
		  $cross_opts $*
		;;
	gcc-lto)
		nice scons platform=frt use_llvm=no use_lto=yes \
		  tools=no target=debug \
		  $cross_opts $*
		;;
	*)
		echo "compile.sh: unknwon <preset>: $preset"
		usage
		;;
esac
