#! /bin/sh
set -e

die() {
	echo $*
	exit 1
}

if [ ! -f scripts/debian/release.sh ] ; then
	echo "not in frt directory"
	exit 1
fi

if [ -x /usr/bin/arm-linux-gnueabihf-gcc ] ; then
	ARCH=armhf
	TRIPLE=arm-linux-gnueabihf
elif [ -x /usr/bin/aarch64-linux-gnu-gcc ] ; then
	ARCH=arm64
	TRIPLE=aarch64-linux-gnu
elif [ -x /usr/bin/riscv64-linux-gnu-gcc ] ; then
	ARCH=riscv64
	TRIPLE=riscv64-linux-gnu
else
	die "unknown arch/triple"
fi

FRT_HOME=`pwd`
mkdir -p releases

export BUILD_NAME=frt

FRTVER=`grep "define FRT_VERSION" frt.cc \
      | grep -o '".*"' \
      | sed 's/"//g'`-trixie

OPTIONS="
	platform=frt
	tools=no
	arch=${ARCH}
	triple=${TRIPLE}
	module_webm_enabled=no
	extra_suffix=trixie
	lto=full
	use_static_cpp=yes
	LINKFLAGS=-s
"

cd ../..
GODOT_HOME=`pwd`

compile_and_release() {
	PUBLISHED_TARGET=$1
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

	cd ${GODOT_HOME}
	scons ${OPTIONS} target=${COMPILE_TARGET}

	COMPILED_NAME=godot.frt.${BIN_TARGET}.${ARCH}.trixie.lto
	PUBLISHED_NAME=frt_${FRTVER}_${ARCH}_${PUBLISHED_TARGET}

	cd ${FRT_HOME}/releases
	cp ${GODOT_HOME}/bin/${COMPILED_NAME} ${PUBLISHED_NAME}
	rm -f ${PUBLISHED_NAME}.xz
	xz ${PUBLISHED_NAME}
}

compile_and_release debug
compile_and_release release
