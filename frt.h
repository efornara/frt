// frt.h
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2022  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#define FRT_VERSION "2.0.1"

#if __cplusplus >= 201103L
#define FRT_OVERRIDE override
#else
#define FRT_OVERRIDE
#endif

namespace frt {

void warn(const char *format, ...);

#if __cplusplus >= 201103L
[[ noreturn ]]
#endif
void fatal(const char *format, ...);

} // namespace frt
