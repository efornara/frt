FRT
===

FRT is a Godot "platform" targeting single board computers.

## Usage

If you have a raspberry Pi with raspbian, you don't need to compile from
source.

Make sure that you have increased the amount of memory for the GPU. I
use 256M on a 512M Raspberry Pi B.

Download, uncompress, install somewhere in your path one binary from
here:

[https://sourceforge.net/projects/frt/files](https://sourceforge.net/projects/frt/files)

	$ gunzip godot.frt.opt.pi1.gz
	$ sudo install godot.frt.opt.pi1 /usr/local/bin

Download and uncompress the "Demos and Examples" archive from
here:

[https://godotengine.org/download](https://godotengine.org/download)

Run a demo:

	$ cd Godot-Demos-2.1-20170121/2d/platformer
	$ godot.frt.opt.pi1 -v

*IMPORTANT*: Press **Win+Q** to exit.

## Compiling

FRT is not really a patch, as it leaves the godot source untouched. To
compile, start from the official 2.1.2-stable source tarball:

[https://github.com/godotengine/godot/releases/tag/2.1.2-stable](https://github.com/godotengine/godot/releases/tag/2.1.2-stable)

Go to the platform directory and clone this repository:

	$ cd ~/godot-2.1.2-stable
	$ cd platform
	$ git clone https://github.com/efornara/frt

From the main directory, you have a new "platform" available:

	$ cd ~/godot-2.1.2-stable
	$ scons platform=frt target=release tools=no frt_arch=pi3 -j 4

This should work on the upcoming 2.1.3 version too.
