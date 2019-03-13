// mouse_virtual.h
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

#include "../import/gdkeys.h"

namespace frt {

class MouseVirtual : public Mouse, public EventDispatcher {
private:
	static const int speed = 2;
	Handler *h;
	Vec2 size;
	Vec2 pos;
	struct {
		bool up;
		bool down;
		bool left;
		bool right;
	} st;

public:
	MouseVirtual()
		: h(0) {
		st.up = false;
		st.down = false;
		st.left = false;
		st.right = false;
	}
	// Module
	const char *get_id() const { return "mouse_virtual"; }
	bool probe() {
		App::instance()->add_dispatcher(this);
		return true;
	}
	void cleanup() {
		App::instance()->remove_dispatcher(this);
	}
	bool handle_meta(int gd_code, bool pressed) {
		switch (gd_code) {
			case GD_KEY_UP:
				st.up = pressed;
				break;
			case GD_KEY_DOWN:
				st.down = pressed;
				break;
			case GD_KEY_LEFT:
				st.left = pressed;
				break;
			case GD_KEY_RIGHT:
				st.right = pressed;
				break;
			case GD_KEY_RETURN:
				if (h)
					h->handle_mouse_button(Left, pressed);
				break;
			default:
				return false;
		}
		return true;
	}
	// EventDispatcher
	void dispatch_events() {
		if (!st.up && !st.down && !st.left && !st.right)
			return;
		if (st.up)
			pos.y -= speed;
		if (st.down)
			pos.y += speed;
		if (st.left)
			pos.x -= speed;
		if (st.right)
			pos.x += speed;
		if (h)
			h->handle_mouse_motion(pos);
	}
	// Mouse
	Vec2 get_pos() const { return pos; }
	void set_size(Vec2 size) {
		this->size = size;
		pos.x = size.x - 1;
		pos.y = size.y - 1;
	}
	void set_handler(Handler *handler) {
		h = handler;
	}
};

} // namespace frt
