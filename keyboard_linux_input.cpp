// keyboard_linux_input.cpp
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

#ifdef FRT_TEST
#define FRT_MOCK_GODOT_INPUT_MODIFIER_STATE
#else
#include "core/version.h"
#if VERSION_MAJOR == 3
#define FRT_MOCK_GODOT_INPUT_MODIFIER_STATE
#endif
#include "core/os/input_event.h"
#endif

#include "frt.h"

#include <stdio.h>
#include <string.h>

#include "bits/linux_input.h"

#include "import/gdkeys.h"

namespace frt {

static struct KeyMap {
	int kernel_code;
	int gd_code;
} keymap[] = {
	  { KEY_SPACE, ' ' },
	  { KEY_A, 'A' },
	  { KEY_B, 'B' },
	  { KEY_C, 'C' },
	  { KEY_D, 'D' },
	  { KEY_E, 'E' },
	  { KEY_F, 'F' },
	  { KEY_G, 'G' },
	  { KEY_H, 'H' },
	  { KEY_I, 'I' },
	  { KEY_J, 'J' },
	  { KEY_K, 'K' },
	  { KEY_L, 'L' },
	  { KEY_M, 'M' },
	  { KEY_N, 'N' },
	  { KEY_O, 'O' },
	  { KEY_P, 'P' },
	  { KEY_Q, 'Q' },
	  { KEY_R, 'R' },
	  { KEY_S, 'S' },
	  { KEY_T, 'T' },
	  { KEY_U, 'U' },
	  { KEY_V, 'V' },
	  { KEY_W, 'W' },
	  { KEY_X, 'X' },
	  { KEY_Y, 'Y' },
	  { KEY_Z, 'Z' },
	  { KEY_0, '0' },
	  { KEY_1, '1' },
	  { KEY_2, '2' },
	  { KEY_3, '3' },
	  { KEY_4, '4' },
	  { KEY_5, '5' },
	  { KEY_6, '6' },
	  { KEY_7, '7' },
	  { KEY_8, '8' },
	  { KEY_9, '9' },
	  { KEY_F1, GD_KEY_F1 },
	  { KEY_F2, GD_KEY_F2 },
	  { KEY_F3, GD_KEY_F3 },
	  { KEY_F4, GD_KEY_F4 },
	  { KEY_F5, GD_KEY_F5 },
	  { KEY_F6, GD_KEY_F6 },
	  { KEY_F7, GD_KEY_F7 },
	  { KEY_F8, GD_KEY_F8 },
	  { KEY_F9, GD_KEY_F9 },
	  { KEY_F10, GD_KEY_F10 },
	  { KEY_F11, GD_KEY_F11 },
	  { KEY_F12, GD_KEY_F12 },
	  { KEY_UP, GD_KEY_UP },
	  { KEY_DOWN, GD_KEY_DOWN },
	  { KEY_LEFT, GD_KEY_LEFT },
	  { KEY_RIGHT, GD_KEY_RIGHT },
	  { KEY_TAB, GD_KEY_TAB },
	  { KEY_BACKSPACE, GD_KEY_BACKSPACE },
	  { KEY_INSERT, GD_KEY_INSERT },
	  { KEY_DELETE, GD_KEY_DELETE },
	  { KEY_HOME, GD_KEY_HOME },
	  { KEY_END, GD_KEY_END },
	  { KEY_PAGEUP, GD_KEY_PAGEUP },
	  { KEY_PAGEDOWN, GD_KEY_PAGEDOWN },
	  { KEY_ENTER, GD_KEY_RETURN },
	  { KEY_ESC, GD_KEY_ESCAPE },
	  { KEY_LEFTCTRL, GD_KEY_CONTROL },
	  { KEY_RIGHTCTRL, GD_KEY_CONTROL },
	  { KEY_LEFTALT, GD_KEY_ALT },
	  { KEY_RIGHTALT, GD_KEY_ALT },
	  { KEY_LEFTSHIFT, GD_KEY_SHIFT },
	  { KEY_RIGHTSHIFT, GD_KEY_SHIFT },
	  { KEY_LEFTMETA, GD_KEY_META },
	  { KEY_RIGHTMETA, GD_KEY_META },
	  { KEY_KP0, GD_KEY_KP_0 },
	  { KEY_KP1, GD_KEY_KP_1 },
	  { KEY_KP2, GD_KEY_KP_2 },
	  { KEY_KP3, GD_KEY_KP_3 },
	  { KEY_KP4, GD_KEY_KP_4 },
	  { KEY_KP5, GD_KEY_KP_5 },
	  { KEY_KP6, GD_KEY_KP_6 },
	  { KEY_KP7, GD_KEY_KP_7 },
	  { KEY_KP8, GD_KEY_KP_8 },
	  { KEY_KP9, GD_KEY_KP_9 },
	  { KEY_KPASTERISK, GD_KEY_KP_MULTIPLY },
	  { KEY_KPMINUS, GD_KEY_KP_SUBTRACT },
	  { KEY_KPPLUS, GD_KEY_KP_ADD },
	  { KEY_KPDOT, GD_KEY_KP_PERIOD },
	  { KEY_KPENTER, GD_KEY_KP_ENTER },
	  { KEY_KPSLASH, GD_KEY_KP_DIVIDE },
	  { 0, 0 },
  };

class KeyboardLinuxInput : public Keyboard, public EventDispatcher, public LinuxInput {
private:
	static const int left_mask = 0x01;
	static const int right_mask = 0x02;
	static const int wait_ms = 100;
	bool valid;
	Handler *h;
	bool grabbed;
	InputModifierState st;
	struct {
		int shift;
		int alt;
		int control;
		int meta;
	} left_right_mask;
	void update_modifier(int in_code, int left_code, int right_code, bool pressed, int &mask, bool &state) {
		if (in_code != left_code && in_code != right_code)
			return;
		if (in_code == left_code) {
			if (pressed)
				mask |= left_mask;
			else
				mask &= ~left_mask;
		} else if (in_code == right_code) {
			if (pressed)
				mask |= right_mask;
			else
				mask &= ~right_mask;
		}
		state = mask;
	}

public:
	KeyboardLinuxInput()
		: valid(false), h(0), grabbed(false) {
		st.shift = false;
		st.alt = false;
		st.control = false;
		st.meta = false;
		left_right_mask.shift = 0;
		left_right_mask.alt = 0;
		left_right_mask.control = 0;
		left_right_mask.meta = 0;
	}
	// Module
	const char *get_id() const { return "keyboard_linux_input"; }
	bool probe() {
		valid = open("event-kbd");
		if (valid)
			App::instance()->add_dispatcher(this);
		return valid;
	}
	void cleanup() {
		close();
		if (valid)
			App::instance()->remove_dispatcher(this);
		valid = false;
	}
	bool handle_meta(int gd_code, bool pressed) {
		if (pressed)
			return false;
		switch (gd_code) {
			case 'K':
				if (LinuxInput::grab(!grabbed, wait_ms))
					grabbed = !grabbed;
				return true;
			default:
				return false;
		}
	}
	// LinuxInput
	void handle(const input_event &ev) {
		if (ev.type != EV_KEY || (ev.value != KV_Pressed && ev.value != KV_Released))
			return;
		bool pressed = (ev.value == KV_Pressed);
		update_modifier(ev.code, KEY_LEFTSHIFT, KEY_RIGHTSHIFT, pressed, left_right_mask.shift, st.shift);
		update_modifier(ev.code, KEY_LEFTALT, KEY_RIGHTALT, pressed, left_right_mask.alt, st.alt);
		update_modifier(ev.code, KEY_LEFTCTRL, KEY_RIGHTCTRL, pressed, left_right_mask.control, st.control);
		update_modifier(ev.code, KEY_LEFTMETA, KEY_RIGHTMETA, pressed, left_right_mask.meta, st.meta);
		for (int i = 0; keymap[i].kernel_code; i++) {
			if (keymap[i].kernel_code == ev.code) {
				if (h)
					h->handle_keyboard_key(keymap[i].gd_code, pressed, 0, false);
				break;
			}
		}
	}
	// EventDispatcher
	void dispatch_events() { poll(); }
	// Keyboard
	void set_handler(Handler *handler) {
		h = handler;
		grabbed = LinuxInput::grab(true, wait_ms);
	}
	void get_modifier_state(InputModifierState &state) const { state = st; }
};

FRT_REGISTER(KeyboardLinuxInput)

} // namespace frt
