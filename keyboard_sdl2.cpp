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
#include "sdl2.h"

#include "import/gdkeys.h"

namespace frt {

class KeyboardSDL2 : public Keyboard {
private:
	SDL2User *sdl2;
	Handler *h;
	InputModifierState st;

public:
	KeyboardSDL2() : sdl2(0), h(0) {
		st.shift = false;
		st.alt = false;
		st.control = false;
		st.meta = false;
	}
	// Module
	const char *get_id() const { return "keyboard_sdl2"; }
	bool probe() {
		if (!sdl2)
			sdl2 = SDL2Context::acquire(0);
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
	bool poll() {
		sdl2->poll(0);
		return false;
	}
	void get_modifier_state(InputModifierState &state) const { state = st; }
};

FRT_REGISTER(KeyboardSDL2)

} // namespace frt
