Bugs
====

When reading the list of known bugs below, please keep this in mind:

- FRT has been written with the idea of treating SBCs as cheap and open game
  consoles, so some features you can find on the official Godot x11 platform,
  like cut and paste, drag and drop, or more in general proper desktop
  integration, are out of scope for FRT.

- Apart from the now deprecated 3.0-gles2 version, most of the code, and the
  rendering code in particular, is unpatched from the official Godot codebase.
  However, the code-path used is roughly the one used on mobile phones. If you
  find a bug, check upstream with similar bugs on phones with a similar SoC to
  the one used in your board.

- There are different code-paths within FRT, so it can behave differently on
  different hardware and even on different configurations of the same
  hardware.

- Major refactoring work in the FRT codebase is on hold until Godot 4.0
  reaches beta / release candidate stage.

## Common

- No proper fullscreen support

## Module: `video_bcm`

- Might panic the kernel if 3D is used in Godot 3.2

## Module: `video_kmsdms` and `video_fbdev`

- No mouse pointer

## Module: `keyboard_linux_input`

- Might provide the wrong key presses on some layouts
