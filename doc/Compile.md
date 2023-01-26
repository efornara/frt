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

Here it is assumed that the top directory of the Godot source is ~/godot.

Go to the platform directory and clone this repository:

	$ cd ~/godot
	$ cd platform
	$ git clone -b 2.0 https://github.com/efornara/frt

From the main directory, you now have a new platform available:

	$ cd ~/godot
	$ scons platform=frt tools=no target=release use_llvm=yes -j 4

_NOTE: If you only have access to a recent version of gcc (i.e. 6 or later),
you can only compile Godot 2 by using debug as a target._

## Cross compilation, crossbuild and build.sh

As an example, here are the steps needed to build some binaries on
a PC using docker. Even if you don't use docker, my crossbuild images
or the official script, these instructions, the Dockerfiles and the
script should have all the info that you need.

Create the docker images:

	$ git clone https://github.com/efornara/crossbuild
	$ cd crossbuild/docker
	$ ./build.sh base
	[...]
	$ ./build.sh arm32v7
	[...]
	$ ./docker.sh arm32v7
	root@.....:/# su - builder
	builder@.....:$

You should be able to su to your user (builder in my case) and find your
home directory.

You might have everything you need in the container, but I usually
open another terminal and work outside the container, except for
compiling.

The build.sh script needs the current directory to look like this:

	$ ls ~/somewhere
	releases/ tag_216/ tag_342/

A directory where to copy the binaries (releases/) and every godot
version in its own directory named according to its "tag" version
and with frt already inside platform (see above).

From within the container, type:

	$ cd ~/somewhere
	$ ./tag_216/platform/frt/build.sh -p tag_*

If everything goes well, after a while you should have your binaries
ready in ~/somewhere/releases.
