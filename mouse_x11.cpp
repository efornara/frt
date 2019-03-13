// mouse_x11.cpp
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
#include "bits/x11.h"

namespace frt {

static const long handled_mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | Button1MotionMask | Button2MotionMask | Button3MotionMask | Button4MotionMask | Button5MotionMask | ButtonMotionMask;

static const int handled_types[] = {
	MotionNotify,
	ButtonPress,
	ButtonRelease,
	0,
};

class MouseX11 : public Mouse, public EventHandler {
private:
	X11User *x11;
	Handler *h;
	Vec2 pos;

public:
	MouseX11()
		: x11(0), h(0) {}
	// Module
	const char *get_id() const { return "mouse_x11"; }
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
	// Mouse
	Vec2 get_pos() const { return pos; }
	void set_size(Vec2 size) {}
	void set_handler(Handler *handler) { h = handler; }
	// EventHandler
	void handle_event() {
		XEvent ev;
		x11->get_event(ev);
		Button button;
		bool unhandled = false;
		switch (ev.type) {
			case MotionNotify:
				pos.x = ev.xmotion.x;
				pos.y = ev.xmotion.y;
				if (h)
					h->handle_mouse_motion(pos);
				break;
			case ButtonPress:
			case ButtonRelease:
				switch (ev.xbutton.button) {
					case 1:
						button = Left;
						break;
					case 2:
						button = Middle;
						break;
					case 3:
						button = Right;
						break;
					case 4:
						button = WheelUp;
						break;
					case 5:
						button = WheelDown;
						break;
					default:
						unhandled = true;
						break;
				}
				if (h && !unhandled)
					h->handle_mouse_button(button, ev.type == ButtonPress);
				break;
		}
	}
};

FRT_REGISTER(MouseX11)

} // namespace frt
