#! /bin/sh
set -e

# godot4.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2023  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

die() {
	echo $1
	exit 1
}

usage() {
	die "usage: godot4.sh [arch]"
}

print_header() {
	echo "Building tag:$tag arch:$arch..."
}

release() {
	local bin
	[ -d releases ] || return
	bin=releases/frt_${commitid}_${tag}_${arch}.bin
	cp tag_400/bin/godot.frt.template_release.auto.llvm.$arch $bin
	$stripcmd $bin
}

build_common="platform=frt target=template_release use_llvm=yes use_static_cpp=yes -j 4"

build() {
	print_header
	( cd tag_400 ; nice scons frt_arch=$arch $archopts $build_common )
	release
}

if [ $# -eq 0 ] ; then
	if [ -f /usr/bin/arm-linux-gnueabihf-strip ] ; then
		arch=arm32v7
	elif [ -f /usr/bin/aarch64-linux-gnu-strip ] ; then
		arch=arm64v8
	else
		arch=x86_64
	fi
elif [ $# -eq 1 ] ; then
	arch=$1
	shift
else
	usage
fi

case $arch in
	arm32v7)
		stripcmd="arm-linux-gnueabihf-strip"
		archopts="frt_cross=auto"
		;;
	arm64v8)
		stripcmd="aarch64-linux-gnu-strip"
		archopts="frt_cross=auto"
		;;
	x86_64)
		stripcmd="x86_64-linux-gnu-strip"
		archopts="frt_cross=auto"
		;;
	*) die "godot4.sh: invalid arch: $arch."
esac

[ -d releases ] || die "godot4.sh: no releases directory."
[ -d tag_400 ] || die "godot4.sh: tag directory tag_400 not found."

if [ -f tag_400/suffix.txt ] ; then
	GODOT_SUFFIX=`cat tag_400/suffix.txt`
fi

commitid=`git -C tag_400/platform/frt rev-parse HEAD | cut -b -7`
tag="400$GODOT_SUFFIX"
export BUILD_NAME=frt
build
