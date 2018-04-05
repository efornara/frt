Usage
=====

FRT focuses on the runtime / export template only (i.e. not on the editor).
You can use the official editor on Windows, Linux or MacOS to package a game.

Compile FRT, or download and uncompress a binary from:

[https://sourceforge.net/projects/frt/files](https://sourceforge.net/projects/frt/files)

This page assumes that the binary has been renamed to
`godot2.frt.bin` or `godot3.frt.bin` and it has been installed somewhere
in PATH.

### Godot 2

To pack a game, use the Linux X11 Platform. Uncheck "64 Bits",
uncheck "Debugging Enabled" and point the Custom Binary / Release file to
the uncompressed FRT binary.

### Godot 3

Support for Godot 3 is experimental. For most boards (i.e. the ones
based on ES 2.0), only limited support for 2D games is available.
See [GLES2.md](https://github.com/efornara/godot/blob/3.0-gles2/GLES2.md)
for the details.

You can generally use the Linux X11 Platform as for Godot 2.
However, if you use compressed textures (e.g. 3D Games) you need
to use the Android export template and generate a PCK/ZIP file.

## Binary Releases

Starting from FRT 0.9.3, binary releases follow the following naming
convention:

frt\_*frt-version*\_*godot-version*\_*arch-tag*.bin

For example:

frt\_093\_302v1\_arm7hf.bin

is FRT 0.9.3 compiled against Godot 3.0-gles2, version 3.0.2-gles2-v1. It is
compiled for a generic arm7 Linux system.

Here is a description of the architecture tags:

- *pi1* and *pi2*. The pi1 versions are compatible with the Pi Zero and the
  first generation of Raspberry Pis. If you plan to publish your game and the
  game is simple enough, you should probably use these versions even if you
  have a faster Raspberry Pi. If your game is too heavy to run on an older
  Raspberry Pi, you might as well use the pi2 versions. These versions are the
  most tested.

- *arm7hf*. These versions should work on generic 32-bits Linux distributions
  like Armbian. They should work on X11 regardless of the graphics chipset.
  Without X11 (i.e. using fbdev), only initial support for the Mali chipsets
  is available. They have been tested on a Olimex A10-OLinuXino-LIME.

- *arm64*. These versions might work on generic 64-bits Linux distributions.
  Note that on this architecture Godot-2.1.x is compiled with
  `CCFLAGS=-DNO_THREADS`. They have never been tested on real hardware.

Early tests suggest that *pi1* versions also work on Armbian and *arm7hf*
versions also work on Raspbian, so, in the future, a reduction in the number
of architectures is possible.

## Demos

A few exported demo projects are available for testing:

[https://sourceforge.net/projects/frt/files/demos](https://sourceforge.net/projects/frt/files/demos)

### Godot 2

You can unzip a demo and run it by either switching to its directory
or giving its directory as a command line option:

	~ $ mkdir demo
	~ $ cd demo
	~/demo $ unzip ~/Downloads/21_2d_platformer.zip
	~/demo $ godot2.frt.bin
	~/demo $ cd
	~ $ godot2.frt.bin -path demo

You can also put the demo ZIP in the same directory as the binary and
give them the same base name:

	~ $ mkdir tmp
	~ $ cd tmp
	~/tmp $ cp ~/Downloads/21_2d_platformer.zip platformer.zip
	~/tmp $ cp /usr/local/bin/godot2.frt.bin platformer.bin
	~/tmp $ ./platformer.bin

### Godot 3

You can unzip a demo (just use `--path`) or rename it as for Godot 2.
However, you can also run a PCK/ZIP file directly:

	~ $ godot3.frt.bin --main-pack ~/Downloads/30_2d_platformer.zip

The version of Godot 3 supported by FRT (3.0-gles2) defaults to GLES2.
If your board supports GLES3, you can run a GLES3 demo on it, but
you have to select the GLES3 renderer explicitly:

	~ $ godot3.frt.bin --video-driver GLES3 --main-pack ~/Downloads/30_2d_platformer.zip

Particles should now be working. GLES3 has only been tested on a PC with an
Intel Graphics Card (Sandybridge Mobile) running debian stretch.

For a (quite heavy) 3D demo you can try:

	~ $ godot3.frt.bin --video-driver GLES3 --main-pack ~/Downloads/30_3d_material_testers.pck

Please note that the above Intel GPU can only render a few frames per seconds
of this demo. It might be helpful to add an option like `--resolution 320x240`
to the command line.
