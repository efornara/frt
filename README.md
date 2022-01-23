FRT
===

[Godot](https://godotengine.org) is a full 2D and 3D game engine with editor.

FRT is a Godot "platform" targeting single board computers. In plain English,
you can export a Godot game to most of them by using FRT binaries, *as long as
the game has been designed with the limitation of the hardware in mind*.

## When to use FRT

Godot comes with a generic X11 platform that works very well on most
modern single board computer. So, if you are using X11, it is probably
better to compile the official engine for ARM and use that.

If your distro uses something other than X11, you can try if FRT works
for you. KMS/DRM and FBDEV are common display technologies, but Wayland
is also slowly becoming more popular.

## How to use FRT

First, you need to export a game from the official Godot editor.
The platform where you run the editor doesn't matter.

One option is to use precompiled binaries from here:

<https://sourceforge.net/projects/frt/files/>

as custom templates. Another option is to export a .PCK file and use
the FRT binary to run it. Details vary, but the end result is usually
a script with a command that looks something like this:

    ./frt_200_342_arm64v8.bin --main-pack MyGame.pck

There are some guides and posts around describing the process for FRT 1.0.
The process is pretty much the same for FRT 2.0.

### Which version?

FRT binary releases follow the following naming convention:

frt\_*frt-version*\_*godot-version*\_*arch-tag*.bin

For example:

frt\_200\_342\_arm32v7.bin

is FRT 2.0.0 compiled against Godot 3.4.2-stable. It is compiled for
a 32-bit distro. While:

frt\_200\_342\_arm64v8.bin

is compiled for a 64-bit distro.
The arm32v6 ones are there mainly to support older Pis.

My policy is to publish binaries for the latest release from upstream
plus 2.1.6 and the ones that were/are in debian stable (currenty 3.0.6
and 3.2.3). You are encouraged to compile any version you need yourself.
See [Compile](doc/Compile.md) for more info.

Ideally the codebase should be able to support building against any
upstream stable version since 2.1.6, but this is rarely tested.

## SDL2

Starting from version 2.0, FRT uses and dynamically links the SDL2 library,
so you can leverage a custom version of SDL2 patched for your board and
distro.

Keep in mind that SDL2 is a fairly complex library, and you can customize
its behaviour using environment variables. Before looking for alternatives
to the version of SDL2 already installed, it is probably worth spending some
time testing different drivers and options.

### Example: Pi Zero (older model)

In my (limited) experience, an exception is older Pis (the ones best used
with legacy drivers), where the SDL that you get out of the box is not
that great.

This would also serve as an example of how to use a custom SDL library.

Download a binary archive from here:

<https://github.com/efornara/sdl2/releases>

uncompress it somewhere, and run the FRT binary like this:

    export LD_LIBRARY_PATH=~/local/sdl/linux-arm32v6
    ./frt_200_216_arm32v6.bin -path ~/games/mygame
