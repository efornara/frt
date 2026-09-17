// platform_config.h
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2026  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include <alloca.h>

#ifdef GLAD_ENABLED
#define GLES2_INCLUDE_H "thirdparty/glad/glad/glad.h"
#define GLES3_INCLUDE_H "thirdparty/glad/glad/glad.h"
#else
#define GLES2_INCLUDE_H "dl/gles2.gen.h"
#define GLES3_INCLUDE_H "dl/gles3.gen.h"
#endif
