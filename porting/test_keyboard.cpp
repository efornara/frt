// test_keyboard.cpp
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
#include "frt.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include <unistd.h>

#include "bits/frt_app_impl.h"

using namespace frt;

static struct KeyboardHandler : public Keyboard::Handler {
	Keyboard *keyboard;
	void handle_keyboard_key(int gd_code, bool pressed, uint32_t unicode, bool echo) {
		InputModifierState st;
		keyboard->get_modifier_state(st);
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
			exit(0);
	}
} handler;

int main(int argc, char *argv[]) {
	App *app = App::instance();
	Keyboard *keyboard = (Keyboard *)App::instance()->probe_single();
	assert(keyboard);
	handler.keyboard = keyboard;
	keyboard->set_handler(&handler);
	while (app->is_running()) {
		app->dispatch_events();
		usleep(20000);
	}
	keyboard->cleanup();
}
