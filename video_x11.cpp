// video_x11.cpp
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

#include <string.h>
#include <assert.h>

#include <unistd.h>

#ifdef FRT_TEST
#define FRT_MOCK_GODOT_GL_CONTEXT
#else
#include "os/os.h"
#include "drivers/gl_context/context_gl.h"
#endif

#include "frt.h"

#include "bits/x11.h"
#include "bits/egl_base_context.h"

// on pi, skip EGL/GLESv2 in /opt/vc/lib
static const char *lib(const char *s) {
#if defined(__arm__) || defined(__aarch64__)
	static char buf[64]; // large enough
	strcpy(buf, "/opt/vc/lib/");
	strcat(buf, s);
	if (access(buf, R_OK) != 0)
		return s;
#if defined(__arm__)
	strcpy(buf, "/usr/lib/arm-linux-gnueabihf/");
#else
	strcpy(buf, "/usr/lib/aarch64-linux-gnu/");
#endif
	strcat(buf, s);
	return buf;
#else // !pi
	return s;
#endif
}

extern bool frt_load_gles2(const char *filename);
extern bool frt_load_gles3(const char *filename);

static bool frt_load_gles(int version) {
#if FRT_GLES_VERSION == 3
	if (version == 3)
		return frt_load_gles3(lib("libGLESv2.so"));
#endif
	return frt_load_gles2(lib("libGLESv2.so"));
}

namespace frt {

class VideoX11 : public Video, public ContextGL {
private:
	X11User *x11;
	Display *display;
	Window window;
	Vec2 screen_size;
	Vec2 view_size;
	EGLBaseContext egl;
	int gl_version;
	bool vsync;
	void gles_init() {
		if (!x11)
			return;
		window = x11->create_window(view_size.x, view_size.y, FRT_WINDOW_TITLE);
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
		long mask = 0;
		const int *types = 0;
		x11 = X11Context::acquire(mask, types, 0, true);
		display = x11->get_display();
		if (!frt_load_egl(lib("libEGL.so"))) {
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
	// GL_Context
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
	Error initialize() {
		gles_init();
		return OK;
	}
	void set_use_vsync(bool use) {
		egl.swap_interval(use ? 1 : 0);
		vsync = use;
	}
	bool is_using_vsync() const { return vsync; }
};

FRT_REGISTER(VideoX11)

} // namespace frt
