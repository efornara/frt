Compile
=======

FRT is not really a patch, as it leaves the Godot source untouched. To
compile it, start from the official 2.1.2-stable source tarball:

[https://github.com/godotengine/godot/releases/tag/2.1.2-stable](https://github.com/godotengine/godot/releases/tag/2.1.2-stable)

Go to the platform directory and clone this repository:

	$ cd ~/godot-2.1.2-stable
	$ cd platform
	$ git clone https://github.com/efornara/frt

From the main directory, you now have a new "platform" available:

	$ cd ~/godot-2.1.2-stable
	$ scons platform=frt target=release frt_arch=pi3 -j 4

This should work on the upcoming 2.1.3 version too.

## Godot 3.0 compilation status

[![Build Status](https://api.travis-ci.org/efornara/frt.svg?branch=master)](https://travis-ci.org/efornara/frt/builds)

An eventual error here does not reflect the compilation status of FRT on the
latest stable Godot branch (2.1).
