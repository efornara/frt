// video_kmsdrm.cpp
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

#include "frt.h"

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "dl/drm.gen.h"
#include "dl/gbm.gen.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#include "bits/egl_base_context.h"
#include "bits/frt_load_gles.h"

namespace frt {

class EGLKMSDRMContext : public EGLBaseContext {
private:
	int device;
	drmModeRes *resources;
	drmModeConnector *connector;
	uint32_t connector_id;
	drmModeEncoder *encoder;
	drmModeModeInfo mode_info;
	drmModeCrtc *crtc;
	struct gbm_device *gbm_device;
	struct gbm_surface *gbm_surface;
	struct gbm_bo *previous_bo = NULL;
	uint32_t previous_fb;
	struct gbm_bo *bo;
	uint32_t handle;
	uint32_t pitch;
	uint32_t fb;
	uint64_t modifier;
	EGLConfig configs[32];
	int config_index;

	drmModeConnector *find_connector(drmModeRes *resources) {
		for (int i = 0; i < resources->count_connectors; i++) {
			drmModeConnector *connector = drmModeGetConnector(device, resources->connectors[i]);
			if (connector->connection == DRM_MODE_CONNECTED)
				return connector;
			drmModeFreeConnector(connector);
		}

		return NULL;
	}

	drmModeEncoder *find_encoder(drmModeRes *resources, drmModeConnector *connector) {
		if (connector->encoder_id)
			return drmModeGetEncoder(device, connector->encoder_id);
		return NULL;
	}

	int match_config_to_visual(EGLDisplay egl_display, EGLint visual_id, EGLConfig *configs, int count) {
		EGLint id;
		for (int i = 0; i < count; ++i) {
			if (!eglGetConfigAttrib(egl_display, configs[i], EGL_NATIVE_VISUAL_ID, &id)) continue;
			if (id == visual_id)
				return i;
		}
		return -1;
	}

public:
	void init(int version) {
		EGLBoolean result;
		EGLint num_config;
		EGLint count = 0;

		static const EGLint context_attribs[] = {
			EGL_CONTEXT_CLIENT_VERSION, version,
			EGL_NONE
		};

		static EGLint attributes[] = {
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, EGL_DONT_CARE,
			EGL_NONE
		};

		//! Try and get the correct card to use from the env
		const char *s = getenv("FRT_KMSDRM_DEVICE");
		if (s) {
			if (access(s, R_OK) == 0)
				device = open(s, O_RDWR);
			else
				fatal("couldn't open %s.", s);
		}
		//! No env var found, try card1 (rpi4)
		else if (access("/dev/dri/card1", R_OK) == 0)
			device = open("/dev/dri/card1", O_RDWR);
		//! That didn't work, fall back to card0 (pc, pi3, others)
		else if (access("/dev/dri/card0", R_OK) == 0)
			device = open("/dev/dri/card0", O_RDWR);
		else
			fatal("no /dev/dri/card found.");
		if (device < 0)
			fatal("open returned an invalid device.");
		else {
			resources = drmModeGetResources(device);
			if (resources == 0)
				fatal("failed to get resources.");
			connector = find_connector(resources);
			if (connector == 0)
				fatal("failed to get connector. no fb?");
		}
		connector_id = connector->connector_id;
		mode_info = connector->modes[0];
		encoder = find_encoder(resources, connector);
		crtc = drmModeGetCrtc(device, encoder->crtc_id);
		drmModeFreeEncoder(encoder);
		drmModeFreeConnector(connector);
		drmModeFreeResources(resources);
		gbm_device = gbm_create_device(device);
		gbm_surface = gbm_surface_create(gbm_device, mode_info.hdisplay, mode_info.vdisplay, GBM_FORMAT_XRGB8888, GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
		display = eglGetDisplay(gbm_device);

		result = eglInitialize(display, NULL, NULL);
		if (result == EGL_FALSE)
			fatal("eglInitialize failed.");
		result = eglBindAPI(EGL_OPENGL_ES_API);
		if (result == EGL_FALSE)
			fatal("eglBindAPI failed.");

		eglGetConfigs(display, NULL, 0, &count);
		result = eglChooseConfig(display, attributes, &configs[0], count, &num_config);
		if (result == EGL_FALSE)
			fatal("eglChooseConfig failed.");

		config_index = match_config_to_visual(display, GBM_FORMAT_XRGB8888, &configs[0], num_config);
		context = eglCreateContext(display, configs[config_index], EGL_NO_CONTEXT, context_attribs);
		if (context == EGL_NO_CONTEXT)
			fatal("eglCreateContext failed.");
	}

	void create_surface() {
		surface = eglCreateWindowSurface(display, configs[config_index], gbm_surface, NULL);
		if (surface == EGL_NO_SURFACE)
			fatal("eglCreateWindowSurface failed.");
	}

	void swap_buffers() {
		eglSwapBuffers(display, surface);
		bo = gbm_surface_lock_front_buffer(gbm_surface);
		handle = gbm_bo_get_handle(bo).u32;
		pitch = gbm_bo_get_stride(bo);
		drmModeAddFB(device, mode_info.hdisplay, mode_info.vdisplay, 24, 32, pitch, handle, &fb);
		drmModeSetCrtc(device, crtc->crtc_id, fb, 0, 0, &connector_id, 1, &mode_info);
		if (previous_bo != 0) {
			drmModeRmFB(device, previous_fb);
			gbm_surface_release_buffer(gbm_surface, previous_bo);
		}
		previous_bo = bo;
		previous_fb = fb;
	}
	int getDevice() { return device; }
	drmModeCrtc *getCrtc() { return crtc; }
	uint32_t *getConnector_id() { return &connector_id; }
	struct gbm_bo *getPrevious_bo() {
		return previous_bo;
	}
	uint32_t getPrevious_fb() { return previous_fb; }
	struct gbm_surface *getGbm_surface() {
		return gbm_surface;
	}
	struct gbm_device *getGbm_device() {
		return gbm_device;
	}
};

class VideoKMSDRM : public Video, public ContextGL {
private:
	EGLKMSDRMContext egl;
	bool initialized;
	Vec2 screen_size;
	bool vsync;
	drmModeCrtc *crtc;
	int gl_version;

	void init_egl(Vec2 size) {
		egl.init(gl_version);
		egl.create_surface();
		egl.make_current();

		crtc = egl.getCrtc();

		screen_size.x = get_window_width();
		screen_size.y = get_window_height();

		initialized = true;
	}
	void cleanup_egl() {
		if (!initialized)
			return;

		if (!crtc)
			return;

		drmModeSetCrtc(egl.getDevice(), crtc->crtc_id, crtc->buffer_id, crtc->x, crtc->y, egl.getConnector_id(), 1, &crtc->mode);
		drmModeFreeCrtc(crtc);
		if (egl.getPrevious_bo()) {
			drmModeRmFB(egl.getDevice(), egl.getPrevious_fb());
			gbm_surface_release_buffer(egl.getGbm_surface(), egl.getPrevious_bo());
		}
		egl.destroy_surface();
		gbm_surface_destroy(egl.getGbm_surface());
		egl.cleanup();
		gbm_device_destroy(egl.getGbm_device());
		close(egl.getDevice());
		initialized = false;
	}

public:
	// Module
	VideoKMSDRM()
		: initialized(false), vsync(true) {}
	const char *get_id() const { return "video_kmsdrm"; }
	bool probe() {
		if (!frt_load_gbm("libgbm.so.1"))
			return false;
		if (!frt_load_drm("libdrm.so.2"))
			return false;
		if (!frt_load_egl(lib("libEGL.so.1")))
			return false;
		return true;
	}
	void cleanup() {
		cleanup_egl();
	}
	// Video
	Vec2 get_screen_size() const { return screen_size; }
	Vec2 get_view_size() const { return screen_size; }
	ContextGL *create_the_gl_context(int version, Vec2 size) {
		if (!frt_load_gles(version))
			return 0;
		gl_version = version;
		screen_size = size;
		return this;
	}
	bool provides_quit() { return false; }
	void set_title(const char *title) {}
	Vec2 move_pointer(const Vec2 &screen) {
		return Vec2(0, 0);
	}
	void show_pointer(bool enable) {}
	int get_window_height() {
		int h = 0;
		if (crtc)
			h = (int)crtc->height;
		return h;
	}
	int get_window_width() {
		int w = 0;
		if (crtc)
			w = (int)crtc->width;
		return w;
	}

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
	bool initialize() {
		init_egl(screen_size);
		return true;
	}
	void set_use_vsync(bool use) {
		egl.swap_interval(use ? 1 : 0);
		vsync = use;
	}
	bool is_using_vsync() const { return vsync; }
};

FRT_REGISTER(VideoKMSDRM)

} // namespace frt
