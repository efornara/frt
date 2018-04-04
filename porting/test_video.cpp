// test_video.cpp
/*
 * FRT - A Godot platform targeting single board computers
 * Copyright (c) 2017-2018  Emanuele Fornara
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

#define FRT_MOCK_GODOT_GL_CONTEXT
#include "frt.h"

#include <math.h>
#include <assert.h>

#include "dl/gles2.gen.h"

#include "bits/frt_app_impl.h"

using namespace frt;

App *app;
Video *video;
ContextGL *gl;
int y;
bool repeat = false;

#ifdef __unix__

#include <ctype.h>
#include <unistd.h>
#include <sys/select.h>

static void dispatch_meta() {
	struct timeval tv = { 0, 0 };
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);
	if (select(1, &fds, NULL, NULL, &tv) == 1) {
		char buf[64];
		int r = read(STDIN_FILENO, buf, sizeof(buf));
		for (int i = 0; i < r; i++) {
			int c = buf[i];
			if (c == '1') {
				repeat = true;
			} else if (c == '0') {
				repeat = false;
			} else if (isalpha(c)) {
				video->handle_meta(toupper(c), true);
				video->handle_meta(toupper(c), false);
			}
		}
	}
}

#else
static void dispatch_meta() {
}
static void usleep(int us) {
}
#endif

void iteration(bool vsync) {
	const int position_period = 300;
	const int visibility_period = 100;
	gl->set_use_vsync(vsync);
	for (int i = 0; i < position_period; i++, y++) {
		video->move_pointer(Vec2(y, y));
		if (vsync)
			glClearColor(0., 0., sin((M_PI * i) / position_period), 1.);
		else
			glClearColor(sin((M_PI * i) / position_period), 0., 0., 1.);
		glClear(GL_COLOR_BUFFER_BIT);
		switch (i % visibility_period) {
			case 0:
				video->show_pointer(true);
				break;
			case visibility_period / 2:
				video->show_pointer(false);
				break;
			default:
				break;
		}
		app->dispatch_events();
		gl->swap_buffers();
		dispatch_meta();
		if (!vsync)
			usleep(2000); // too fast otherwise
	}
}

int main(int argc, char *argv[]) {
	app = App::instance();
	video = (Video *)app->probe_single();
	assert(video);
	Vec2 size(640, 480);
	gl = video->create_the_gl_context(2, size);
	assert(gl);
	gl->initialize();
	gl->make_current();
	do {
		y = 0;
		iteration(true);
		iteration(false);
	} while (repeat);
	video->cleanup();
}
