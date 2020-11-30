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

There are two reasons to use FRT:

1. You want to publish your game and you don't want to force your users
to switch to the new GL driver.

2. You want to support other single board computers. The Pi
is the exception here: a person was hired to write an OpenGL desktop driver
for its SOC. Other boards usually adapt an OpenGL ES driver designed for
Android, and this is usually exposed to games via EGL/ES.

If you develop using the X11 platform and plan to release with FRT,
keep in mind that the two underlying drivers (vc4 and bcm) perform
differently. FRT detects at run-time which one to load, but you might
want to test with both of them anyway.

### Godot: 3.1 vs 2.1

Here is my personal opinion.

If you develop a 2D game on your PC and you only care about new generation
Pis (2/3), you should probably use Godot 3.1. It is slightly
slower than Godot 2.1, but, as an example, the 2D platform is capped
at 60 fps in both on a (non-overclocked) Pi 3B+.
Godot 3.1 is nicer to use and the community has largely migrated to Godot 3.

However, you might want to consider Godot 2.1 in some cases:

1. You want to develop a game on the Pi itself.
While the Godot 2.1 *editor* seems to be useable on a modern Pi
(3B+ with vc4), the Godot 3.1 *editor* doesn't seem to be.
It works, but I wouldn't want to actually write a game with it.
UPDATE: Compiling with the latest compilers (e.g. gcc 8 or clang 8), and
, if possible, enabling lto, might help to make the 3.1 editor useable.

2. You target slow boards (e.g. Pi 1 / Zero).

3. You want to develop a 3D game.

For the last two points, it is probably better to write a minimal prototype
of your game in both Godot 2.1 / 3.1 and measure the performance you are
getting.

Also note that Godot 3.1 is not as tested as Godot 2.1 on the Pi,
and some issues might come up. For example, particles are known not to work:
[#27407](https://github.com/godotengine/godot/issues/27407).

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

frt\_093\_302v1\_pi2.bin

is FRT 0.9.3 compiled against Godot 3.0-gles2, version 3.0.2-gles2-v1. It is
compiled for fairly recent ARM-based 32-bit boards.

Here is a description of the architecture tags:

- *pi1* and *pi2*. The pi1 versions are compatible with the Pi Zero and the
  first generation of Pis. If you plan to publish your game and the
  game is simple enough, you should probably use these versions even if you
  have a faster Pi. If your game is too heavy to run on an older
  Pi, you might as well use the pi2 versions. These versions are the
  most tested. They should work on other boards too, and, beside on
  Pis, they have also been tested on a A10-based board.

- *arm64*. These versions should work on generic 64-bits Linux distributions.
  Note that, on this architecture, Godot 2.1 is compiled with
  `CCFLAGS=-DNO_THREADS`. They have been tested on a Pi 3B+
  running [Gentoo 64-bit](https://github.com/sakaki-/gentoo-on-rpi3-64bit)
  (vc4 driver).

An older version of FRT (0.9.3) also came compiled for *arm7hf*.
Its use is now discouraged. Use pi1 or pi2 instead.

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

### Godot 3.0? For OpenGL ES 2.0?

Godot 3.0 is not supposed to run on a on OpenGL ES 2.0 class device.

I backported a limited, old version of the 3.1 GLES2 renderer to Godot 3.0
(3.0-gles2). See here:
[GLES2.md](https://github.com/efornara/godot/tree/3.0-gles2/GLES2.md).

Since Godot 3.1 has now been released and it officially supports
OpenGL ES 2.0, users still using 3.0-gles2 are advised to migrate to
Godot 3.1 or later.
