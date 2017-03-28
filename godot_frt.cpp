// godot_frt.cpp
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

#include "main/main.h"

#include "frt.h"

#define FRT_IMPLEMENT_REGISTRY
#include "bits/frt_registry_impl.h"

using namespace frt;

static Environment env;
static Runnable *runnable;

static void probe_modules() {
	Registry *frt = Registry::instance();
	Module *module;

	const char *video_modules[] = {
		"video_bcm",
		"video_x11",
		0
	};
	module = frt->probe(video_modules);
	env.video = (Video *)module;

	const char *keyboard_modules[] = {
		"keyboard_linux_input",
		0
	};
	module = frt->probe(keyboard_modules);
	env.keyboard = (Keyboard *)module;

	const char *mouse_modules[] = {
		"mouse_linux_input",
		0
	};
	module = frt->probe(mouse_modules);
	env.mouse = (Mouse *)module;

	const char *runnable_modules[] = {
		"frt_os_unix",
		0
	};
	module = frt->probe(runnable_modules);
	runnable = (Runnable *)module;
}

static void cleanup_modules() {
	if (env.mouse)
		env.mouse->cleanup();
	if (env.keyboard)
		env.keyboard->cleanup();
	if (env.video)
		env.video->cleanup();
	if (runnable)
		runnable->cleanup();
}

int main(int argc, char *argv[]) {
	int ret_code = 255;
	bool started = false;
	probe_modules();
	if (!runnable)
		goto quit_with_ret_code;
	runnable->setup_env(&env);
	if (Main::setup(argv[0], argc - 1, &argv[1]) != OK)
		goto quit_with_ret_code;
	if (!Main::start())
		goto quit;
	started = true;
	runnable->run_();
quit:
	if (started)
		Main::cleanup();
	ret_code = runnable->get_exit_code_();
quit_with_ret_code:
	cleanup_modules();
	return ret_code;
}
