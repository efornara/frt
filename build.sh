#! /bin/sh
set -e

# build.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2023  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

# WARNING:
#  This script is quite complex.
#  A better place to start is scripts/cc.sh

usage() {
	cat <<END
usage: build.sh [-a arch] [-j jobs] [-v] [-p]
END
	exit 1
}

die() {
	echo "build.sh: $1."
	exit 1
}

parse_godot_version() {
	gver=`echo $srcdir | cut -b 5- | sed 's/\///g'`
	if [ ! -f $srcdir/suffix.txt ] ; then
		unset GODOT_VERSION_STATUS
		gverext=$gver
	else
		gsta=`cat $srcdir/suffix.txt`
		case $gsta in
			alpha*) shortsta=a ; gstaid=`echo $gsta | cut -b 6-` ;;
			beta*) shortsta=b ; gstaid=`echo $gsta | cut -b 5-` ;;
			rc*) shortsta=c ; gstaid=`echo $gsta | cut -b 3-` ;;
			*) die "unsupported godot status: $gsta"
		esac
		export GODOT_VERSION_STATUS=$gsta
		gverext="$gver$shortsta$gstaid"
	fi
}

parse_frt_version() {
	frtcc="$srcdir/platform/frt/frt.cc"
	[ -f $frtcc ] || die "$frtcc not found"
	fver=`grep "define FRT_VERSION" $frtcc \
		| grep -o '".*"' \
		| sed 's/[\."]//g'`
	fsta=`grep "define FRT_STATUS" $frtcc \
		| grep -o '".*"' \
		| sed 's/"//g'`
	if [ $fsta = stable ] ; then
		fverext=$fver
	else
		commitid=`git -C $srcdir/platform/frt rev-parse HEAD | cut -b -7`
		shortsta=`echo $fsta | cut -b -1`
		fverext="$fver$shortsta-$commitid"
	fi
}

build() {
	local patch
	echo "Building frt:$fverext tag:$gverext arch:$arch..."
	if [ $arch = arm64v8 -a $fver = 216 ] ; then patch="CCFLAGS=-DNO_THREADS" ; fi
	( cd $srcdir ; nice scons \
		platform=frt \
		frt_arch=$arch \
		frt_cross=auto \
		use_llvm=yes \
		use_static_cpp=yes \
		$patch \
		$buildopts $verboseopts $jobsopts
	)
}

publish() {
	echo "Publishing..."
	[ -d $exportdir ] || die "export directory $exportdir not found."
	bin=$exportdir/frt_${fverext}_${gverext}_${arch}.bin
	cp $srcdir/bin/$srcbin $bin
	$stripcmd $bin
	ls -l $bin
}

arch=auto
exportdir=releases
while getopts a:e:j:vpd name ; do
	case $name in
		a) arch="$OPTARG" ;;
		e) exportdir="$OPTARG" ;;
		j) jobsopts="-j $OPTARG" ;;
		v) verboseopts="verbose=yes" ;;
		p) publish=1 ;;
		?) usage ;;
	esac
done
shift $(expr $OPTIND - 1 )

if [ $arch = auto ] ; then
	if [ -f /usr/bin/arm-linux-gnueabihf-strip ] ; then
		if [ -f /etc/dpkg/origins/raspbian ] ; then
			arch=arm32v6
		else
			arch=arm32v7
		fi
	elif [ -f /usr/bin/aarch64-linux-gnu-strip ] ; then
		arch=arm64v8
	else
		arch=amd64
	fi
fi
case $arch in
	arm32v6) stripcmd="arm-linux-gnueabihf-strip" ;;
	arm32v7) stripcmd="arm-linux-gnueabihf-strip" ;;
	arm64v8) stripcmd="aarch64-linux-gnu-strip" ;;
	amd64) stripcmd="x86_64-linux-gnu-strip" ;;
	*) die "invalid arch: $arch"
esac

export BUILD_NAME=frt
while [ $# -gt 0 ] ; do
	srcdir=$1
	case $srcdir in
		tag_*)
			[ -d $srcdir ] || die "tag directory $srcdir not found"
			parse_godot_version
			parse_frt_version
			case $gver in
				2*)
					buildopts="target=release tools=no"
					srcbin="godot.frt.opt.llvm.$arch"
					;;
				3*)
					buildopts="target=release tools=no module_webm_enabled=no"
					srcbin="godot.frt.opt.llvm.$arch"
					;;
				4*)
					buildopts="target=template_release optimize=speed debug_symbols=no"
					srcbin="godot.frt.template_release.auto.llvm.$arch"
					;;
				*) die "unsupported godot version: $gver"
			esac
			build
			if [ ! -z "$publish" ] ; then publish ; fi
			;;
		*) die "invalid tag $srcdir"
	esac
	shift
done
