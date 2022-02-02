#! /bin/sh
set -e

# rolling.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2022  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

if [ $# -ge 2 -a "$1" = '-t' ] ; then
	tag=$2
	shift 2
else
	tag=342
fi

build_216_98() {
	echo 'Building 216 (C++ 98)'
	cd tag_216
	nice scons platform=frt tools=no target=release use_llvm=yes frt_std=c++98 -j 3
	cd ..
}

build_216_14() {
	echo 'Building 216 (C++ 14)'
	cd tag_216
	nice scons platform=frt tools=no target=release use_llvm=yes frt_std=c++14 -j 3
	cd ..
}

build_latest() {
	echo "Building ${tag} (C++ 20)"
	cd tag_${tag}
	nice scons platform=frt tools=no target=release use_static_cpp=yes frt_std=c++20 -j 3
	cd ..
	strip tag_${tag}/bin/godot.frt.opt
	cp tag_${tag}/bin/godot.frt.opt releases/frt_${tag}_amd64.bin
}

pack() {
	echo "Packing ${tag}"
	cd releases
	gzip --keep --force frt_${tag}_amd64.bin
	ls -l frt_${tag}_amd64.bin.gz
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
