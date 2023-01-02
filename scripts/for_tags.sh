#! /bin/sh
set -e

# for_tags.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2023  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

usage() {
	echo 'usage: for_tags.sh -g|-p|-f command...'
	exit 1
}

[ $# -ge 1 ] || usage

arg="$1"
shift

case "$arg" in
	-g) for i in tag_* ; do (cd $i ; $*) ; done ;;
	-p) for i in tag_* ; do (cd $i/platform ; $*) ; done ;;
	-f) for i in tag_* ; do (cd $i/platform/frt ; $*) ; done ;;
	*) usage
esac
