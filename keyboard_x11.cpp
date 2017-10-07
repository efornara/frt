// keyboard_x11.cpp
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

#ifdef FRT_TEST
#define FRT_MOCK_GODOT_INPUT_MODIFIER_STATE
#else
#include "version.h"
#if VERSION_MAJOR == 3
#define FRT_MOCK_GODOT_INPUT_MODIFIER_STATE
#endif
#include "os/input_event.h"
#endif

#include "frt.h"
#include "bits/x11.h"

#include "import/gdkeys.h"

#ifndef FRT_MOCK_KEY_MAPPING_X11
#include "import/key_mapping_x11.h"
#endif

// TODO: unicode
// TODO: echo

namespace frt {

static const long handled_mask = KeyPressMask | KeyReleaseMask;

static const int handled_types[] = {
	KeyPress,
	KeyRelease,
	0,
};

class KeyboardX11 : public Keyboard, public EventHandler {
private:
	X11User *x11;
	Handler *h;
	InputModifierState st;

public:
	KeyboardX11()
		: x11(0), h(0) {
		st.shift = false;
		st.alt = false;
		st.control = false;
		st.meta = false;
	}
	// Module
	const char *get_id() const { return "keyboard_x11"; }
	bool probe() {
		if (!x11)
			x11 = X11Context::acquire(handled_mask, handled_types, this);
		return true;
	}
	void cleanup() {
		if (x11) {
			x11->release();
			x11 = 0;
		}
	}
	// Keyboard
	void set_handler(Handler *handler) {
		h = handler;
	}
	void get_modifier_state(InputModifierState &state) const { state = st; }
	// EventHandler
	void handle_event() {
		XEvent ev;
		x11->get_event(ev);
		st.shift = ev.xkey.state & ShiftMask;
		st.alt = ev.xkey.state & Mod1Mask;
		st.control = ev.xkey.state & ControlMask;
		st.meta = ev.xkey.state & Mod4Mask;
		KeySym keysym_keycode = 0;
		//KeySym keysym_unicode = 0;
		char str[256 + 1];
		XLookupString(&ev.xkey, str, 256, &keysym_keycode, NULL);
#ifndef FRT_MOCK_KEY_MAPPING_X11
		int keycode = KeyMappingX11::get_keycode(keysym_keycode);
#else
		int keycode = str[0];
		if (!keycode)
			return;
#endif
		if (keycode >= 'a' && keycode <= 'z')
			keycode -= 'a' - 'A';
		bool pressed = ev.type == KeyPress;
		if (h)
			h->handle_keyboard_key(keycode, pressed);
	}
};

FRT_REGISTER(KeyboardX11)

} // namespace frt
