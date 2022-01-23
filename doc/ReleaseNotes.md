Release Notes
=============

## FRT 1.1.0

The code itself is unchanged compared to FRT 1.0.0. However:

- Backported cross compilation support from the 2.0 branch.

- New architecture tags (arm32v6, arm32v7 and arm64v8) replace the old
  ones (pi1, pi2 and arm64).

- The official binaries are now compiled on buster instead of jessie.
  For Godot 2, clang is used, and for Godot 3, gcc is used. arm32v6
  builds are compiled on an arm device (Pi 3B+), and arm32v7/arm64v8
  builds are cross-compiled using a docker container on a PC.
  See efornara/crossbuild.

## FRT 1.0.0

- Released additional binaries that link against pulseaudio. Godot 3.3 is
  able to dynamically load alsa or pulseaudio, so a single binary without
  hard dependencies on either is provided starting with Godot 3.3.

- Not really about FRT, but the way Godot 3.3 allocates the shadow buffer
  has changed, and might cause problems on some low-end devices.
  If you are developing a 2D game, make sure to state your intent by
  setting Framebuffer Allocation to 2D in the Project Settings (Rendering /
  Quality / Intended Usage) and, if you use Viewports, by setting Usage
  to 2D in the their properties (Rendering).

- Added the `FRT_X11_UNDECORATED` environment variable, to work around the
  lack of fullscreen support in the x11 module.

- Added the `exit_on_shiftenter` parameter, to provide a universal way to
  terminate the process that matches the default key configuration in
  RetroPie.

- Fixed a Physics initialization bug.

