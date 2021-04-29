FRT
===

[Godot](https://godotengine.org) is a full 2D and 3D game engine with editor.

FRT is a Godot "platform" targeting single board computers. In plain English,
you can export a Godot game to most of them by using FRT binaries, *as long as
the game has been designed with the limitation of the hardware in mind*.

## When (not) to use FRT

Godot comes with a generic X11 platform that works very well on Pis, as long
as you enable the experimental OpenGL driver (not needed on a Pi 4, not
recommended on Pi 0-1).

So, in general, if you are only interested in exporting games for Pis starting
from the Pi 2, your best option is probably to compile an ARM export template
yourself starting from the official Godot source code.

## How to use FRT

Precompiled versions of FRT can be found here:

<https://sourceforge.net/projects/frt/files/>

And instructions of how to export a game from the Godot editor running on your
PC [here](doc/Usage.md).

If you have come across an FRT game and it is running on the console, you can
press *Win+Q* to close it.

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

## User Info

- [ReleaseNotes](doc/ReleaseNotes.md)
- [Usage](doc/Usage.md)
- [Environment](doc/Environment.md)
- [Extensions](doc/Extensions.md)
- [Bugs](doc/Bugs.md)

## Hardware Info

- [Utgard](doc/Utgard.md)
- [VC4](doc/VC4.md)
- [VC6](doc/VC6.md)

## Developer Info

- [Compile](doc/Compile.md)
- [Porting](doc/Porting.md)
