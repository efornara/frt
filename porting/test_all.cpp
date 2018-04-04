// test_all.cpp
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

#define FRT_MOCK_GODOT_INPUT_MODIFIER_STATE
#define FRT_MOCK_GODOT_GL_CONTEXT
#include "frt.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "dl/gles2.gen.h"

#include "bits/frt_app_impl.h"

using namespace frt;

App *app;
Env env;
ContextGL *gl;

static struct Handler : public Keyboard::Handler, public Mouse::Handler {
	void handle_keyboard_key(int gd_code, bool pressed, uint32_t unicode, bool echo) {
		InputModifierState st;
		env.keyboard->get_modifier_state(st);
		printf("%c%c%c%c '%c' %010x %10d %08X %c %-10s\n",
			   st.shift ? 'S' : '-',
			   st.alt ? 'A' : '-',
			   st.control ? 'C' : '-',
			   st.meta ? 'M' : '-',
			   gd_code < 128 && isprint(gd_code) ? gd_code : '.', gd_code,
			   gd_code,
			   unicode,
			   echo ? '+' : '.',
			   pressed ? "pressed" : "released");
		if (st.meta && gd_code == 'Q')
			app->quit();
	}
	const char *button2string(Mouse::Button button) {
		switch (button) {
			case Mouse::Left:
				return "left";
			case Mouse::Middle:
				return "middle";
			case Mouse::Right:
				return "right";
			case Mouse::WheelUp:
				return "wheel_up";
			case Mouse::WheelDown:
				return "wheel_down";
			default:
				assert(0);
		}
	}
	void handle_mouse_button(Mouse::Button button, bool pressed) {
		Vec2 pos = env.mouse->get_pos();
		printf("%-10s %-10s (%d,%d)\n", button2string(button),
			   pressed ? "pressed" : "released", pos.x, pos.y);
	}
	void handle_mouse_motion(Vec2 pos) {
		printf("%4d %4d motion\n", pos.x, pos.y);
	}
} handler;

int main(int argc, char *argv[]) {
	app = App::instance();
	EnvProbe *env_probe = (EnvProbe *)app->probe("envprobe");
	assert(env_probe);
	env_probe->probe_env(&env);
	assert(env.video);
	assert(env.keyboard);
	assert(env.mouse);
	env.keyboard->set_handler(&handler);
	env.mouse->set_handler(&handler);
	Vec2 size(640, 480);
	gl = env.video->create_the_gl_context(2, size);
	assert(gl);
	gl->initialize();
	gl->make_current();
	glClearColor(0.0, 0.0, 0.5, 1.0);
	while (app->is_running()) {
		glClear(GL_COLOR_BUFFER_BIT);
		app->dispatch_events();
		gl->swap_buffers();
	}
	env.mouse->cleanup();
	env.keyboard->cleanup();
	env.video->cleanup();
}
