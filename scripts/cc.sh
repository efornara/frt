#! /bin/sh
set -e

# cc.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2023  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

buildcommon="platform=frt frt_arch=amd64 frt_cross=auto use_llvm=yes -j 3"

build_216_98() {
	echo 'Building 216 (C++ 98)'
	cd tag_216
	nice scons tools=no target=release frt_std=c++98 $buildcommon
	cd ..
}

build_216_14() {
	echo 'Building 216 (C++ 14)'
	cd tag_216
	nice scons tools=no target=release frt_std=c++14 $buildcommon
	cd ..
}

build_351() {
	echo "Building 351 (C++ 20)"
	cd tag_351
	nice scons tools=no target=release frt_std=c++20 module_webm_enabled=no $buildcommon
	cd ..
}

build_400() {
	echo 'Building 400 (C++ 17)'
	cd tag_400
	nice scons target=template_release frt_std=c++17 optimize=speed debug_symbols=no $buildcommon
	cd ..
}

export BUILD_NAME=frt
case "$1" in
	298)
		build_216_98
		;;
	2*)
		build_216_14
		;;
	3*)
		build_351
		;;
	4*)
		build_400
		;;
	all)
		build_216_98
		build_216_14
		build_351
		build_400
		;;
	*)
		echo '- default -'
		build_351
		;;
esac
