#! /bin/sh
set -e

# basic_compile.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2025  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

FRT_OPTIONS="
	platform=frt
	tools=no
"

PRODUCTION_OPTIONS="
	verbose=yes
	warnings=no
	progress=no
	production=yes
	LINKFLAGS=-s
"

# Options above are manually kept in sync with compile.sh.
# If they differ, the ones in compile.sh are the right ones.

if [ ! -f godot3/platform/frt/README.md ]
then
	echo
	echo "File godot3/platform/frt/README.md does not exist!"
	echo
	echo "Have you cloned the repository with frt-pull?"
	echo "Are you in the right directory? (ls should show godot3)"
	echo
	exit 1
fi
cd godot3

if [ -z "${GODOT_SDK_LINUX_ARM64}" ]
then
	echo
	echo "GODOT_SDK_LINUX_ARM64 is not set!"
	echo
	echo "Are you inside the docker container?"
	echo
	exit 1
fi

OPTIONS="${FRT_OPTIONS} ${PRODUCTION_OPTIONS}"

export BUILD_NAME=frt

PATH="${GODOT_SDK_LINUX_ARM32}/bin:${SDL2_ARM32}/bin:${BASE_PATH}"
scons ${OPTIONS} arch=arm32 target=release
scons ${OPTIONS} arch=arm32 target=release_debug

PATH="${GODOT_SDK_LINUX_ARM64}/bin:${SDL2_ARM64}/bin:${BASE_PATH}"
scons ${OPTIONS} arch=arm64 target=release
scons ${OPTIONS} arch=arm64 target=release_debug
