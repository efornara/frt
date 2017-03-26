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

#include <assert.h>

#ifdef FRT_TEST
#define FRT_MOCK_GODOT_GL_CONTEXT
#else
#include "os/os.h"
#include "drivers/gl_context/context_gl.h"
#endif

#include "frt.h"

#include <EGL/egl.h>
#include <X11/Xlib.h>

/*
	WARNING: This is not a proper implementation!
	It is just a minimal version to help to test other modules on a typical
	UNIX setup.
 */

namespace frt {

class VideoX11 : public Video, public ContextGL {
private:
	Display *x_display;
	Vec2 screen_size;
	Vec2 view_size;
	Window x_window;
	EGLDisplay egl_display;
	EGLContext egl_context;
	EGLSurface egl_surface;
	void gles2_init() {
		int res;
		Window root = DefaultRootWindow(x_display);
		XSetWindowAttributes swa;
		swa.event_mask = ExposureMask;
		x_window = XCreateWindow(x_display, root, 0, 0, view_size.x, view_size.y, 0, CopyFromParent,
								 InputOutput, CopyFromParent, CWEventMask, &swa);
		XMapWindow(x_display, x_window);
		XStoreName(x_display, x_window, "FRT");
		XFlush(x_display);

		egl_display = eglGetDisplay((EGLNativeDisplayType)x_display);
		assert(egl_display != EGL_NO_DISPLAY);
		res = eglInitialize(egl_display, 0, 0);
		assert(res);

		EGLint attr[] = { EGL_BUFFER_SIZE, 16, EGL_RENDERABLE_TYPE,
						  EGL_OPENGL_ES2_BIT, EGL_NONE };
		EGLConfig ecfg;
		EGLint num_config;
		res = eglChooseConfig(egl_display, attr, &ecfg, 1, &num_config);
		assert(res);
		assert(num_config == 1);

		egl_surface = eglCreateWindowSurface(egl_display, ecfg, x_window, 0);
		assert(egl_surface != EGL_NO_SURFACE);

		EGLint ctxattr[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };
		egl_context = eglCreateContext(egl_display, ecfg, EGL_NO_CONTEXT, ctxattr);
		assert(egl_context != EGL_NO_CONTEXT);
		make_current();
	}

public:
	VideoX11()
		: x_display(0) {
		screen_size.x = 1200;
		screen_size.y = 680;
	}
	// Module
	const char *get_id() const { return "video_x11"; }
	bool probe() {
		if (x_display)
			return true;
		x_display = XOpenDisplay(NULL);
		return x_display;
	}
	void cleanup() {
		if (!x_display)
			return;
		XCloseDisplay(x_display);
		x_display = 0;
	}
	// Video
	Vec2 get_screen_size() const { return screen_size; }
	Vec2 get_view_size() const { return view_size; }
	Vec2 move_pointer(const Vec2 &screen) {
		XWarpPointer(x_display, None, x_window, 0, 0, 0, 0, screen.x, screen.y);
		XFlush(x_display);
		return screen;
	}
	void show_pointer(bool enable) {}
	ContextGL *create_the_gl_context(Vec2 size) {
		view_size = size;
		return this;
	}
	// GL_Context
	void release_current() {
		eglMakeCurrent(egl_display, 0, 0, 0);
	}
	void make_current() {
		assert(x_display);
		eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);
	}
	void swap_buffers() {
		assert(x_display);
		eglSwapBuffers(egl_display, egl_surface);
	}
	int get_window_width() { return view_size.x; }
	int get_window_height() { return view_size.y; }
	Error initialize() {
		gles2_init();
		return OK;
	}
	void set_use_vsync(bool use) {}
	bool is_using_vsync() const { return true; }
};

FRT_REGISTER(VideoX11)

} // namespace frt
