// frt_godot3.cc
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2021  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "frt.h"
#include "sdl2_adapter.h"
#include "sdl2_godot_mapping.h"

#include "frt_lib.h"

extern "C" int frt_godot_main(int argc, char *argv[]) {
	frt::fatal("Hello, World!");
}
