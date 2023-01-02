/* frt_lib.h */
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2023  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#ifndef FRT_LIB_H
#define FRT_LIB_H

#ifdef __cplusplus
extern "C" {
#endif

void frt_parse_frt_args(int argc, char *argv[]);
int frt_godot_main(int argc, char *argv[]);

#ifdef __cplusplus
}
#endif

#endif /* FRT_LIB_H */
