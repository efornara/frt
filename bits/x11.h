// x11.h
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

#include <stdlib.h>

#include "dl/x11.gen.h"

namespace frt {

class X11Context;

struct X11User {
private:
	friend class X11Context;
	X11Context *ctx;
	EventHandler *h;
	long mask;
	const int *types;
	bool valid;
	X11User()
		: valid(false) {}

public:
	inline Display *get_display();
	inline Window create_window(int width, int height, const char *title);
	inline void get_event(XEvent &ev);
	inline void release();
};

class X11Context : public EventDispatcher {
private:
	friend struct X11User;
	static const int max_users = 10;
	X11User users[max_users];
	int n_users;
	bool create_window;
	Display *display;
	Window window;
	XEvent ev;
	long current_mask;
	long server_mask;
	X11Context()
		: n_users(0), create_window(true), display(0), window(0), current_mask(0), server_mask(0) {
		display = XOpenDisplay(NULL);
	}
	virtual ~X11Context() {
		if (!display)
			return;
		if (window)
			XDestroyWindow(display, window);
		XCloseDisplay(display);
	}
	Window create(int width, int height, const char *title) {
		bool undecorated = getenv("FRT_X11_UNDECORATED");
		Window root = DefaultRootWindow(display);
		XSetWindowAttributes swa;
		window = XCreateWindow(display, root, 0, 0,
							   width, height, 0, CopyFromParent,
							   InputOutput, CopyFromParent, 0, &swa);
		XSizeHints sh;
		sh.min_width = width;
		sh.min_height = height;
		sh.max_width = width;
		sh.max_height = height;
		sh.flags = PMinSize | PMaxSize;
		if (undecorated) {
			sh.x = 0;
			sh.y = 0;
			sh.flags |= PPosition;
		}
		XSetWMNormalHints(display, window, &sh);
		XMapWindow(display, window);
		XStoreName(display, window, title);
		if (undecorated) {
			Atom motif_wm_hints = XInternAtom(display, "_MOTIF_WM_HINTS", False);
			long hints[5] = { 2, 0, 0, 0, 0 };
			XChangeProperty(display, window, motif_wm_hints, motif_wm_hints, 32, PropModeReplace, (unsigned char *)hints, 5);
		}
		XFlush(display);
		return window;
	}
	X11User *acquire_(long mask, const int *types, EventHandler *handler, bool window_owner) {
		for (int i = 0; i < max_users; i++) {
			if (users[i].valid)
				continue;
			n_users++;
			users[i].valid = true;
			users[i].ctx = this;
			users[i].mask = mask;
			users[i].types = types;
			users[i].h = handler;
			if (window_owner)
				create_window = false;
			current_mask |= users[i].mask;
			return &users[i];
		}
		return 0;
	}
	void release(X11User *u) {
		u->valid = false;
		if (--n_users == 0) {
			App *app = App::instance();
			app->remove_dispatcher(this);
			delete this;
			X11Context **ctx = (X11Context **)app->get_context("x11");
			*ctx = 0;
		}
	}
	// EventDispatcher
	void dispatch_events() {
		if (create_window && !window)
			create(100, 100, FRT_WINDOW_TITLE);
		if (current_mask != server_mask) {
			XSelectInput(display, window, current_mask);
			server_mask = current_mask;
		}
		while (XPending(display)) {
			XNextEvent(display, &ev);
			int i;
			for (i = 0; i < max_users; i++) {
				if (!users[i].valid || !users[i].types)
					continue;
				const int *t = users[i].types;
				for (int j = 0; t[j] != 0; j++) {
					if (t[j] == ev.type) {
						if (users[i].h)
							users[i].h->handle_event();
					}
				}
			}
		}
	}

	/*
		Unfortunately, no RAII since users are driven by probe/cleanup.
		Be careful: no safeguards here!
	 */
public:
	static X11User *acquire(long mask, const int *types, EventHandler *handler, bool window_owner = false) {
		App *app = App::instance();
		X11Context **ctx = (X11Context **)app->get_context("x11");
		if (!*ctx) {
			if (!frt_load_x11("libX11.so.6"))
				return 0;
			*ctx = new X11Context();
			app->add_dispatcher(*ctx);
		}
		return (*ctx)->acquire_(mask, types, handler, window_owner);
	}
};

inline Display *X11User::get_display() {
	return ctx->display;
}

inline Window X11User::create_window(int width, int height, const char *title) {
	return ctx->create(width, height, title);
}

inline void X11User::get_event(XEvent &ev) {
	ev = ctx->ev;
}

inline void X11User::release() {
	if (valid)
		ctx->release(this);
}

} // namespace frt
