// frt_exe.cc
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2023  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "frt_lib.h"

#include <string.h>

static void handle_frt_args(int *argc, char ***argv) {
	for (int i = 1; i < *argc; i++) {
		if (!strcmp((*argv)[i], "--frt")) {
			(*argv)[i] = (*argv)[0];
			int frt_argc = *argc - i;
			char **frt_argv = &(*argv)[i];
			frt_parse_frt_args(frt_argc, frt_argv);
			*argc = i;
			break;
		}
	}
}

int main(int argc, char *argv[]) {
	handle_frt_args(&argc, &argv);
	return frt_godot_main(argc, argv);
}
