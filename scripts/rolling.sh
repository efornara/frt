#! /bin/sh
set -e

# rolling.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2022  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

build_216_98() {
	echo 'Building Godot 2.1.6 (C++ 98)'
	cd tag_216
	nice scons platform=frt tools=no target=release use_llvm=yes frt_std=c++98 -j 3
	cd ..
}

build_216_14() {
	echo 'Building Godot 2.1.6 (C++ 14)'
	cd tag_216
	nice scons platform=frt tools=no target=release use_llvm=yes frt_std=c++14 -j 3
	cd ..
}

build_latest() {
	echo 'Building Godot 3.4.2 (C++ 20)'
	cd tag_342
	nice scons platform=frt tools=no target=release use_static_cpp=yes frt_std=c++20 -j 3
	cd ..
	strip tag_342/bin/godot.frt.opt
	cp tag_342/bin/godot.frt.opt releases/frt_342_amd64.bin
}

pack() {
	echo 'Packing Godot 3.4.2'
	cd releases
	gzip --keep --force frt_342_amd64.bin
	ls -l frt_342_amd64.bin.gz
	cd ..
}

case "$1" in
	298)
		build_216_98
		;;
	2*)
		build_216_14
		;;
	3*)
		build_latest
		;;
	all)
		build_216_98
		build_216_14
		build_latest
		pack
		;;
	*)
		echo '- default -'
		build_latest
		;;
esac
