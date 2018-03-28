// video_fbdev.cc
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

#include "bits/egl_base_context.h"

extern bool frt_load_gles2(const char *filename);
extern bool frt_load_gles3(const char *filename);

static bool frt_load_gles(int version) {
#if FRT_GLES_VERSION == 3
	if (version == 3)
		return frt_load_gles3("libGLESv2.so");
#endif
	return frt_load_gles2("libGLESv2.so");
}

#define FRT_FBDEV_MALI 1

static int probe_fbdev() {
	return FRT_FBDEV_MALI;
}

namespace frt {

class EGLFBDevContext : public EGLBaseContext {
public:
	virtual void create_surface(const Vec2 &size) = 0;
};

class EGLMaliContext : public EGLFBDevContext {
private:
	struct {
		uint16_t width;
		uint16_t height;
	} nativewindow;

public:
	void create_surface(const Vec2 &size) {
		nativewindow.width = size.x;
		nativewindow.height = size.y;
		surface = eglCreateWindowSurface(display, config,
										 (EGLNativeWindowType)&nativewindow, 0);
		assert(surface != EGL_NO_SURFACE);
	}
};

class VideoFBDev : public Video, public ContextGL {
private:
	bool initialized;
	Vec2 screen_size;
	Vec2 view_size;
	EGLFBDevContext *egl;
	int gl_version;
	bool vsync;
	void gles_init() {
		egl->init(gl_version);
		egl->create_surface(view_size);
		egl->make_current();
		initialized = true;
	}

public:
	// Module
	VideoFBDev()
		: initialized(false), egl(0), gl_version(2), vsync(true) {}
	const char *get_id() const { return "video_fbdev"; }
	bool probe() {
		if (egl)
			return true;
		switch (probe_fbdev()) {
		case FRT_FBDEV_MALI:
			egl = new EGLMaliContext();
			break;
		default:
			assert(false);
		}
		if (!frt_load_egl("libEGL.so")) {
			delete egl;
			egl = 0;
			return false;
		}
		screen_size.x = 720;
		screen_size.y = 480;
		return true;
	}
	void cleanup() {
		if (initialized) {
			egl->destroy_surface();
			egl->cleanup();
			initialized = false;
		}
		if (egl) {
			delete egl;
			egl = 0;
		}
	}
	// Video
	Vec2 get_screen_size() const { return screen_size; }
	Vec2 get_view_size() const { return view_size; }
	Vec2 move_pointer(const Vec2 &screen) { return screen; }
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
		egl->release_current();
	}
	void make_current() {
		egl->make_current();
	}
	void swap_buffers() {
		egl->swap_buffers();
	}
	int get_window_width() { return view_size.x; }
	int get_window_height() { return view_size.y; }
	Error initialize() {
		gles_init();
		return OK;
	}
	void set_use_vsync(bool use) {
		egl->swap_interval(use ? 1 : 0);
		vsync = use;
	}
	bool is_using_vsync() const { return vsync; }
};

FRT_REGISTER(VideoFBDev)

} // namespace frt
