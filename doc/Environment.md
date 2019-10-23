Environment
===========

Environment variables can be passed to FRT like this:

    export FRT_KMSDRM_DEVICE=/dev/dri/renderD128
	frt_096_216_pi2.bin -path ~/mygame

Or like this:

    FRT_KMSDRM_DEVICE=/dev/dri/renderD128 frt_096_216_pi2.bin -path ~/mygame

In most cases, you shouldn't need to set environment variables to use FRT.

## `FRT_KEYBOARD_ID` and `FRT_MOUSE_ID`

If FRT fails to select the correct device for your keyboard and/or your mouse,
you can force it to by specifying either the full path of the device, or its
name as seen by the input bus. For example, to find out which name to use, you
can use the following command:

    grep Name= /proc/bus/input/devices

`FRT_KEYBOARD_ID` can also be used to let FRT open a "virtual keyboard". For
example (GPIOnext):

    FRT_KEYBOARD_ID="GPIOnext Keyboard"

or (Adafruit-Retrogame):

    FRT_KEYBOARD_ID=retrogame

These variables are not relevant under X11.

## `FRT_KMSDRM_DEVICE`

Selection of the dri card to use is done by an heuristic tuned for the
Pis. For other devices, you might have to select it manually. For
example:

    FRT_KMSDRM_DEVICE=/dev/dri/renderD128

## `FRT_MODULES`

This should be used as a last resort and only if you know what you are doing.
Forcing a module that cannot run is likely to result in FRT just crashing.

To override which modules are loaded by FRT, use the following pattern:

    FRT_MODULES=<video>,<keyboard>,<mouse>

For example, this forces FRT to use the legacy fbdev module (Mali binary
driver for FBDEV):

    FRT_MODULES=video_fbdev,keyboard_linux_input,mouse_linux_input

And this prevents FRT from grabbing/using the keyboard and the mouse:

    FRT_MODULES=video_kmsdrm,,

