#! /bin/sh
set -e

# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2019  Emanuele Fornara
#
# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

die() {
	echo $1
	exit 1
}

usage() {
	die "usage: release.sh arch tag..."
}

print_header() {
	echo "Building frt:$fver tag:$tag arch:$arch..."
}

release() {
	local bin
	[ -d releases ] || return
	bin=releases/frt_${fver}_${tag}_${arch}.bin
	cp tag_$tag/bin/godot.frt.opt$extrasuffix.$arch $bin
	$stripcmd $bin
}

build_common="platform=frt tools=no target=release -j 4"

build() {
	local patch
	print_header
	if [ $arch = arm64v8 -a $fver = 216 ] ; then patch="CCFLAGS=-DNO_THREADS" ; fi
	( cd tag_$tag ; nice scons frt_arch=$arch \
		$archopts \
		$gveropts \
		$patch \
	$build_common )
	release
}

[ $# -gt 1 ] || usage

arch=$1
case $arch in
	arm32v6)
		stripcmd="arm-linux-gnueabihf-strip"
		archopts="frt_cross=no"
		;;
	arm32v7)
		stripcmd="arm-linux-gnueabihf-strip"
		archopts="frt_cross=auto"
		;;
	arm64v8)
		stripcmd="aarch64-linux-gnu-strip"
		archopts="frt_cross=auto"
		;;
	*) die "release.sh: invalid arch: $arch."
esac
shift

[ -d releases ] || die "release.sh: no releases directory."

while [ $# -gt 0 ] ; do
	case $1 in
		tag_*)
			[ -d $1 ] || die "release.sh: tag directory $1 not found."
			tag=`echo $1 | cut -b 5- | sed 's/\///g'`
			frth="tag_$tag/platform/frt/frt.h"
			[ -f $frth ] || die "release.sh: $frth not found."
			gver=`echo $tag | cut -b -2`
			case $gver in
				2*)
					gveropts="use_llvm=yes"
					extrasuffix=".llvm"
					;;
				3*)
					gveropts="use_llvm=no module_webm_enabled=no"
					extrasuffix=""
					;;
				*) die "release.sh: unsupported godot version: $gver."
			esac
			fver=`grep FRT_VERSION $frth \
				| grep -o '".*"' \
				| sed 's/[\."]//g'`
			build
			;;
		*) die "release.sh: invalid tag $tag."
	esac
	shift
done
