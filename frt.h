// frt.h
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2025  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

namespace frt {

void warn(const char *format, ...)
#ifdef __GNUC__
__attribute__((format(printf, 1, 2)))
#endif
;

[[ noreturn ]] void fatal(const char *format, ...)
#ifdef __GNUC__
__attribute__((format(printf, 1, 2)))
#endif
;

} // namespace frt
