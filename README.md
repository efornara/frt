FRT
===

[Godot](https://godotengine.org) is a full 2D and 3D game engine with editor.

FRT is a Godot 3 "platform" targeting single board computers. In plain English,
you can export a Godot game to most of them by using FRT binaries, *as long as
the game has been designed with the limitation of the hardware in mind*.

## When to use FRT

The latest versions of Godot 3 come with official export templates for
arm32 / arm64 architectures.
*Using the official export templates should be your first choice*.

The main difference is that FRT uses SDL2 under the hood, so, if your distro
uses something other than X11, you can try if FRT works better for you.
KMS/DRM and FBDEV are sometimes used, but Wayland has become more and more
popular, and it might be interesting to see if your game runs better
without going through xwayland.

## How to use FRT

Download the custom FRT templates from
[Releases](https://github.com/efornara/frt/releases)
and export to arm32 / arm64 as usual, selecting the FRT binaries in the
Custom Templates fields of the Options tab.

You can also export a pck / zip file and manually use the FRT template
to run it. This has the advantage that you only need to upload the FRT
binaries once.

Details vary depending on your target board, but the end result is usually
(a script with) a command that looks something like this:

    frt_3.6.1-1_arm64_release --main-pack MyGame.pck

FRT tracks the latest stable version of Godot 3. Older versions of FRT
can still be found here:

<https://sourceforge.net/projects/frt/files/>

## How to compile FRT yourself

The easiest way to compile FRT is to use docker.

Download [Dockerfile](https://raw.githubusercontent.com/efornara/frt/refs/heads/master/scripts/Dockerfile) and create a new docker image.

Start a new container and run:

    $ frt-pull

to clone (or pull) the right branches from <https://github.com/efornara/godot3>
and <https://github.com/efornara/frt>.

Then run:

    $ frt-compile

to generate the templates.

## Known issues

The arm32 templates terminate with "Illegal instruction" on some older boards.
The official templates behave the same way.
See <https://github.com/godotengine/godot/issues/112189> for more info.

If you are running a recent distro (e.g. trixie-based armbian),
as a stopgap you can try the armhf binaries from these experimental
[trixie](https://github.com/efornara/frt/releases/tag/3.6.2-1-trixie) builds.
