// gles2_egl.h
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2023  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "dl/gles2.gen.h"

// Hack to make the GLES2 driver happy and prevent it from including EGL.
// No dl/egl.gen.h because indirectly including X11 causes name clashes.
#define __egl_h_
#define __eglext_h_
#define eglGetProcAddress(x) (0)
