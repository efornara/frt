VC4
===

Used on Pis 0-3 and very well documented.

There are two drivers for it: an open-source one (vc4) and a closed-source,
legacy one (bcm).

## New Driver (vc4)

As of today, users have to enable it manually on Raspbian,

Usually, too slow to be used on Pis 0-1.

Very good integration with the Linux standard ecosystem, both X11 and KMS/DRM.

## Legacy Driver (bcm)

As of today, the default driver on Raspbian.

The only realistic option for Pis 0-1: it uses a coprocessor (other than the
main CPU and the main 3D core), and so it can offload some of the work from
the CPU.

No interaction with the X11 desktop.

## Known Limitations

- 3D on Godot 3.x only works with the vc4 driver.

- 2D particles on Godot 3.x don't work.
