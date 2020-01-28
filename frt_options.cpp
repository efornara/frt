// frt_options.cpp
/*
 * FRT - A Godot platform targeting single board computers
 * Copyright (c) 2017-2019  Emanuele Fornara
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "frt.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace frt {

const char *perfmon_filename = 0;
const char *extract_resource_name = 0;

struct Param params[] = {
	Param("color_size", 8),
	Param("alpha_size", 8),
	Param("depth_size", 16),
	Param("multisample", false),
	Param("disable_meta_keys", false),
	Param("blacklist_video_bcm", false),
};
#define N_OF_PARAMS (sizeof(params) / sizeof(Param))

void show_param_list() {
	printf("params:\n");
	for (unsigned i = 0; i < N_OF_PARAMS; i++)
		printf("  %s\n", params[i].name);
	printf("\n");
}

void usage(const char *program_name, int code = 1) {
	printf("\n"
		   "usage: %s [godot args] [--frt [options] [param=value...]]\n"
		   "\n"
		   "options:\n"
		   "  -v                  show version and exit\n"
		   "  -h                  show this page and exit\n"
		   "  -p perfmon.csv      save performance monitor data\n"
		   "  -e resource         extract resource\n"
		   "\n",
		   program_name);
	show_param_list();
	exit(code);
}

void parse_frt_param(const char *name, const char *value) {
	Param *p = App::instance()->get_param(name);
	if (!p) {
		printf("frt: unknown param '%s'\n\n", name);
		show_param_list();
		exit(1);
	}
	Value &v = p->value;
	switch (v.t) {
		case Value::Bool:
			if (!strcmp(value, "true")) {
				v.u.b = true;
			} else if (!strcmp(value, "false")) {
				v.u.b = false;
			} else {
				printf("frt: invalid boolean for '%s' (true, false)\n", name);
				exit(1);
			}
			break;
		case Value::Int:
			v.u.i = atoi(value);
			break;
		case Value::Float:
			v.u.f = (float)atof(value);
			break;
		case Value::String:
			v.u.s = value;
			break;
	}
	p->source = Param::CommandLine;
}

void parse_frt_args(int argc, char **argv) {
	const char *program_name = argv[0];
	for (int i = 1; i < argc; i++) {
		const char *s = argv[i];
		if (!strcmp(s, "-v")) {
			printf("frt " FRT_VERSION "\n");
			exit(0);
		} else if (!strcmp(s, "-h")) {
			usage(program_name, 0);
		} else if (!strcmp(s, "-p")) {
			if (++i == argc)
				usage(program_name);
			perfmon_filename = argv[i];
		} else if (!strcmp(s, "-e")) {
			if (++i == argc)
				usage(program_name);
			extract_resource_name = argv[i];
		} else {
			char *sep = strchr(argv[i], '=');
			if (!sep)
				usage(program_name);
			*sep = '\0';
			parse_frt_param(argv[i], sep + 1);
		}
	}
}

int App::get_n_of_params() const {
	return N_OF_PARAMS;
}

Param *App::get_param(int i) const {
	return &params[i];
}

Param *App::get_param(const char *name) const {
	for (unsigned i = 0; i < N_OF_PARAMS; i++)
		if (!strcmp(name, params[i].name))
			return &params[i];
	return 0;
}

void App::parse_args(int *argc, char ***argv) {
	for (int i = 1; i < *argc; i++)
		if (!strcmp((*argv)[i], "--frt")) {
			(*argv)[i] = (*argv)[0];
			int frt_argc = *argc - i;
			char **frt_argv = &(*argv)[i];
			parse_frt_args(frt_argc, frt_argv);
			*argc = i;
			break;
		}
}

} // namespace frt
