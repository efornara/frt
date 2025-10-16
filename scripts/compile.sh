#! /bin/sh
set -e

# compile.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2025  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

die() {
	echo $*
	exit 1
}

[ -d godot3 ] || die "no godot3/ in current directory"

cd godot3
BASEDIR=`pwd`

if [ ! -z "$FRT_SCONS_EXTRA" ] ; then
	echo "using FRT_SCONS_EXTRA: ${FRT_SCONS_EXTRA}"
else
	echo "FRT_SCONS_EXTRA is empty"
fi
END=${FRT_SCONS_EXTRA}

FRT_OPTIONS="
	platform=frt
	tools=no
	module_webm_enabled=no
"

PRODUCTION_OPTIONS="
	verbose=yes
	warnings=no
	progress=no
	production=yes
	LINKFLAGS=-s
"

compile_production() {
	OPTIONS="${FRT_OPTIONS} ${PRODUCTION_OPTIONS}"
	ARCH=$1
	case $ARCH in
		arm32)
			PATH=${GODOT_SDK_LINUX_ARM32}/bin:${SDL2_ARM32}/bin:${BASE_PATH}
			scons ${OPTIONS} arch=arm32 target=release ${END}
			;;
		arm64)
			PATH=${GODOT_SDK_LINUX_ARM64}/bin:${SDL2_ARM64}/bin:${BASE_PATH}
			scons ${OPTIONS} arch=arm64 target=release ${END}
			;;
		*)
			die "unknown build type: ${ARCH}"
			;;
	esac
}

DEVELOPMENT_OPTIONS="
	warnings=extra
"

compile_development() {
	OPTIONS="${FRT_OPTIONS} ${DEVELOPMENT_OPTIONS}"
	scons ${OPTIONS} target=release ${END}
}

if [ $# -eq 0 ] ; then
	compile_development
fi
while [ $# -ge 1 ] ; do
	compile_production $1
	shift
done
