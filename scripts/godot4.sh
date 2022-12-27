#! /bin/sh
set -e

# godot4.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2022  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

die() {
	echo $1
	exit 1
}

usage() {
	die "usage: godot4.sh arch"
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

[ $# -eq 1 ] || usage

arch=$1
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
shift

[ -d releases ] || die "godot4.sh: no releases directory."
[ -d tag_400 ] || die "godot4.sh: tag directory tag_400 not found."

if [ -f tag_400/suffix.txt ] ; then
	GODOT_SUFFIX=`cat tag_400/suffix.txt`
fi

commitid=`git -C tag_400/platform/frt rev-parse HEAD | cut -b -7`
tag="400$GODOT_SUFFIX"
export BUILD_NAME=frt
build
