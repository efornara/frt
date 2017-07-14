// video_sdl2.cpp
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
#include "bits/sdl2.h"

namespace frt {

class VideoSDL2 : public Video, public ContextGL {
private:
	SDL2User *sdl2;
	SDL_Window *window;
	Vec2 screen_size;
	Vec2 view_size;
	SDL_GLContext gl;

public:
	VideoSDL2()
		: sdl2(0), window(0) {
		screen_size.x = 1200;
		screen_size.y = 680;
	}
	// Module
	const char *get_id() const { return "video_sdl2"; }
	bool probe() {
		if (!sdl2)
			sdl2 = SDL2Context::acquire(0, 0, true);
		return true;
	}
	void cleanup() {
		if (window)
			SDL_DestroyWindow(window);
		// SDL_GL_DeleteContext(gl);
		if (sdl2) {
			sdl2->release();
			sdl2 = 0;
		}
	}
	// Video
	Vec2 get_screen_size() const { return screen_size; }
	Vec2 get_view_size() const { return view_size; }
	Vec2 move_pointer(const Vec2 &screen) {
		/*
		XWarpPointer(x_display, None, x_window, 0, 0, 0, 0, screen.x, screen.y);
		XFlush(x_display);
		*/
		return screen;
	}
	void show_pointer(bool enable) {}
	ContextGL *create_the_gl_context(Vec2 size) {
		view_size = size;
		return this;
	}
	// GL_Context
	void release_current() {
		// TODO
	}
	void make_current() {
		SDL_GL_MakeCurrent(window, gl);
	}
	void swap_buffers() {
		SDL_GL_SwapWindow(window);
	}
	int get_window_width() { return view_size.x; }
	int get_window_height() { return view_size.y; }
	Error initialize() {
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, FRT_GLES_VERSION);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		window = SDL_CreateWindow("FRT", SDL_WINDOWPOS_UNDEFINED,
								  SDL_WINDOWPOS_UNDEFINED, view_size.x, view_size.y,
								  SDL_WINDOW_OPENGL);
		assert(window);
		gl = SDL_GL_CreateContext(window);
		return OK;
	}
	void set_use_vsync(bool use) {}
	bool is_using_vsync() const { return true; }
};

FRT_REGISTER(VideoSDL2)

} // namespace frt
