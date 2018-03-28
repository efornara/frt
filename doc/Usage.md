Usage
=====

FRT focuses on the runtime / export template only (i.e. not on the editor).
You can use the official editor on Windows, Linux or MacOS to package a game.

Compile FRT, or download and uncompress a binary from
[here](https://sourceforge.net/projects/frt/files).
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

## Demos

A few exported demo projects are available for testing
[here](https://sourceforge.net/projects/frt/files/demos).

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
If your board supports GLES3, you can try to run a GLES3 demo on it, but
you have to select the GLES3 renderer explicitly:

	~ $ godot3.frt.bin --video-driver GLES3 --main-pack ~/Downloads/30_3d_material_testers.zip

This has only been tested on a PC with an Intel Graphics Card
(Sandybridge Mobile) running debian stretch.
