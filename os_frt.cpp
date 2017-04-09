// os_frt.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include "version.h"
#include "os/os.h"
#include "os/input.h"
#include "drivers/unix/os_unix.h"
#include "drivers/gl_context/context_gl.h"
#include "servers/visual_server.h"
#include "servers/visual/rasterizer.h"
#include "servers/physics_server.h"
#include "servers/audio/audio_driver_dummy.h"
#include "servers/physics/physics_server_sw.h"
#include "servers/physics_2d/physics_2d_server_sw.h"
#include "servers/physics_2d/physics_2d_server_wrap_mt.h"
#include "servers/visual/visual_server_raster.h"
#include "drivers/alsa/audio_driver_alsa.h"
#include "drivers/pulseaudio/audio_driver_pulseaudio.h"
#include "drivers/rtaudio/audio_driver_rtaudio.h"
#include "main/main.h"
#include "main/input_default.h"
#include "print_string.h"

#if VERSION_MAJOR == 2

#include "servers/visual/visual_server_wrap_mt.h"
#include "servers/audio/audio_server_sw.h"
#include "servers/audio/sample_manager_sw.h"
#include "servers/spatial_sound/spatial_sound_server_sw.h"
#include "servers/spatial_sound_2d/spatial_sound_2d_server_sw.h"
#include "drivers/gles2/rasterizer_gles2.h"

#elif VERSION_MAJOR == 3

#include "drivers/gles3/rasterizer_gles3.h"
typedef AudioDriverManager AudioDriverManagerSW;
typedef AudioDriver AudioDriverSW;

#else
#error "unhandled godot version"
#endif

#include "frt.h"
#include "bits/mouse_virtual.h"

using namespace frt;

class OS_FRT : public OS_Unix, public Runnable {
private:
	Environment *env;
	Vec2 screen_size;
	ContextGL *context_gl;
	VisualServer *visual_server;
	VideoMode current_videomode;
	List<String> args;
	MainLoop *main_loop;
#ifdef ALSA_ENABLED
	AudioDriverALSA driver_alsa;
#endif
	AudioDriverDummy driver_dummy;
	int audio_driver_index;
	Point2 mouse_pos;
	MouseMode mouse_mode;
	int mouse_state;
	bool grab;
	PhysicsServer *physics_server;
	Physics2DServer *physics_2d_server;
#if VERSION_MAJOR == 2
	Rasterizer *rasterizer;
	AudioServerSW *audio_server;
	SampleManagerMallocSW *sample_manager;
	SpatialSoundServerSW *spatial_sound_server;
	SpatialSound2DServerSW *spatial_sound_2d_server;
#endif
	int event_id;
	InputDefault *input;
	bool quit;
	MouseVirtual mouse_virtual;

public:
	int get_video_driver_count() const { return 1; }
	const char *get_video_driver_name(int driver) const { return "GLES2"; }
	OS::VideoMode get_default_video_mode() const {
		return OS::VideoMode(screen_size.x, screen_size.y, true, false, true);
	}
	int get_audio_driver_count() const {
		return AudioDriverManagerSW::get_driver_count();
	}
	const char *get_audio_driver_name(int driver) const {
		AudioDriverSW *driver_ = AudioDriverManagerSW::get_driver(driver);
		ERR_FAIL_COND_V(!driver_, "");
		return driver_->get_name();
	}
	void initialize(const VideoMode &desired, int video_driver, int audio_driver) {
		args = OS::get_singleton()->get_cmdline_args();
		current_videomode = desired;
		main_loop = 0;
		Vec2 view(current_videomode.width, current_videomode.height);
		context_gl = env->video->create_the_gl_context(view);
		context_gl->initialize();
#if VERSION_MAJOR == 2
		rasterizer = memnew(RasterizerGLES2);
		visual_server = memnew(VisualServerRaster(rasterizer));
		if (get_render_thread_mode() != RENDER_THREAD_UNSAFE)
			visual_server = memnew(VisualServerWrapMT(
					visual_server, get_render_thread_mode() == RENDER_SEPARATE_THREAD));
#else
		visual_server = memnew(VisualServerRaster);
#endif
		// TODO: Audio Module
		AudioDriverManagerSW::get_driver(audio_driver)->set_singleton();
		audio_driver_index = audio_driver;
		if (AudioDriverManagerSW::get_driver(audio_driver)->init() != OK) {
			audio_driver_index = -1;
			for (int i = 0; i < AudioDriverManagerSW::get_driver_count(); i++) {
				if (i == audio_driver)
					continue;
				AudioDriverManagerSW::get_driver(i)->set_singleton();
				if (AudioDriverManagerSW::get_driver(i)->init() == OK) {
					audio_driver_index = i;
					break;
				}
			}
		}
#if VERSION_MAJOR == 2
		sample_manager = memnew(SampleManagerMallocSW);
		audio_server = memnew(AudioServerSW(sample_manager));
		audio_server->init();
		spatial_sound_server = memnew(SpatialSoundServerSW);
		spatial_sound_server->init();
		spatial_sound_2d_server = memnew(SpatialSound2DServerSW);
		spatial_sound_2d_server->init();
#endif
		ERR_FAIL_COND(!visual_server);
		visual_server->init();
		physics_server = memnew(PhysicsServerSW);
		physics_server->init();
		physics_2d_server = memnew(Physics2DServerSW);
		physics_2d_server->init();
		input = memnew(InputDefault);
		_ensure_data_dir();
	}
	void finalize() {
		if (main_loop)
			memdelete(main_loop);
		main_loop = NULL;
#if VERSION_MAJOR == 2
		spatial_sound_server->finish();
		memdelete(spatial_sound_server);
		spatial_sound_2d_server->finish();
		memdelete(spatial_sound_2d_server);
		memdelete(sample_manager);
		audio_server->finish();
		memdelete(audio_server);
		visual_server->finish();
#endif
		memdelete(visual_server);
#if VERSION_MAJOR == 2
		memdelete(rasterizer);
#endif
		physics_server->finish();
		memdelete(physics_server);
		physics_2d_server->finish();
		memdelete(physics_2d_server);
		memdelete(input);
		args.clear();
	}
	void set_mouse_show(bool show) {}
	void set_mouse_grab(bool grab) { this->grab = grab; }
	bool is_mouse_grab_enabled() const { return grab; }
	int get_mouse_button_state() const { return mouse_state; }
	Point2 get_mouse_pos() const { return mouse_pos; }
	void set_mouse_mode(MouseMode mode) { mouse_mode = mode; }
	OS::MouseMode get_mouse_mode() const { return mouse_mode; }
	void set_window_title(const String &title) {}
	void set_video_mode(const VideoMode &video_mode, int screen) {}
	OS::VideoMode get_video_mode(int screen) const {
		return current_videomode;
	}
	Size2 get_window_size() const {
		return Vector2(current_videomode.width, current_videomode.height);
	}
	void get_fullscreen_mode_list(List<VideoMode> *list, int screen) const {}
	MainLoop *get_main_loop() const { return main_loop; }
	void delete_main_loop() {
		if (main_loop)
			memdelete(main_loop);
		main_loop = 0;
	}
	void set_main_loop(MainLoop *main_loop) {
		this->main_loop = main_loop;
		input->set_main_loop(main_loop);
	}
	bool can_draw() const { return true; };
	String get_name() { return "FRT"; }
	void move_window_to_foreground() {}
	void set_cursor_shape(CursorShape shape) {}
	void release_rendering_thread() { context_gl->release_current(); }
	void make_rendering_thread() { context_gl->make_current(); }
	void swap_buffers() { context_gl->swap_buffers(); }

	void fill_modifier_state(InputModifierState &state) {
		if (env->keyboard) {
			env->keyboard->get_modifier_state(state);
		} else {
			state.shift = false;
			state.control = false;
			state.alt = false;
			state.meta = false;
		}
	}
	void process_keyboard_event(int key, bool pressed) {
		InputEvent event;
		event.ID = ++event_id;
		event.type = InputEvent::KEY;
		event.device = 0;
		fill_modifier_state(event.key.mod);
		event.key.pressed = pressed;
		event.key.scancode = key;
		event.key.unicode = 0;
		event.key.echo = 0;
		input->parse_input_event(event);
	}
	void process_mouse_motion(int x, int y) {
		mouse_pos.x = x;
		mouse_pos.y = y;
		InputEvent motion_event;
		motion_event.ID = ++event_id;
		motion_event.type = InputEvent::MOUSE_MOTION;
		motion_event.device = 0;
		fill_modifier_state(motion_event.mouse_button.mod);
		motion_event.mouse_button.button_mask = mouse_state;
		motion_event.mouse_motion.x = mouse_pos.x;
		motion_event.mouse_motion.y = mouse_pos.y;
		input->set_mouse_pos(mouse_pos);
		motion_event.mouse_motion.global_x = mouse_pos.x;
		motion_event.mouse_motion.global_y = mouse_pos.y;
#if VERSION_MAJOR == 2
		motion_event.mouse_motion.speed_x = input->get_mouse_speed().x;
		motion_event.mouse_motion.speed_y = input->get_mouse_speed().y;
#else // VERSION_MAJOR == 3
		motion_event.mouse_motion.speed_x = input->get_last_mouse_speed().x;
		motion_event.mouse_motion.speed_y = input->get_last_mouse_speed().y;
#endif
		motion_event.mouse_motion.relative_x = 0;
		motion_event.mouse_motion.relative_y = 0;
		input->parse_input_event(motion_event);
	}
	void process_mouse_button(int index, bool pressed) {
		int bit = (1 << index);
		if (pressed)
			mouse_state |= bit;
		else
			mouse_state &= ~bit;
		InputEvent mouse_event;
		mouse_event.ID = ++event_id;
		mouse_event.type = InputEvent::MOUSE_BUTTON;
		mouse_event.device = 0;
		fill_modifier_state(mouse_event.mouse_button.mod);
		mouse_event.mouse_button.button_mask = mouse_state;
		mouse_event.mouse_button.x = mouse_pos.x;
		mouse_event.mouse_button.y = mouse_pos.y;
		mouse_event.mouse_button.global_x = mouse_pos.x;
		mouse_event.mouse_button.global_y = mouse_pos.y;
		mouse_event.mouse_button.button_index = index;
		mouse_event.mouse_button.pressed = pressed;
		input->parse_input_event(mouse_event);
	}
	struct KeyboardHandler : Keyboard::Handler {
		OS_FRT *instance;
		Keyboard *keyboard;
		void handle_keyboard_key(int gd_code, bool pressed) {
			InputModifierState st;
			keyboard->get_modifier_state(st);
			if (st.meta && instance->handle_meta(gd_code, pressed))
				return;
			instance->process_keyboard_event(gd_code, pressed);
		}
	} keyboard_handler;
	struct MouseHandler : Mouse::Handler {
		OS_FRT *instance;
		Video *video;
		void handle_mouse_button(Mouse::Button button, bool pressed) {
			int index;
			switch (button) {
				case Mouse::Left:
					index = 1;
					break;
				case Mouse::Middle:
					index = 3;
					break;
				case Mouse::Right:
					index = 2;
					break;
				case Mouse::WheelUp:
					index = BUTTON_WHEEL_UP;
					break;
				case Mouse::WheelDown:
					index = BUTTON_WHEEL_DOWN;
					break;
				default:
					return;
			}
			instance->process_mouse_button(index, pressed);
		}
		void handle_mouse_motion(Vec2 pos) {
			Vec2 view = video->move_pointer(pos);
			instance->process_mouse_motion(view.x, view.y);
		}
	} mouse_handler;
	bool dispatch_handle_meta(int gd_code, bool pressed) {
		// keep it simple: hard-coded order should be fine
		if (env->mouse && env->mouse->handle_meta(gd_code, pressed))
			return true;
		if (env->keyboard && env->keyboard->handle_meta(gd_code, pressed))
			return true;
		if (env->video && env->video->handle_meta(gd_code, pressed))
			return true;
		return false;
	}
	void run() {
		if (!main_loop)
			return;
		keyboard_handler.instance = this;
		keyboard_handler.keyboard = env->keyboard;
		mouse_handler.instance = this;
		mouse_handler.video = env->video;
		if (env->keyboard && !env->mouse)
			env->mouse = &mouse_virtual;
		if (env->mouse) {
			env->mouse->set_size(screen_size);
			Vec2 pos = env->mouse->get_pos();
			env->video->move_pointer(pos);
		}
		// mouse set_handler first to increase the chances of RETURN release
		if (env->mouse) {
			env->mouse->set_handler(&mouse_handler);
		}
		if (env->keyboard) {
			env->keyboard->set_handler(&keyboard_handler);
		}
		main_loop->init();
		while (!quit) {
			if (env->mouse && env->mouse->poll())
				break;
			if (env->keyboard && env->keyboard->poll())
				break;
			if (Main::iteration() == true)
				break;
		};
		main_loop->finish();
	}
	OS_FRT()
		: event_id(0), quit(false) {
#ifdef ALSA_ENABLED
		AudioDriverManagerSW::add_driver(&driver_alsa);
#endif
		if (AudioDriverManagerSW::get_driver_count() == 0)
			AudioDriverManagerSW::add_driver(&driver_dummy);
		mouse_mode = MOUSE_MODE_VISIBLE;
		mouse_state = 0;
		grab = false;
	};
	// Module
	const char *get_id() const { return "frt_os_unix"; }
	bool probe() { return true; }
	void cleanup() {}
	bool handle_meta(int gd_code, bool pressed) {
		switch (gd_code) {
		case 'Q':
			quit = true;
			break;
		default:
			return dispatch_handle_meta(gd_code, pressed);
		}
		return true;
	}
	// Runnable
	void setup_env(Environment *env) {
		this->env = env;
		assert(env->video);
		screen_size = env->video->get_screen_size();
	}
	void run_() { run(); }
	int get_exit_code_() { return get_exit_code(); }
};

FRT_REGISTER(OS_FRT)
