Compile
=======

## Native Compilation

As a prerequisite for compiling FRT, these packages should be enough
(using debian as an example):

	$ sudo apt-get install build-essential scons pkg-config \
	  clang llvm lld libsdl2-dev libgles2-mesa-dev

Download and uncompress an official Godot source tarball from:

<https://github.com/godotengine/godot/releases>

or:

<https://downloads.tuxfamily.org/godotengine>

_NOTE: At the moment, only Godot 2 is implemented and only Godot 2.1.6-stable
has been tested._

Here it is assumed that the top directory of the Godot source is `~/godot`.

Go to the platform directory and clone this repository:

	$ cd ~/godot
	$ cd platform
	$ git clone -b 2.0 https://github.com/efornara/frt

From the main directory, you now have a new platform available:

	$ cd ~/godot
	$ scons platform=frt tools=no target=release use_llvm=yes -j 4

_NOTE: If you only have access to a recent version of gcc (i.e. 6 or later),
you can only compile Godot 2 by using debug as a target._
