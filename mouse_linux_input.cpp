// mouse_linux_input.cpp
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
#include <string.h>
#include <assert.h>

#include "bits_linux_input.h"

namespace frt {

inline void clamp(int &v, const int max) {
	if (v < 0)
		v = 0;
	if (v >= max)
		v = max - 1;
}

class MouseLinuxInput : public Mouse, public LinuxInput {
private:
	static const int wait_ms = 100;
	Handler *h;
	Vec2 size;
	Vec2 pos;

public:
	// Module
	const char *get_id() const { return "mouse_linux_input"; }
	bool probe() { return open("event-mouse"); }
	void cleanup() { close(); }
	// LinuxInput
	bool handle(const input_event &ev) {
		if (ev.type == EV_REL) {
			if (ev.code == ABS_X) {
				pos.x += ev.value;
				clamp(pos.x, size.x);
			} else if (ev.code == ABS_Y) {
				pos.y += ev.value;
				clamp(pos.y, size.y);
			} else if (ev.code == ABS_WHEEL) {
				Button button = ev.value > 0 ? WheelUp : WheelDown;
				if (h) {
					h->handle_mouse_button(button, true);
					h->handle_mouse_button(button, false); // TODO: needed?
				}
				return false;
			}
			if (h)
				h->handle_mouse_motion(pos);
		} else if (ev.type == EV_KEY && (ev.value >= 0 || ev.value <= 2)) {
			const KeyValue value = (KeyValue)ev.value;
			if (value == KV_Repeated)
				return false;
			Button button;
			switch (ev.code) {
				case BTN_LEFT:
					button = Left;
					break;
				case BTN_RIGHT:
					button = Right;
					break;
				case BTN_MIDDLE:
					button = Middle;
					break;
				default:
					return false;
			}
			if (h)
				h->handle_mouse_button(button, value == KV_Pressed);
		}
		return false;
	}
	// Mouse
	Vec2 get_pos() const { return pos; }
	void set_size(Vec2 size) {
		this->size = size;
		pos.x = size.x - 1;
		pos.y = size.y - 1;
	}
	void set_handler(Handler *handler) {
		LinuxInput::grab(wait_ms);
		h = handler;
	}
	bool poll() { return LinuxInput::poll(); }
};

FRT_REGISTER(MouseLinuxInput)

} // namespace frt
