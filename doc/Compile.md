Compile
=======

FRT is not really a patch, as it leaves the Godot source untouched. To
compile it, start from the official 2.1.4-stable source tarball:

[https://github.com/godotengine/godot/releases/tag/2.1.4-stable](https://github.com/godotengine/godot/releases/tag/2.1.4-stable)

Go to the platform directory and clone this repository:

	$ cd ~/godot-2.1.4-stable
	$ cd platform
	$ git clone https://github.com/efornara/frt

From the main directory, you now have a new "platform" available:

	$ cd ~/godot-2.1.4-stable
	$ scons platform=frt target=release frt_arch=pi3 -j 4

See below for a list of the needed packages.

When compiling on a real Raspberry Pi, using an external Hard Disk is
strongly advised: everything is faster and more reliable, and you can
enable a swap partition without stressing the Memory Card.
Swap is hardly used, especially if you start Raspbian without the GUI,
but enabling it allows you to use `-j 4` and leave the compilation
unattended.

Compiling godot-2.1.4-stable + FRT from scratch on a non-overclocked
Raspberry Pi 2 running Raspbian jessie should take about one hour.

On some Linux systems, FRT can run on a desktop. Intel IGP are known
to work. Compile it with:

	$ scons platform=frt target=release use_llvm=yes -j 4

The `use_llvm=yes` option is needed when compiling release /
release\_debug targets with Godot 2.1.x.
on a recent GCC (6+).

## Cross compile (QEMU)

These are a few notes on how to cross-compile FRT on a PC running debian
stable (stretch). A standard armv7 debian is used as a target. See
below on tips on using Raspbian as a target.

There might be better ways, and compilation is quite slow
(around 5 hours to compile godot-2.1.4-stable from scratch on a,
quite slow, Acer C7 C710-2847).

Familiarity with the Linux environment is assumed.
Notes based on 
[https://wiki.debian.org/EmDebian/CrossDebootstrap](https://wiki.debian.org/EmDebian/CrossDebootstrap).

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
	[chroot] # apt-get install -y build-essential scons pkg-config libx11-dev libgles2-mesa-dev libasound2-dev libfreetype6-dev libssl-dev libpng12-dev zlib1g-dev

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

## Godot 3.0 compilation status

[![Build Status](https://api.travis-ci.org/efornara/frt.svg?branch=master)](https://travis-ci.org/efornara/frt/builds)

At the moment only [3.0-gles2](https://github.com/efornara/godot/tree/3.0-gles2) is supported.

Note that you have to use `module_webm_enabled=no` to compile Godot 3 on
ARM targets.
