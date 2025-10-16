#! /bin/sh
set -e

# frt-clone.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2025  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

if [ -d godot3 ] ; then
	cd godot3
	git pull
	cd ..
else
	git clone -b frt https://github.com/efornara/godot3
fi

cd godot3/platform
if [ -d frt ] ; then
	cd frt
	git pull
else
	git clone https://github.com/efornara/frt
fi
