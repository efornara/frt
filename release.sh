#! /bin/sh

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
	die "usage: release.sh [--pulse] arch tag..."
}

print_header() {
	echo "Building frt:$fver tag:$tag arch:$arch suffix:[$suffix]..."
}

release() {
	local bin
	[ -d releases ] || return
	bin=releases/frt_${fver}_${tag}_${arch}${suffix}.bin
	cp tag_$tag/bin/godot.frt.opt$suffix.$arch $bin
	strip $bin
}

build_common="platform=frt target=release -j 4"

build_21() {
	local patch
	print_header
	if [ $arch = arm64 ] ; then patch="CCFLAGS=-DNO_THREADS" ; fi
	( cd tag_$tag ; scons frt_arch=$arch \
		builtin_zlib=no \
		builtin_freetype=no \
		module_openssl_enabled=no \
		$pulse \
		$patch \
	$build_common )
	release
}

build_30() {
	print_header
	( cd tag_$tag ; scons frt_arch=$arch \
		builtin_zlib=no \
		builtin_freetype=yes \
		module_openssl_enabled=no \
		module_webm_enabled=no \
		$pulse \
	$build_common )
	release
}

build_31() {
	print_header
	( cd tag_$tag ; scons frt_arch=$arch \
		warnings=no \
		builtin_zlib=no \
		builtin_freetype=yes \
		builtin_mbedtls=no \
		builtin_libwebsockets=no \
		module_mbedtls_enabled=no \
		module_websocket_enabled=no \
		module_webm_enabled=no \
		$pulse \
	$build_common )
	release
}

build_33() {
	print_header
	( cd tag_$tag ; scons frt_arch=$arch \
		warnings=no \
		builtin_zlib=no \
		builtin_freetype=yes \
		builtin_mbedtls=no \
		builtin_libwebsockets=no \
		module_mbedtls_enabled=no \
		module_websocket_enabled=no \
		module_webm_enabled=no \
		$pulse \
	$build_common )
	release
}

build_32() {
	echo "WARNING: 3.2.x deprecated upstream"
	build_33
}

[ $# -gt 1 ] || usage

if [ $1 = "--pulse" ] ; then
	pulse="pulseaudio=yes extra_suffix=pulse"
	suffix=".pulse"
	shift
else
	pulse="pulseaudio=no"
	suffix=""
fi

[ $# -gt 1 ] || usage

varch=$1
case $varch in
	pi|pi1|pi2|pi3|pc|arm64) ;;
	*) die "release.sh: invalid arch: $varch."
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
				21|30|31|32|33) ;;
				*) die "release.sh: unsupported godot version: $gver."
			esac
			fver=`grep FRT_VERSION $frth \
				| grep -o '".*"' \
				| sed 's/[\."]//g'`
			case $varch in
				pi)
					arch=pi1 ; build_$gver
					arch=pi2 ; build_$gver
					;;
				*) arch=$varch build_$gver
			esac
			;;
		*) die "release.sh: invalid tag $tag."
	esac
	shift
done
