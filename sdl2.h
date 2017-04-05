// sdl2.h
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

#include <stdlib.h>
#include <assert.h>

#include <SDL2/SDL.h>

/*
	WARNING: This is not a proper implementation!
	It is just a minimal version to help to test other modules on a typical
	UNIX setup.
 */

namespace frt {

class SDL2Context {
private:
	int users;
	bool create_window;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL2Context() : users(0), create_window(true), window(0), renderer(0) {
		SDL_Init(SDL_INIT_VIDEO);
	}
	~SDL2Context() {
		if (renderer)
			SDL_DestroyRenderer(renderer);
		if (window)
			SDL_DestroyWindow(window);
		SDL_Quit();
	}

public:
	void poll() {
		if (create_window && !window)
			SDL_CreateWindowAndRenderer(100, 100, 0, &window, &renderer);
		SDL_Event ev;
		while (SDL_PollEvent(&ev))
			if (ev.type == SDL_QUIT)
				exit(0);
		if (window && renderer) {
			SDL_RenderClear(renderer);
			SDL_RenderPresent(renderer);
		}
	}
	/*
		Unfortunately, no RAII since users are driven by probe/cleanup.
		Be careful: no safeguards here!
	 */
	static SDL2Context *acquire(bool window_owner = false) {
		Registry *registry = Registry::instance();
		SDL2Context **ctx = (SDL2Context **)registry->get_context("sdl2");
		if (!*ctx)
			*ctx = new SDL2Context();
		(*ctx)->users++;
		if (window_owner)
			(*ctx)->create_window = false;
		return *ctx;
	}
	void release() {
		if (--users == 0) {
			delete this;
			Registry *registry = Registry::instance();
			SDL2Context **ctx = (SDL2Context **)registry->get_context("sdl2");
			*ctx = 0;
		}
	}
};

} // namespace frt
