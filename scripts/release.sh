#! /bin/sh
set -e

# release.sh
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2025  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

if [ ! -f scripts/compile.sh ] ; then
	echo "not in frt directory"
	exit 1
fi

unset FRT_SCONS_EXTRA
export FRT_BUILD_TYPE=prod
export BUILD_NAME=frt

FRTVER=`grep "define FRT_VERSION" frt.cc \
      | grep -o '".*"' \
      | sed 's/"//g'`

compile_and_release() {
	ARCH=$1
	PUBLISHED_TARGET=$2
	case ${PUBLISHED_TARGET} in
		release)
			BIN_TARGET="opt"
			COMPILE_TARGET="release"
			;;
		debug)
			BIN_TARGET="opt.debug"
			COMPILE_TARGET="release_debug"
			;;
	esac

	./scripts/compile.sh ${ARCH}-${COMPILE_TARGET}

	COMPILED_NAME=godot.frt.${BIN_TARGET}.${ARCH}
	PUBLISHED_NAME=frt_${FRTVER}_${ARCH}_${PUBLISHED_TARGET}

	cp ../../bin/${COMPILED_NAME} releases/${PUBLISHED_NAME}
	cd releases
	xz ${PUBLISHED_NAME}
	cd ..
}

mkdir -p releases
compile_and_release arm32 release
compile_and_release arm32 debug
compile_and_release arm64 release
compile_and_release arm64 debug

ls -l releases/*${FRTVER}*
