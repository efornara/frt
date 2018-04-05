// video_bcm.cc
/*
 * FRT - A Godot platform targeting single board computers
 * Copyright (c) 2017-2018  Emanuele Fornara
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

#include <sys/time.h>

#include "dl/bcm.gen.h"
#include "dl/gles2.gen.h"
#include "bits/egl_base_context.h"

#define ELEMENT_CHANGE_DEST_RECT (1 << 2)

#define NULL_ALPHA NULL
#define NULL_CLAMP NULL

namespace frt {

class Display {
private:
	static const int lcd = 0;
	DISPMANX_DISPLAY_HANDLE_T handle;
	Vec2 size;
	bool initialized;

public:
	Display()
		: initialized(false) {}
	DISPMANX_DISPLAY_HANDLE_T get_handle() const { return handle; }
	Vec2 get_size() const { return size; }
	bool init() {
		if (initialized)
			return size.x;
		bcm_host_init();
		initialized = true;
		uint32_t width, height;
		if (graphics_get_display_size(lcd, &width, &height) >= 0) {
			size.x = (int)width;
			size.y = (int)height;
		}
		return size.x;
	}
	bool open() {
		handle = vc_dispmanx_display_open(lcd);
		// TODO: 0 for error?
		return true;
	}
	void cleanup() {
		vc_dispmanx_display_close(handle);
	}
};

class Element {
private:
	bool has_resource;
	bool added;

protected:
	DISPMANX_RESOURCE_HANDLE_T resource;
	DISPMANX_ELEMENT_HANDLE_T element;
	VC_RECT_T src;
	VC_RECT_T dst;
	int layer;
	void create_resource(VC_IMAGE_TYPE_T type, int width, int height, int pitch, uint8_t *data) {
		uint32_t ptr;
		resource = vc_dispmanx_resource_create(type, width, height, &ptr);
		VC_RECT_T rect;
		vc_dispmanx_rect_set(&rect, 0, 0, width, height);
		vc_dispmanx_resource_write_data(resource, type, pitch, data, &rect);
		has_resource = true;
	}

public:
	Element(int layer_)
		: has_resource(false), added(false), layer(layer_) {}
	DISPMANX_ELEMENT_HANDLE_T get_element() const { return element; }
	void add(DISPMANX_UPDATE_HANDLE_T update, const Display &display) {
		element = vc_dispmanx_element_add(
				update, display.get_handle(), layer, &dst, resource, &src,
				DISPMANX_PROTECTION_NONE, NULL_ALPHA, NULL_CLAMP,
				DISPMANX_NO_ROTATE);
		added = true;
	}
	void remove(DISPMANX_UPDATE_HANDLE_T update) {
		if (!added)
			return;
		vc_dispmanx_element_remove(update, element);
		added = false;
	}
	void delete_resource() {
		if (!has_resource)
			return;
		vc_dispmanx_resource_delete(resource);
		has_resource = false;
	}
	void show(DISPMANX_UPDATE_HANDLE_T update, const Display &display, bool enable) {
		if (added_to_display() && !enable)
			remove(update);
		else if (!added_to_display() && enable)
			add(update, display);
	}
	bool added_to_display() const { return added; }
};

class Background : public Element {
private:
	static const VC_IMAGE_TYPE_T type = VC_IMAGE_RGB565;
	static const int pixel_size = 2;
	static const int width = 16;
	static const int height = 16;
	static const int pitch = width * pixel_size;
	uint8_t data[width * height * pixel_size];

public:
	Background()
		: Element(100) {
		memset(data, 0, sizeof(data));
	}
	void create_resource() {
		vc_dispmanx_rect_set(&src, 0, 0, width << 16, height << 16);
		Element::create_resource(type, width, height, pitch, data);
	}
	void set_metrics(const Display &display) {
		Vec2 dpy_size = display.get_size();
		vc_dispmanx_rect_set(&dst, 0, 0, dpy_size.x, dpy_size.y);
	}
};

static void update_cb(DISPMANX_UPDATE_HANDLE_T u, void *arg) {
}

class View : public Element {
private:
	enum Gravity {
		Center,
		TopLeft,
		TopRight,
		BottomRight,
		BottomLeft,
	};
	friend class Pointer;
	Vec2 size;
	Vec2 dpy_size;
	int ox, oy;
	float scalex, scaley;
	bool fullscreen;
	Gravity gravity;

	void set_dst_fullscreen() {
		Vec2 win;
		double req_aspect = (double)size.x / size.y;
		double scr_aspect = (double)dpy_size.x / dpy_size.y;
		if (req_aspect >= scr_aspect) {
			win.x = dpy_size.x;
			win.y = (int)(dpy_size.x / req_aspect);
		} else {
			win.x = (int)(dpy_size.y * req_aspect);
			win.y = dpy_size.y;
		}
		ox = (dpy_size.x - win.x) / 2;
		oy = (dpy_size.y - win.y) / 2;
		scalex = (float)size.x / win.x;
		scaley = (float)size.y / win.y;
		vc_dispmanx_rect_set(&dst, ox, oy, win.x, win.y);
	}
	void set_dst_window() {
		switch (gravity) {
			case Center:
				ox = (dpy_size.x - size.x) / 2;
				break;
			case TopLeft:
			case BottomLeft:
				ox = 0;
				break;
			case TopRight:
			case BottomRight:
				ox = dpy_size.x - size.x;
				break;
		}
		switch (gravity) {
			case Center:
				oy = (dpy_size.y - size.y) / 2;
				break;
			case TopLeft:
			case TopRight:
				oy = 0;
				break;
			case BottomLeft:
			case BottomRight:
				oy = dpy_size.y - size.y;
				break;
		}
		scalex = 1.0f;
		scaley = 1.0f;
		vc_dispmanx_rect_set(&dst, ox, oy, size.x, size.y);
	}
	void set_dst() {
		if (fullscreen)
			set_dst_fullscreen();
		else
			set_dst_window();
	}
	void schedule_update(DISPMANX_UPDATE_HANDLE_T update) {
		vc_dispmanx_element_change_attributes(
				update, element, ELEMENT_CHANGE_DEST_RECT, 0, 255, &dst,
				&src, 0, DISPMANX_NO_ROTATE);
	}

public:
	View()
		: Element(101), fullscreen(true), gravity(Center) {}
	Vec2 get_size() const { return size; }
	void set_metrics(const Display &display, const Vec2 &size) {
		this->size = size;
		vc_dispmanx_rect_set(&src, 0, 0, size.x << 16, size.y << 16);
		dpy_size = display.get_size();
		set_dst();
	}
	bool toggle_fullscreen(DISPMANX_UPDATE_HANDLE_T update) {
		fullscreen = !fullscreen;
		set_dst();
		schedule_update(update);
		return fullscreen;
	}
	bool toggle_window(DISPMANX_UPDATE_HANDLE_T update) {
		if (fullscreen)
			fullscreen = false;
		else if (gravity == BottomLeft)
			gravity = Center;
		else
			gravity = (Gravity)((int)gravity + 1);
		set_dst();
		schedule_update(update);
		return fullscreen;
	}
};

#include "import/cursor.h"

class Pointer : public Element {
private:
	static const VC_IMAGE_TYPE_T type = VC_IMAGE_RGBA32;
	static const int pixel_size = 4;
	static const int width = 16;
	static const int height = 16;
	static const int pitch = width * pixel_size;
	static const uint32_t shape_color = 0xff000000;
	static const uint32_t mask_color = 0xffffffff;
	static const uint32_t transparent_color = 0x00000000;
	static const int layer_visible = 102;
	static const int min_dt_us = 16667;
	uint8_t data[width * height * pixel_size];
	int x, y;
	bool visible, need_updating;
	struct timeval last_update;
	inline uint8_t *fill_byte(uint8_t *p, int shape, int mask) {
		uint32_t color;
		int bitmask = 0x01;
		for (int i = 0; i < 8; i++) {
			if (shape & bitmask)
				color = shape_color;
			else if (mask & bitmask)
				color = mask_color;
			else
				color = transparent_color;
			memcpy(p, &color, 4);
			p += 4;
			bitmask <<= 1;
		}
		return p;
	}
	void convert_bitmaps(const uint8_t *shape, const uint8_t *mask) {
		uint8_t *p = data;
		for (int y = 0; y < 32; y++)
			p = fill_byte(p, *shape++, *mask++);
	}
	void fill_dst() {
		vc_dispmanx_rect_set(&dst, x - left_ptr_x_hot, y - left_ptr_y_hot,
							 width, height);
	}

public:
	Pointer()
		: Element(layer_visible), visible(true) {
		convert_bitmaps(left_ptr_bits, left_ptrmsk_bits);
	}
	void create_resource() {
		vc_dispmanx_rect_set(&src, 0, 0, width << 16, height << 16);
		Element::create_resource(type, width, height, pitch, data);
	}
	void set_metrics(const Display &display) {
		Vec2 dpy_size = display.get_size();
		x = dpy_size.x - 1;
		y = dpy_size.y - 1;
		fill_dst();
		gettimeofday(&last_update, 0);
	}
	Vec2 set_pos(const View &view, const Vec2 &screen) {
		Vec2 res;
		res.x = (int)((screen.x - view.ox) * view.scalex);
		res.y = (int)((screen.y - view.oy) * view.scaley);
		x = screen.x;
		y = screen.y;
		fill_dst();
		if (visible)
			need_updating = true;
		return res;
	}
	void set_visible(const Display &display, bool visible) {
		if (this->visible == visible)
			return;
		this->visible = visible;
		DISPMANX_UPDATE_HANDLE_T update;
		update = vc_dispmanx_update_start(1);
		show(update, display, visible);
		vc_dispmanx_update_submit_sync(update);
	}
	void schedule_update_if_needed(bool vsync) {
		if (!need_updating || !visible)
			return;
		if (!vsync) {
			struct timeval now;
			struct timeval dt;
			gettimeofday(&now, 0);
			timersub(&now, &last_update, &dt);
			if (dt.tv_sec == 0 && dt.tv_usec < min_dt_us)
				return;
			last_update = now;
		}
		DISPMANX_UPDATE_HANDLE_T update;
		update = vc_dispmanx_update_start(1);
		vc_dispmanx_element_change_attributes(
				update, element, ELEMENT_CHANGE_DEST_RECT, 0, 255, &dst,
				&src, 0, DISPMANX_NO_ROTATE);
		vc_dispmanx_update_submit(update, update_cb, 0);
		need_updating = false;
	}
};

class EGLDispmanxContext : public EGLBaseContext {
private:
	EGL_DISPMANX_WINDOW_T nativewindow;

public:
	void create_surface(const View &view) {
		nativewindow.element = view.get_element();
		Vec2 size = view.get_size();
		nativewindow.width = size.x;
		nativewindow.height = size.y;
		surface = eglCreateWindowSurface(display, config,
										 (EGLNativeWindowType)&nativewindow, 0);
		if (surface == EGL_NO_SURFACE)
			fatal("video_bcm: eglCreateWindowSurface failed.");
	}
};

class VideoBCM : public Video, public ContextGL {
private:
	Display dpymnx;
	EGLDispmanxContext egl;
	Background background;
	View view;
	Pointer pointer;
	bool initialized;
	Vec2 screen_size;
	Vec2 view_size;
	bool vsync;
	void init_egl_and_dpymnx(Vec2 size) {
		egl.init(2);
		dpymnx.open();
		background.create_resource();
		pointer.create_resource();
		background.set_metrics(dpymnx);
		view.set_metrics(dpymnx, size);
		pointer.set_metrics(dpymnx);
		DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
		background.add(update, dpymnx);
		view.add(update, dpymnx);
		pointer.add(update, dpymnx);
		vc_dispmanx_update_submit_sync(update);
		egl.create_surface(view);
		egl.make_current();
		initialized = true;
	}
	void cleanup_egl_and_dpymnx() {
		if (!initialized)
			return;
		egl.destroy_surface();
		DISPMANX_UPDATE_HANDLE_T update = vc_dispmanx_update_start(0);
		background.remove(update);
		view.remove(update);
		pointer.remove(update);
		vc_dispmanx_update_submit_sync(update);
		background.delete_resource();
		pointer.delete_resource();
		dpymnx.cleanup();
		egl.cleanup();
		initialized = false;
	}

public:
	// Module
	VideoBCM()
		: initialized(false), vsync(true) {}
	const char *get_id() const { return "video_bcm"; }
	bool probe() {
		if (!frt_load_bcm("libbcm_host.so"))
			return false;
		if (!frt_load_gles2("libbrcmGLESv2.so"))
			return false;
		if (!frt_load_egl("libbrcmEGL.so"))
			return false;
		if (!dpymnx.init())
			return false;
		screen_size = dpymnx.get_size();
		return true;
	}
	void cleanup() {
		cleanup_egl_and_dpymnx();
	}
	bool handle_meta(int gd_code, bool pressed) {
		DISPMANX_UPDATE_HANDLE_T update;
		bool fullscreen;
		if (!pressed)
			return false;
		switch (gd_code) {
			case 'F':
				update = vc_dispmanx_update_start(1);
				fullscreen = view.toggle_fullscreen(update);
				background.show(update, dpymnx, fullscreen);
				vc_dispmanx_update_submit(update, update_cb, 0);
				return true;
			case 'W':
				update = vc_dispmanx_update_start(1);
				fullscreen = view.toggle_window(update);
				background.show(update, dpymnx, fullscreen);
				vc_dispmanx_update_submit(update, update_cb, 0);
				return true;
			default:
				return false;
		}
	}
	// Video
	Vec2 get_screen_size() const { return screen_size; }
	Vec2 get_view_size() const { return view_size; }
	void set_title(const char *title) {}
	Vec2 move_pointer(const Vec2 &screen) {
		return pointer.set_pos(view, screen);
	}
	void show_pointer(bool enable) {
		pointer.set_visible(dpymnx, enable);
	}
	ContextGL *create_the_gl_context(int version, Vec2 size) {
		if (version != 2)
			return 0;
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
		pointer.schedule_update_if_needed(vsync);
		egl.swap_buffers();
	}
	int get_window_width() { return view_size.x; }
	int get_window_height() { return view_size.y; }
	Error initialize() {
		init_egl_and_dpymnx(view_size);
		return OK;
	}
	void set_use_vsync(bool use) {
		egl.swap_interval(use ? 1 : 0);
		vsync = use;
	}
	bool is_using_vsync() const { return vsync; }
};

FRT_REGISTER(VideoBCM)

} // namespace frt
