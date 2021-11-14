// frt_utils.cc
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2021  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "frt_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

static void print_msg(const char *format, va_list ap) {
	fprintf(stderr, "frt: ");
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
}

namespace frt {

void warn(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	print_msg(format, ap);
	va_end(ap);
}

void fatal(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	print_msg(format, ap);
	va_end(ap);
	exit(1);
}

} // namespace frt
