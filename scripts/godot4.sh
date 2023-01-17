#! /bin/sh
set -e

# godot4.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2023  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

usage() {
	cat <<END
usage: godot4.sh [-a arch] [-s srcdir] [-e exportdir] [-j jobs] [-v] [-p|-d]
END
	exit 1
}

die() {
	echo "godot4.sh: $1"
	exit 1
}

arch=auto
srcdir=4.0
exportdir=releases
while getopts a:s:e:j:vpd name ; do
	case $name in
		a) arch="$OPTARG" ;;
		s) srcdir="$OPTARG" ;;
		e) exportdir="$OPTARG" ;;
		j) jobsopts="-j $OPTARG" ;;
		v) verboseopts="verbose=yes" ;;
		p) publish=1 ;;
		d) debug=1 ;;
		?) usage ;;
	esac
done
[ -z "$publish" -o -z "$debug" ] || usage

if [ $arch = auto ] ; then
	if [ -f /usr/bin/arm-linux-gnueabihf-strip ] ; then
		arch=arm32v7
	elif [ -f /usr/bin/aarch64-linux-gnu-strip ] ; then
		arch=arm64v8
	else
		arch=x86_64
	fi
fi
case $arch in
	arm32v6) stripcmd="arm-linux-gnueabihf-strip" ;;
	arm32v7) stripcmd="arm-linux-gnueabihf-strip" ;;
	arm64v8) stripcmd="aarch64-linux-gnu-strip" ;;
	x86_64) stripcmd="x86_64-linux-gnu-strip" ;;
	*) die "invalid arch: $arch."
esac

[ -d $srcdir ] || die "source directory $srcdir not found."
if [ -f $srcdir/suffix.txt ] ; then
	godot_suffix=`cat $srcdir/suffix.txt`
	export GODOT_VERSION_STATUS=beta$godot_suffix
fi
tag="400b$godot_suffix"

if [ ! -z "$debug" ] ; then
	build="(debug)"
	buildopts="target=template_debug optimize=debug debug_symbols=yes separate_debug_symbols=yes"
else
	build="(release)"
	buildopts="target=template_release optimize=speed debug_symbols=no use_static_cpp=yes"
fi

echo "Building tag $tag for $arch $build..."

export BUILD_NAME=frt
( cd $srcdir ; nice scons \
	platform=frt \
	frt_arch=$arch \
	frt_cross=auto \
	use_llvm=yes \
	$buildopts $verboseopts $jobsopts
)

if [ ! -z "$publish" ] ; then
	echo "Publishing:"
	[ -d $exportdir ] || die "export directory $exportdir not found."
	commitid=`git -C $srcdir/platform/frt rev-parse HEAD | cut -b -7`
	bin=$exportdir/frt_${commitid}_${tag}_${arch}.bin
	cp $srcdir/bin/godot.frt.template_release.auto.llvm.$arch $bin
	$stripcmd $bin
	ls -l $bin
fi
