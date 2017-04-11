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

class SDL2Context;

struct SDL2User {
private:
	friend SDL2Context;
	SDL2Context *ctx;
	EventHandler *h;
	const SDL_EventType *types;
	bool valid;
	SDL2User()
		: valid(false) {}

public:
	inline void get_event(SDL_Event &ev);
	inline void release();
};

class SDL2Context : public EventDispatcher {
private:
	friend SDL2User;
	static const int max_users = 10;
	SDL2User users[max_users];
	int n_users;
	bool create_window;
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Event ev;
	SDL2Context()
		: n_users(0), create_window(true), window(0), renderer(0) {
		SDL_Init(SDL_INIT_VIDEO);
	}
	virtual ~SDL2Context() {
		if (renderer)
			SDL_DestroyRenderer(renderer);
		if (window)
			SDL_DestroyWindow(window);
		SDL_Quit();
	}
	SDL2User *acquire_(const SDL_EventType *types, EventHandler *handler, bool window_owner) {
		for (int i = 0; i < max_users; i++) {
			if (users[i].valid)
				continue;
			n_users++;
			users[i].valid = true;
			users[i].ctx = this;
			users[i].types = types;
			users[i].h = handler;
			if (window_owner)
				create_window = false;
			return &users[i];
		}
		return 0;
	}
	void release(SDL2User *u) {
		u->valid = false;
		if (--n_users == 0) {
			App *app = App::instance();
			app->remove_dispatcher(this);
			delete this;
			SDL2Context **ctx = (SDL2Context **)app->get_context("sdl2");
			*ctx = 0;
		}
	}
	// EventDispatcher
	void dispatch_events() {
		if (create_window && !window)
			SDL_CreateWindowAndRenderer(100, 100, 0, &window, &renderer);
		while (SDL_PollEvent(&ev)) {
			int i;
			for (i = 0; i < max_users; i++) {
				if (!users[i].valid || !users[i].types)
					continue;
				const SDL_EventType *t = users[i].types;
				for (int j = 0; t[j] != SDL_LASTEVENT; j++) {
					if (t[j] == ev.type) {
						if (users[i].h)
							users[i].h->handle_event();
					}
				}
			}
			if (ev.type == SDL_QUIT)
				App::instance()->quit();
		}
		if (window && renderer) {
			SDL_RenderClear(renderer);
			SDL_RenderPresent(renderer);
		}
	}

	/*
		Unfortunately, no RAII since users are driven by probe/cleanup.
		Be careful: no safeguards here!
	 */
public:
	static SDL2User *acquire(const SDL_EventType *types, EventHandler *handler, bool window_owner = false) {
		App *app = App::instance();
		SDL2Context **ctx = (SDL2Context **)app->get_context("sdl2");
		if (!*ctx) {
			*ctx = new SDL2Context();
			app->add_dispatcher(*ctx);
		}
		return (*ctx)->acquire_(types, handler, window_owner);
	}
};

inline void SDL2User::get_event(SDL_Event &ev) {
	ev = ctx->ev;
}

inline void SDL2User::release() {
	if (valid)
		ctx->release(this);
}

} // namespace frt
