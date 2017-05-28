// video_mali.cc
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
#define FRT_MOCK_GODOT_GL_CONTEXT
#else
#include "os/os.h"
#include "drivers/gl_context/context_gl.h"
#endif

#include "frt.h"

#include <stdio.h>
#include <assert.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

namespace frt {

// TODO: factor out common EGL context
class EGLMaliContext {
private:
	EGLDisplay display;
	EGLContext context;
	EGLSurface surface;
	EGLConfig config;
	struct {
		uint16_t width;
		uint16_t height;
	} nativewindow;

public:
	void init() {
		static const EGLint attr_list[] = {
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_NONE
		};
		static const EGLint ctx_attrs[] = {
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE
		};
		EGLBoolean result;
		EGLint num_config;
		display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
		assert(display != EGL_NO_DISPLAY);
		result = eglInitialize(display, 0, 0);
		assert(result != EGL_FALSE);
		result = eglChooseConfig(display, attr_list, &config, 1, &num_config);
		assert(result != EGL_FALSE);
		result = eglBindAPI(EGL_OPENGL_ES_API);
		assert(result != EGL_FALSE);
		context = eglCreateContext(display, config, EGL_NO_CONTEXT, ctx_attrs);
		assert(context != EGL_NO_CONTEXT);
	};
	void cleanup() {
		eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroyContext(display, context);
		eglTerminate(display);
	}
	void create_surface(const Vec2 &size) {
		nativewindow.width = size.x;
		nativewindow.height = size.y;
		surface = eglCreateWindowSurface(display, config,
										 (EGLNativeWindowType)&nativewindow, 0);
		assert(surface != EGL_NO_SURFACE);
	}
	void destroy_surface() {
		eglDestroySurface(display, surface);
	}
	void make_current() {
		eglMakeCurrent(display, surface, surface, context);
	}
	void release_current() {
		eglMakeCurrent(display, 0, 0, 0);
	}
	void swap_buffers() {
		eglSwapBuffers(display, surface);
	}
	void swap_interval(int interval) {
		eglSwapInterval(display, interval);
	}
};

class VideoMali : public Video, public ContextGL {
private:
	EGLMaliContext egl;
	bool initialized;
	Vec2 screen_size;
	Vec2 view_size;
	bool vsync;
	void init_egl(Vec2 size) {
		egl.init();
		egl.create_surface(size);
		egl.make_current();
		initialized = true;
	}
	void cleanup_egl() {
		if (!initialized)
			return;
		egl.destroy_surface();
		egl.cleanup();
		initialized = false;
	}

public:
	// Module
	VideoMali()
		: initialized(false), vsync(true) {}
	const char *get_id() const { return "video_mali"; }
	bool probe() {
		screen_size.x = 720;
		screen_size.y = 480;
		return true;
	}
	void cleanup() {
		cleanup_egl();
	}
	// Video
	Vec2 get_screen_size() const { return screen_size; }
	Vec2 get_view_size() const { return view_size; }
	Vec2 move_pointer(const Vec2 &screen) { return screen; }
	void show_pointer(bool enable) {}
	ContextGL *create_the_gl_context(Vec2 size) {
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
		init_egl(view_size);
		return OK;
	}
	void set_use_vsync(bool use) {
		egl.swap_interval(use ? 1 : 0);
		vsync = use;
	}
	bool is_using_vsync() const { return vsync; }
};

FRT_REGISTER(VideoMali)

} // namespace frt
