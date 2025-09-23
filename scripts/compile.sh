#! /bin/sh
set -e

# compile.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2025  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

BASEDIR=`pwd`

if [ -d godot3 ] ; then
	cd godot3
	git pull
else
	git clone -b frt https://github.com/efornara/godot3
fi

cd ${BASEDIR}/godot3/platform
if [ -d frt ] ; then
	cd frt
	git pull
else
	git clone https://github.com/efornara/frt
fi

cd ${BASEDIR}/godot3

OPTIONS="
	platform=frt
	tools=no
	module_webm_enabled=no
	verbose=yes
	warnings=no
	progress=no
	production=yes
	LINKFLAGS=-s
"

PATH=${GODOT_SDK_LINUX_ARM64}/bin:${SDL2_ARM64}/bin:${BASE_PATH}
scons ${OPTIONS} arch=arm64 target=release

PATH=${GODOT_SDK_LINUX_ARM32}/bin:${SDL2_ARM32}/bin:${BASE_PATH}
scons ${OPTIONS} arch=arm32 target=release
