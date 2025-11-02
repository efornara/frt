// frt.cc
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2025  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "frt.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#define FRT_VERSION "3.6.2-1"

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

extern const char *license;

} // namespace frt

#include "frt_lib.h"

static void usage(const char *program_name, int code) {
	printf("\n"
		"usage: %s [godot args] [--frt [options]]\n"
		"\n"
		"options:\n"
		"  -v                  show version and exit\n"
		"  -l                  show license and exit\n"
		"  -h                  show this page and exit\n"
	"\n", program_name);
	exit(code);
}

extern "C" void frt_parse_frt_args(int argc, char *argv[]) {
	const char *program_name = argv[0];
	for (int i = 1; i < argc; i++) {
		const char *s = argv[i];
		if (!strcmp(s, "-v")) {
			printf("%s\n", FRT_VERSION);
			exit(0);
		} else if (!strcmp(s, "-l")) {
			puts(frt::license);
			exit(0);
		} else if (!strcmp(s, "-h")) {
			usage(program_name, 0);
		} else {
			usage(program_name, 1);
		}
	}
}
