// envprobe.cpp
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
#include <errno.h>

#include <unistd.h>
#include <dlfcn.h>

using namespace frt;

#define FRT_ENV_ERROR 0
#define FRT_ENV_BCM 1
#define FRT_ENV_X11 2
#define FRT_ENV_FBDEV 3
#define FRT_ENV_VC4_NOX11 4

static bool bcm_installed() {
#if defined(__arm__) || defined(__aarch64__)
	return access("/opt/vc/lib/libbrcmEGL.so", R_OK) == 0;
#else
	return false;
#endif
}

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
	if (!(lib = dlopen("libX11.so.6", RTLD_LAZY)))
		return false;
	typedef void *(*FN_XOpenDisplay)(const char *);
	typedef int (*FN_XCloseDisplay)(void *);
	FN_XOpenDisplay fn_XOpenDisplay = (FN_XOpenDisplay)dlsym(lib, "XOpenDisplay");
	FN_XCloseDisplay fn_XCloseDisplay = (FN_XCloseDisplay)dlsym(lib, "XCloseDisplay");
	void *display = fn_XOpenDisplay(NULL);
	if (display)
		fn_XCloseDisplay(display);
	dlclose(lib);
	return (bool)display;
}

static int probe_environment() {
	if (bcm_installed()) {
		if (has_vc4()) {
			if (has_x11())
				return FRT_ENV_X11;
			else
				return FRT_ENV_VC4_NOX11;
		}
		return FRT_ENV_BCM;
	} else {
		if (has_x11())
			return FRT_ENV_X11;
		else
			return FRT_ENV_FBDEV;
	}
}

namespace frt {

class EnvProbeImpl : public EnvProbe {
public:
	// Module
	const char *get_id() const { return "envprobe"; }
	bool probe() { return true; }
	void cleanup() {}
	// EnvProbe
	void probe_env(Env *env) {
		App *app = App::instance();
		switch (probe_environment()) {
			case FRT_ENV_BCM:
				env->video = (Video *)app->probe("video_bcm");
				env->keyboard = (Keyboard *)app->probe("keyboard_linux_input");
				env->mouse = (Mouse *)app->probe("mouse_linux_input");
				break;
			case FRT_ENV_X11:
				env->video = (Video *)app->probe("video_x11");
				env->keyboard = (Keyboard *)app->probe("keyboard_x11");
				env->mouse = (Mouse *)app->probe("mouse_x11");
				break;
			case FRT_ENV_FBDEV:
				env->video = (Video *)app->probe("video_fbdev");
				env->keyboard = (Keyboard *)app->probe("keyboard_linux_input");
				env->mouse = (Mouse *)app->probe("mouse_linux_input");
				break;
			case FRT_ENV_VC4_NOX11:
				printf("frt: vc4 driver requires X11.\n");
				exit(1);
		}
	}
};

FRT_REGISTER(EnvProbeImpl)

} // namespace frt
