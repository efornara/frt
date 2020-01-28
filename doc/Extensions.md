Extensions
==========

FRT provides a few non-intrusive extensions compared to the official
godot platforms.  Note that some of them might be removed in the future
if the official godot starts to provide other means to address the same
issues.

## Command Line

When you invoke godot, arguments starting from the option `--frt` are
intercepted by FRT.  For example:

	godot.frt.opt.pi1 myscene.tscn --frt -h

will not start godot. Instead, a message like the following is printed
to stdout:

	usage: godot.frt.opt.pi1 [godot args] [--frt [options] [param=value...]]

	options:
	  -v                  show version and exit
	  -h                  show this page and exit
	  -p perfmon.csv      save performance monitor data
	  -e resource         extract resource

	params:
	  color_size
	  alpha_size
	  depth_size
	  multisample
	  disable_meta_keys
	  blacklist_video_bcm

The performance monitor data has been added because I have found that a
non-optimized build of godot/frt is much slower than an optimized build,
so letting the optimized build dump data to be analyzed later is a
better alternative than to try to connect to an editor from a
non-optimized build.

Resource extraction makes it easy to embed small files in single-file
games.  For example, if you export a standard 2.1.3 project as an EXE
using the Linux template, but giving the editor `godot.frt.opt.pi1` as a
custom binary, you end up with a single file, say `mygame` that you can
gzip and distribute.

Users should be able to just run the game, but they could also type:

	mygame --frt -e README.md

to extract a README file.

Only resources in the res://frt/ folder can be extracted this way, and
there are strict limits on the characters that you can use for the name
of the resource (the first character must be alphanumeric - the
following ones can also be '.', '-' and '\_').

## Parameters

You can add some options in the custom section "frt" of the project
settings.  The section is not required and is in any case ignored by the
standard godot platforms.

For example, adding this:

	[frt]

	depth_size = 24
	multisample = true

to `engine.cfg` will allow you to tune the engine.

You can also override or set the values from the command line.  For
example, if you have a third-party game that shows some artifacts that
you suspect might be due to godot/frt using a 16-bit depth buffer by
default, you can try running it like this:

	godot.frt.opt.pi1 --frt depth_size=24

### Disabling the meta keys

If you already provide a way to close the game (for example, using an
in-game menu), you might prefer to disable the handling of meta keys
by FRT.
Be careful that, since FRT grabs the keyboard when running without X11,
the user might not be able to quit the game in any other way.

### Blacklisting the bcm driver

The Godot 3.x rendering engine might have problems with some 3D games running
with the bcm video driver. In some cases, the Linux kernel could crash.
Unless you can test extensively, it is probably safer to blacklist the bcm
driver when releasing 3D games using Godot 3.x.
The game will run normally when running with other drivers, but will exit
suggesting to switch to the vc4 driver when running with the bcm driver.
