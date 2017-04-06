// mouse_sdl2.cpp
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
#include "bits/sdl2.h"

namespace frt {

static const SDL_EventType handled_types[] = {
	SDL_MOUSEMOTION,
	SDL_MOUSEBUTTONUP,
	SDL_MOUSEBUTTONDOWN,
	SDL_MOUSEWHEEL,
	SDL_LASTEVENT,
};

class MouseSDL2 : public Mouse {
private:
	SDL2User *sdl2;
	Handler *h;
	Vec2 pos;

public:
	MouseSDL2()
		: sdl2(0), h(0) {}
	// Module
	const char *get_id() const { return "mouse_sdl2"; }
	bool probe() {
		if (!sdl2)
			sdl2 = SDL2Context::acquire(handled_types);
		return true;
	}
	void cleanup() {
		if (sdl2) {
			sdl2->release();
			sdl2 = 0;
		}
	}
	// Mouse
	Vec2 get_pos() const { return pos; }
	void set_size(Vec2 size) {}
	void set_handler(Handler *handler) { h = handler; }
	bool poll() {
		SDL_Event ev;
		while (sdl2->poll(&ev)) {
			Button button;
			bool unhandled = false;
			switch (ev.type) {
				case SDL_MOUSEMOTION:
					pos.x = ev.motion.x;
					pos.y = ev.motion.y;
					if (h)
						h->handle_mouse_motion(pos);
					break;
				case SDL_MOUSEBUTTONUP:
				case SDL_MOUSEBUTTONDOWN:
					switch (ev.button.button) {
						case SDL_BUTTON_LEFT:
							button = Left;
							break;
						case SDL_BUTTON_RIGHT:
							button = Right;
							break;
						case SDL_BUTTON_MIDDLE:
							button = Middle;
							break;
						default:
							unhandled = true;
							break;
					}
					if (h && !unhandled)
						h->handle_mouse_button(button, ev.button.state == SDL_PRESSED);
					break;
				case SDL_MOUSEWHEEL:
					if (ev.wheel.y > 0)
						button = WheelUp;
					else if (ev.wheel.y < 0)
						button = WheelDown;
					else
						unhandled = true;
					if (h && !unhandled) {
						h->handle_mouse_button(button, true);
						h->handle_mouse_button(button, false); // TODO: needed?
					}
					break;
			}
		}
		return false;
	}
};

FRT_REGISTER(MouseSDL2)

} // namespace frt
