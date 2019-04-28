FRT
===

FRT is a Godot "platform" targeting single board computers.

## Quick Start

FRT should run on most single board computers, as long as OpenGL ES has
been properly configured. On a Raspberry Pi, FRT does not need X11 to run,
and can also run from the console using the legacy driver (it should
run out of the box on a basic raspbian lite installation).

Download, uncompress, install somewhere in your path the latest
binary from here:

<https://sourceforge.net/projects/frt/files/0.9.5>

For example:

	$ unzip frt_095_310.zip
	$ sudo install frt_095_310_pi1.bin /usr/local/bin

Download and uncompress a demo from here:

<https://sourceforge.net/projects/frt/files/demos>

And run it:

	$ frt_095_310_pi1.bin --main-pack 31_2d_platformer.zip

Press *Win+Q* to quit.

Despite the name, the _pi1_ version should run on any Raspberry Pi,
using either the legacy driver, or the new vc4 driver. It should also run
on most 32-bit ARM-based boards (X11/EGL/ES).

For 64-bit ARM-based boards, use the _arm64_ version.

See [Usage](doc/Usage.md) for how to export a game from the Godot editor
running on your PC.

This is the list of meta (Win) keys recognized by FRT when running on
the console (i.e. non on X11):

| Meta  | Action |
| :---: | :--- |
| *Q* | *Quit* |
| F | Toggle fullscreen |
| W | Change window gravity |
| K | Grab / ungrab the keyboard |
| M | Grab / ungrab the mouse |
| Return | Mouse Left Button (virtual mouse) |
| Cursor Keys | Mouse Motion (virtual mouse) |

The virtual mouse keys are available only if a keyboard is found, but a
mouse is not.

## More Info

- [Usage](doc/Usage.md)
- [Extensions](doc/Extensions.md)
- [Compile](doc/Compile.md)
- [Porting](doc/Porting.md)
