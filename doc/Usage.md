Usage
=====

FRT focuses on the runtime / export template only (i.e. not on the editor).
You can use the official editor on Windows, Linux or MacOS to package a game.

Compile FRT, or download and uncompress a binary from:

<https://sourceforge.net/projects/frt/files>

## Picking a version

### Platform: X11 vs FRT

Export templates for the official X11 platform can be easily generated and
they work on a Pi, *provided that you switch to the new GL driver*.
If this limitation is acceptable to you, there is no reason to use FRT.
Also, the X11 platform supports the X11 environment far better than FRT.

Nowadays, the main reason to use FRT is to run Godot on a KMS/DRM-based
distro like RetroPie.

## Exporting a game

### Godot 2.1

To pack a game, use the Linux X11 Platform. Uncheck "64 Bits",
uncheck "Debugging Enabled" and point the Custom Binary / Release file to
the uncompressed FRT binary.

### Godot 3.0 / 3.1

You can generally use the Linux X11 Platform as for Godot 2.
However, if you use compressed textures (e.g. 3D Games), you need to
select the appropriate texture format (Etc). Since this combination
(Etc for X11) might not be well tested upstream, you might need
to use the Android export template and generate a PCK/ZIP file instead.

## Binary Releases

Starting from FRT 0.9.3, binary releases follow the following naming
convention:

frt\_*frt-version*\_*godot-version*\_*arch-tag*.bin

For example:

frt\_110\_342\_arm32v7.bin

is FRT 1.1.0 compiled against Godot 3.4.2-stable. It is compiled for
a 32-bit distro.

The architecture tags have been changed in FRT 1.1.0. The change of
naming reflects different compilation flags being used.

- *arm32v6* and *arm32v7*. For 32-bit distros. Most users would want arm32v7.
  The arm32v6 versions are provided to support older Pis.

- *arm64v8*. For 64-bit distros. I hardly test them myself, but they are
  increasingly popular, so they are quite tested by users.

## Demos

A few exported demo projects are available for testing:

<https://sourceforge.net/projects/frt/files/demos>

For ease of notation, this page assumes that the chosen binary has been
renamed to `godot2.frt.bin` (for Godot 2.1) or `godot3.frt.bin`
(for Godot 3.0 - 3.2) and it has been installed somewhere in `$PATH`.

### Godot 2.1

You can unzip a demo and run it by either switching to its directory
or giving its directory as a command line option:

	~ $ mkdir demo
	~ $ cd demo
	~/demo $ unzip ~/Downloads/21_2d_platformer.zip
	~/demo $ godot2.frt.bin
	~/demo $ cd
	~ $ godot2.frt.bin -path demo

You cal also run a demo PCK/ZIP directly:

	~ $ godot2.frt.bin -main_pack ~/Downloads/21_2d_platformer.zip

You can finally put the demo PCK/ZIP in the same directory as the
binary and give them the same base name:

	~ $ mkdir tmp
	~ $ cd tmp
	~/tmp $ cp ~/Downloads/21_2d_platformer.zip platformer.zip
	~/tmp $ cp /usr/local/bin/godot2.frt.bin platformer.bin
	~/tmp $ ./platformer.bin

### Godot 3.0 - 3.2

Just like for Godot 2, you can unzip a demo (just use `--path`),
rename it, or run it directly (just use `--main-pack`).

The version of Godot 3.0 supported by FRT (3.0-gles2) defaults to GLES2.
If your board supports GLES3, you can run a GLES3 demo on it, but
you have to select the GLES3 renderer explicitly:

	~ $ godot3.frt.bin --video-driver GLES3 --main-pack ~/Downloads/30_2d_platformer.zip

GPU Particles should now be working. GLES3 has only been tested on a PC with
an Intel Graphics Card (Sandybridge Mobile) running debian stretch.

For a (quite heavy) 3D demo you can try:

	~ $ godot3.frt.bin --video-driver GLES3 --main-pack ~/Downloads/30_3d_material_testers.pck

Please note that the above Intel GPU can only render a few frames per seconds
of this demo. It might be helpful to add an option like `--resolution 320x240`
to the command line.
