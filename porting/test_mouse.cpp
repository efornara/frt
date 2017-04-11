// test_mouse.cpp
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
#include <assert.h>

#include <unistd.h>

#include "bits/frt_app_impl.h"

using namespace frt;

static struct MouseHandler : public Mouse::Handler {
	Mouse *mouse;
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
		Vec2 pos = mouse->get_pos();
		printf("%-10s %-10s (%d,%d)\n", button2string(button),
			   pressed ? "pressed" : "released", pos.x, pos.y);
	}
	void handle_mouse_motion(Vec2 pos) {
		printf("%4d %4d motion\n", pos.x, pos.y);
	}
} handler;

int main(int argc, char *argv[]) {
	App *app = App::instance();
	Mouse *mouse = (Mouse *)app->probe_single();
	assert(mouse);
	mouse->set_size(Vec2(640, 480));
	mouse->set_handler(&handler);
	handler.mouse = mouse;
	while (app->is_running()) {
		app->dispatch_events();
		usleep(20000);
	}
	mouse->cleanup();
}
