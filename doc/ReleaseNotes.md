Release Notes
=============

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

