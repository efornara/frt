#! /bin/sh
set -e

# compile.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2025  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

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

# ### WARNING ###
# This file is quite complex and assume some skills with the shell.
# A better place to start for your own compilation script is basic_compile.sh

X11_OPTIONS="
	platform=x11
	tools=no
"

DEVELOPMENT_OPTIONS="
	warnings=extra
"

die() {
	echo $*
	exit 1
}

if [ -f godot3/platform/frt/README.md ] ; then
	cd godot3
elif [ -f scripts/compile.sh ] ; then
	cd ../..
elif [ -f basic_compile.sh ] ; then
	cd ../../..
elif [ ! -f platform/frt/README.md ] ; then
	die "unknown current directory"
fi

if [ ! -z "${FRT_SCONS_EXTRA}" ] ; then
	echo "using FRT_SCONS_EXTRA: ${FRT_SCONS_EXTRA}"
else
	echo "FRT_SCONS_EXTRA is empty"
fi
END="${FRT_SCONS_EXTRA}"

if [ ! -z "${FRT_BUILD_TYPE}" ] ; then
	echo "using FRT_BUILD_TYPE: ${FRT_BUILD_TYPE}"
else
	echo "FRT_BUILD_TYPE is empty, using dev"
	FRT_BUILD_TYPE=dev
fi

case ${FRT_BUILD_TYPE} in
	prod)
		PLATFORM_OPTIONS="${FRT_OPTIONS}"
		BUILDTYPE_OPTIONS="${PRODUCTION_OPTIONS}"
		;;
	dev)
		PLATFORM_OPTIONS="${FRT_OPTIONS}"
		BUILDTYPE_OPTIONS="${DEVELOPMENT_OPTIONS}"
		;;
	x11)
		PLATFORM_OPTIONS="${X11_OPTIONS}"
		BUILDTYPE_OPTIONS="${DEVELOPMENT_OPTIONS}"
		;;
	*)
		die "unknown FRT_BUILD_TYPE: ${FRT_BUILD_TYPE}"
		;;
esac

compile() {
	ARCH=$1
	case ${ARCH} in
		arm32)
			PATH="${GODOT_SDK_LINUX_ARM32}/bin:${SDL2_ARM32}/bin:${BASE_PATH}"
			scons ${OPTIONS} arch=arm32 target=release ${END}
			;;
		arm64)
			PATH="${GODOT_SDK_LINUX_ARM64}/bin:${SDL2_ARM64}/bin:${BASE_PATH}"
			scons ${OPTIONS} arch=arm64 target=release ${END}
			;;
		native)
			scons ${OPTIONS} target=release ${END}
			;;
		*)
			die "unknown arch: ${ARCH}"
			;;
	esac
}

OPTIONS="${PLATFORM_OPTIONS} ${BUILDTYPE_OPTIONS}"
if [ $# -eq 0 ] ; then
	if [ ! -z "${GODOT_SDK_LINUX_ARM32}" ] ; then
		compile arm32
		compile arm64
	else
		compile native
	fi
fi
while [ $# -ge 1 ] ; do
	compile $1
	shift
done
