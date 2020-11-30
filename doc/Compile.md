Compile
=======

To compile the Godot templates on a Pi, start from the official
3.1-stable source tarball:

<https://github.com/godotengine/godot/releases/tag/3.1-stable>

These packages should be enough:

	$ sudo apt-get install build-essential scons pkg-config libx11-dev libgles2-mesa-dev libasound2-dev libfreetype6-dev libudev-dev libpng12-dev zlib1g-dev clang

## Compile Godot (X11)

To compile the official X11 platform, this is, in theory, what you should do:

	$ cd ~/godot-3.1-stable
	$ scons platform=x11 target=release tools=no -j 4

In practice, it is better to give the compiler some flags to better target
the CPU of the Pi, and especially its FPU.

For example (Pi 2/3):

	$ scons platform=x11 target=release tools=no CCFLAGS="-mcpu=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -mlittle-endian -munaligned-access" -j 4

Or (Pi 1/Zero):

	$ scons platform=x11 target=release tools=no CCFLAGS="-mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -mlittle-endian -munaligned-access" -j 4

You might need to use clang on some versions of debian. If so, add:

	use_llvm=yes

You might also need to disable some modules, for example:

	module_webm_enabled=no

See `release.sh` for the options that are used to generate the official
FRT release binaries, keeping in mind that they are generated on a
debian jessie chroot environment (gcc 4.9.2, so there is no need to use clang)
and that SSL is disabled (so you might want to omit the relevant options if
you want to enable it).

## Compile Godot (FRT)

FRT can be compiled against the Godot engine source as it is.

However there are some optional patches that you might want to apply
to the engine. See the `patches` directory. FRT release binaries are
generated applying the patches.

Go to the platform directory and clone this repository:

	$ cd ~/godot-3.1-stable
	$ cd platform
	$ git clone https://github.com/efornara/frt

From the main directory, you now have a new "platform" available:

	$ cd ~/godot-3.1-stable
	$ scons platform=frt target=release frt_arch=pi3 -j 4

Unlike the X11 platform, FRT disables tools by default and preselect
some CPU-specific options according to frt\_arch.

When compiling on a real Pi, using an external Hard Disk is
strongly advised: everything is faster and more reliable, and you can
enable a swap partition without stressing the Memory Card.
Swap is hardly used, especially if you start Raspbian without the GUI,
but enabling it allows you to use `-j 4` and leave the compilation
unattended.

Compiling godot-3.1-stable + FRT from scratch on a non-overclocked
Pi 3 running Raspbian jessie should take about one hour.

## Compile Godot (FRT) to run on a PC

On some Linux systems, FRT can run on a desktop. This can be useful
to test the ES 2.0 rendering pipeline directly on a PC.
Intel IGPs are known to work.
Just repeat the steps above from your Linux PC and build FRT without
setting the architecture:

	$ scons platform=frt target=release -j 8

## Cross compile (QEMU)

These are a few notes on how to cross-compile FRT on a PC running debian
stable (stretch). A standard armv7 debian is used as a target. See
below on tips on using Raspbian as a target.

There might be better ways, and compilation is quite slow
(around 5 hours to compile godot-2.1.4-stable from scratch on a,
quite slow, Acer C7 C710-2847).

The official FRT release binaries are not cross-compiled. They are generated
on a Pi.

Familiarity with the Linux environment is assumed.
Notes based on 
<https://wiki.debian.org/EmDebian/CrossDebootstrap>

Install debootstrap and qemu:

	[host] # apt-get install binfmt-support qemu qemu-user-static debootstrap

Download and install the base system:

	[host] # cd /opt
	[host] # mkdir arm7hf
	[host] # qemu-debootstrap --arch armhf jessie arm7hf http://deb.debian.org/debian

Mount the proc filesystem:

	[host] # mount -t proc proc /opt/arm7hf/proc

Chroot into the system and create a user (with uid/gid matching your main
user on the host; no action should be required if your main user is the
first user created):

	[host] # LC_ALL=C chroot /opt/arm7hf bash
	[chroot] # adduser user
	...

Install the needed packages:

	[chroot] # apt-get update
	[chroot] # apt-get upgrade -y
	[chroot] # apt-get install -y build-essential scons pkg-config libx11-dev libgles2-mesa-dev libasound2-dev libfreetype6-dev libudev-dev libpng12-dev zlib1g-dev

From another terminal, and using your main user on the host (note the `$`
prompt), clone the Godot and FRT repositories:

	[host] $ cd /opt/arm7hf/home/user
	[host] $ git clone https://github.com/efornara/godot 3.0-gles2
	[host] $ cd 3.0-gles2/platform
	[host] $ git clone https://github.com/efornara/frt

Back on the chroot environment, compile Godot:

	[chroot] # su - user
	[chroot] $ cd 3.0-gles2
	[chroot] $ nice scons platform=frt target=release frt_arch=arm7hf module_webm_enabled=no builtin_freetype=yes -j 2
	...

### Raspbian

- Download the Raspbian keyring and use it with qemu-debootstrap

- Check the chroot `/etc/apt/source.list` after debootstrap, and reset it
to the Raspbian repository if needed.

### BCM

- Download the raspberry firmware to compile BCM support. Only the
include directory (`/opt/vc/include`) is needed as the libraries are
dynamically loaded.
