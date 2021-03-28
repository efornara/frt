// video_x11.cpp
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

#include <string.h>

#include <unistd.h>

#include "frt.h"

#include "bits/x11.h"
#include "bits/egl_base_context.h"
#include "bits/frt_load_gles.h"

namespace frt {

static const long handled_mask = 0;

static const int handled_types[] = {
	ClientMessage,
	0,
};

class VideoX11 : public Video, public ContextGL, public EventHandler {
private:
	X11User *x11;
	Display *display;
	Window window;
	Atom wm_delete_window;
	Vec2 screen_size;
	Vec2 view_size;
	EGLBaseContext egl;
	int gl_version;
	bool vsync;
	void gles_init() {
		window = x11->create_window(view_size.x, view_size.y, FRT_WINDOW_TITLE);
		wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
		XSetWMProtocols(display, window, &wm_delete_window, 1);
		egl.init(gl_version, (EGLNativeDisplayType)display);
		egl.create_simple_surface((EGLNativeWindowType)window);
		egl.make_current();
	}

public:
	VideoX11()
		: x11(0), display(0), window(0), gl_version(2), vsync(true) {
		screen_size.x = 1200;
		screen_size.y = 680;
	}
	// Module
	const char *get_id() const { return "video_x11"; }
	bool probe() {
		if (x11)
			return true;
		x11 = X11Context::acquire(handled_mask, handled_types, this, true);
		display = x11->get_display();
		if (!frt_load_egl(lib("libEGL.so.1"))) {
			x11->release();
			x11 = 0;
			return false;
		}
		return true;
	}
	void cleanup() {
		if (window) {
			egl.destroy_surface();
			egl.cleanup();
		}
		if (x11)
			x11->release();
		window = 0;
		x11 = 0;
	}
	// Video
	Vec2 get_screen_size() const { return screen_size; }
	Vec2 get_view_size() const { return view_size; }
	void set_title(const char *title) { XStoreName(display, window, title); }
	Vec2 move_pointer(const Vec2 &screen) {
		/*
		XWarpPointer(display, None, window, 0, 0, 0, 0, screen.x, screen.y);
		XFlush(x_display);
		*/
		return screen;
	}
	void show_pointer(bool enable) {}
	ContextGL *create_the_gl_context(int version, Vec2 size) {
		if (!frt_load_gles(version))
			return 0;
		gl_version = version;
		view_size = size;
		return this;
	}
	bool provides_quit() { return true; }
	// ContextGL
	void release_current() {
		egl.release_current();
	}
	void make_current() {
		egl.make_current();
	}
	void swap_buffers() {
		egl.swap_buffers();
	}
	int get_window_width() { return view_size.x; }
	int get_window_height() { return view_size.y; }
	bool initialize() {
		gles_init();
		return true;
	}
	void set_use_vsync(bool use) {
		egl.swap_interval(use ? 1 : 0);
		vsync = use;
	}
	bool is_using_vsync() const { return vsync; }
	// EventHandler
	void handle_event() {
		XEvent ev;
		x11->get_event(ev);
		switch (ev.type) {
			case ClientMessage:
				if ((unsigned int)ev.xclient.data.l[0] == (unsigned int)wm_delete_window)
					App::instance()->quit();
				break;
		}
	}
};

FRT_REGISTER(VideoX11)

} // namespace frt
