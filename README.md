FRT
===

FRT is a Godot "platform" targeting single board computers.

## Usage (Raspberry Pi running raspbian)

FRT does not need X11 to run, and should also run on raspbian lite.

Make sure that you have increased the amount of memory for the GPU.
Some demos work with just the default 64M reserved for the GPU, but most
of them will need more.

Download, uncompress, install somewhere in your path one binary from
here:

[https://sourceforge.net/projects/frt/files](https://sourceforge.net/projects/frt/files)

	$ gunzip godot.frt.opt.pi1.gz
	$ sudo install godot.frt.opt.pi1 /usr/local/bin

Download and uncompress the "Demos and Examples" archive from
here:

[https://godotengine.org/download](https://godotengine.org/download)

Run a demo:

	$ cd Godot-Demos-2.1-20170121/2d/platformer
	$ godot.frt.opt.pi1 -v

Press *Win+Q* to quit.

This is the list of meta (Win) keys recognized by *this commit* of FRT.
The current binaries only support Win+Q.

| Meta  | Action |
| :---: | :--- |
| *Q* | *Quit* |
| K | Grab / ungrab the keyboard |
| M | Grab / ungrab the mouse |
| Return | Mouse Left Button (virtual mouse) |
| Cursor Keys | Mouse Motion (virtual mouse) |

The virtual mouse keys are available only if a keyboard is found, but a
mouse is not.

To compile FRT from source, see [Compile.md](doc/Compile.md).
