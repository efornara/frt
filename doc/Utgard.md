Utgard
======

Utgard is an old architecture, but it is the architecture of very cheap and
popular boards.

There are two drivers for it: an open-source one (Lima) and a closed-source,
legacy one.

## Lima Driver

One way to install the Lima driver is to start from the armbian buster
"server" image appropriate to your board, install the -dev version of the
Linux kernel and compile the latest mesa yourself.

On my test A10-based board, I have to use the following environment variables:

    export FRT_KMSDRM_DEVICE=/dev/dri/renderD128
    export LD_LIBRARY_PATH=/usr/local/lib

Some 2.1 demos don't work (among others, IIRC, one of the two 3D materials and
the 2D isometric light one), but most 2D demos, and the 2D platformer in
particular, run well.

## Legacy Driver

If you ever wondered why the official FRT binaries are compiled on a jessie
debootstrap chroot, this driver is the reason.

It is a pain to use because it only supports old versions of the Linux kernel,
but it is full-featured. The Lima driver is already useable and it is the one
I would personally use, but for some use cases, the legacy driver is still the
way to go.

I recommend that you use a X11-based image.

In theory, you could start from a server image and compile a couple of
libraries to go along with the proprietary blob, but you are basically on your
own. FRT includes a fbdev video module, but it is more of starting point /
proof of concept than an useable module. It works but it flickers badly, and I
no longer maintain it. Anyway, if you have one of these boards, and you are up
for a challenge, patches are welcome.
