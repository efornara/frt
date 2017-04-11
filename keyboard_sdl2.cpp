// keyboard_sdl2.cpp
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
#include "os/input_event.h"
#endif

#include "frt.h"
#include "bits/sdl2.h"

#include "import/gdkeys.h"

namespace frt {

static struct KeyMap {
	int sdl2_code;
	int gd_code;
} keymap[] = {
	  { SDLK_SPACE, ' ' },
	  { SDLK_a, 'A' },
	  { SDLK_b, 'B' },
	  { SDLK_c, 'C' },
	  { SDLK_d, 'D' },
	  { SDLK_e, 'E' },
	  { SDLK_f, 'F' },
	  { SDLK_g, 'G' },
	  { SDLK_h, 'H' },
	  { SDLK_i, 'I' },
	  { SDLK_j, 'J' },
	  { SDLK_k, 'K' },
	  { SDLK_l, 'L' },
	  { SDLK_m, 'M' },
	  { SDLK_n, 'N' },
	  { SDLK_o, 'O' },
	  { SDLK_p, 'P' },
	  { SDLK_q, 'Q' },
	  { SDLK_r, 'R' },
	  { SDLK_s, 'S' },
	  { SDLK_t, 'T' },
	  { SDLK_u, 'U' },
	  { SDLK_v, 'V' },
	  { SDLK_w, 'W' },
	  { SDLK_x, 'X' },
	  { SDLK_y, 'Y' },
	  { SDLK_z, 'Z' },
	  { SDLK_0, '0' },
	  { SDLK_1, '1' },
	  { SDLK_2, '2' },
	  { SDLK_3, '3' },
	  { SDLK_4, '4' },
	  { SDLK_5, '5' },
	  { SDLK_6, '6' },
	  { SDLK_7, '7' },
	  { SDLK_8, '8' },
	  { SDLK_9, '9' },
	  { SDLK_F1, GD_KEY_F1 },
	  { SDLK_F2, GD_KEY_F2 },
	  { SDLK_F3, GD_KEY_F3 },
	  { SDLK_F4, GD_KEY_F4 },
	  { SDLK_F5, GD_KEY_F5 },
	  { SDLK_F6, GD_KEY_F6 },
	  { SDLK_F7, GD_KEY_F7 },
	  { SDLK_F8, GD_KEY_F8 },
	  { SDLK_F9, GD_KEY_F9 },
	  { SDLK_F10, GD_KEY_F10 },
	  { SDLK_F11, GD_KEY_F11 },
	  { SDLK_F12, GD_KEY_F12 },
	  { SDLK_UP, GD_KEY_UP },
	  { SDLK_DOWN, GD_KEY_DOWN },
	  { SDLK_LEFT, GD_KEY_LEFT },
	  { SDLK_RIGHT, GD_KEY_RIGHT },
	  { SDLK_TAB, GD_KEY_TAB },
	  { SDLK_BACKSPACE, GD_KEY_BACKSPACE },
	  { SDLK_INSERT, GD_KEY_INSERT },
	  { SDLK_DELETE, GD_KEY_DELETE },
	  { SDLK_HOME, GD_KEY_HOME },
	  { SDLK_END, GD_KEY_END },
	  { SDLK_PAGEUP, GD_KEY_PAGEUP },
	  { SDLK_PAGEDOWN, GD_KEY_PAGEDOWN },
	  { SDLK_RETURN, GD_KEY_RETURN },
	  { SDLK_ESCAPE, GD_KEY_ESCAPE },
	  { SDLK_LCTRL, GD_KEY_CONTROL },
	  { SDLK_RCTRL, GD_KEY_CONTROL },
	  { SDLK_LALT, GD_KEY_ALT },
	  { SDLK_RALT, GD_KEY_ALT },
	  { SDLK_LSHIFT, GD_KEY_SHIFT },
	  { SDLK_RSHIFT, GD_KEY_SHIFT },
	  { SDLK_LGUI, GD_KEY_META },
	  { SDLK_RGUI, GD_KEY_META },
	  { SDLK_KP_0, GD_KEY_KP_0 },
	  { SDLK_KP_1, GD_KEY_KP_1 },
	  { SDLK_KP_2, GD_KEY_KP_2 },
	  { SDLK_KP_3, GD_KEY_KP_3 },
	  { SDLK_KP_4, GD_KEY_KP_4 },
	  { SDLK_KP_5, GD_KEY_KP_5 },
	  { SDLK_KP_6, GD_KEY_KP_6 },
	  { SDLK_KP_7, GD_KEY_KP_7 },
	  { SDLK_KP_8, GD_KEY_KP_8 },
	  { SDLK_KP_9, GD_KEY_KP_9 },
	  { SDLK_KP_MULTIPLY, GD_KEY_KP_MULTIPLY },
	  { SDLK_KP_MINUS, GD_KEY_KP_SUBTRACT },
	  { SDLK_KP_PLUS, GD_KEY_KP_ADD },
	  { SDLK_KP_PERIOD, GD_KEY_KP_PERIOD },
	  { SDLK_KP_ENTER, GD_KEY_KP_ENTER },
	  { SDLK_KP_DIVIDE, GD_KEY_KP_DIVIDE },
	  { 0, 0 },
  };

static const SDL_EventType handled_types[] = {
	SDL_KEYUP,
	SDL_KEYDOWN,
	SDL_LASTEVENT,
};

class KeyboardSDL2 : public Keyboard, public EventHandler {
private:
	SDL2User *sdl2;
	Handler *h;
	InputModifierState st;

public:
	KeyboardSDL2()
		: sdl2(0), h(0) {
		st.shift = false;
		st.alt = false;
		st.control = false;
		st.meta = false;
	}
	// Module
	const char *get_id() const { return "keyboard_sdl2"; }
	bool probe() {
		if (!sdl2)
			sdl2 = SDL2Context::acquire(handled_types, this);
		return true;
	}
	void cleanup() {
		if (sdl2) {
			sdl2->release();
			sdl2 = 0;
		}
	}
	// Keyboard
	void set_handler(Handler *handler) {
		h = handler;
	}
	void get_modifier_state(InputModifierState &state) const { state = st; }
	// EventHandler
	void handle_event() {
		SDL_Event ev;
		sdl2->get_event(ev);
		st.shift = ev.key.keysym.mod & KMOD_SHIFT;
		st.alt = ev.key.keysym.mod & KMOD_ALT;
		st.control = ev.key.keysym.mod & KMOD_CTRL;
		st.meta = ev.key.keysym.mod & KMOD_GUI;
		if (ev.key.repeat)
			return;
		bool pressed = ev.key.state == SDL_PRESSED;
		for (int i = 0; keymap[i].sdl2_code; i++) {
			if (keymap[i].sdl2_code == ev.key.keysym.sym) {
				if (h)
					h->handle_keyboard_key(keymap[i].gd_code, pressed);
				return;
			}
		}
	}
};

FRT_REGISTER(KeyboardSDL2)

} // namespace frt
