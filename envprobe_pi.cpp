// envprobe_pi.cpp
/*
 * FRT - A Godot platform targeting single board computers
 * Copyright (c) 2017  Emanuele Fornara
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
#include <errno.h>

#include <dlfcn.h>

#include "dl/x11.gen.h"

using namespace frt;

#define FRT_PI_ENV_ERROR 0
#define FRT_PI_ENV_BCM 1
#define FRT_PI_ENV_BCM_X11 2
#define FRT_PI_ENV_VC4 3
#define FRT_PI_ENV_VC4_X11 4

static bool has_vc4() {
	FILE *f = fopen("/proc/modules", "r");
	if (!f)
		return false;
	char s[1024];
	bool found = false;
	while (fgets(s, sizeof(s), f)) {
		if (!strncmp("vc4 ", s, 4)) {
			found = true;
			break;
		}
	}
	fclose(f);
	return found;
}

static bool has_x11() {
	void *lib = 0;
	if (!(lib = dlopen("libX11.so", RTLD_LAZY)))
		return false;
	frt_fn_XOpenDisplay = (FRT_FN_XOpenDisplay)dlsym(lib, "XOpenDisplay");
	frt_fn_XCloseDisplay = (FRT_FN_XCloseDisplay)dlsym(lib, "XCloseDisplay");
	Display *display = XOpenDisplay(NULL);
	if (display)
		XCloseDisplay(display);
	dlclose(lib);
	return (bool)display;
}

static int probe_pi() {
	if (has_vc4()) {
		if (has_x11())
			return FRT_PI_ENV_VC4_X11;
		else
			return FRT_PI_ENV_VC4;
	}
	return FRT_PI_ENV_BCM;
}

namespace frt {

class EnvProbePi : public EnvProbe {
public:
	// Module
	const char *get_id() const { return "envprobe"; }
	bool probe() { return true; }
	void cleanup() {}
	// EnvProbe
	void probe_env(Env *env) {
		App *app = App::instance();
		switch (probe_pi()) {
			case FRT_PI_ENV_BCM:
				env->video = (Video *)app->probe("video_bcm");
				env->keyboard = (Keyboard *)app->probe("keyboard_linux_input");
				env->mouse = (Mouse *)app->probe("mouse_linux_input");
				break;
			case FRT_PI_ENV_BCM_X11:
				printf("frt: bcm/x11 integration not implemented.\n");
				exit(1);
			case FRT_PI_ENV_VC4:
				printf("frt: vc4 driver requires.\n");
				exit(1);
			case FRT_PI_ENV_VC4_X11:
				env->video = (Video *)app->probe("video_x11");
				env->keyboard = (Keyboard *)app->probe("keyboard_x11");
				env->mouse = (Mouse *)app->probe("mouse_x11");
				break;
		}
	}
};

FRT_REGISTER(EnvProbePi)

} // namespace frt
