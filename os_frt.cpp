// os_frt.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>

#include "core/version.h"
#include "core/os/os.h"
#include "core/os/input.h"
#include "core/os/file_access.h"
#include "drivers/unix/os_unix.h"
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
#include "main/main.h"
#include "main/input_default.h"
#include "main/performance.h"
#include "core/print_string.h"

#define VIDEO_DRIVER_GLES2 0
#define VIDEO_DRIVER_GLES3 1

#if VERSION_MAJOR == 2

#define VIDEO_DRIVER_COUNT 1
#include "import/joystick_linux.h"
#include "servers/visual/visual_server_wrap_mt.h"
#include "servers/audio/audio_server_sw.h"
#include "servers/audio/sample_manager_sw.h"
#include "servers/spatial_sound/spatial_sound_server_sw.h"
#include "servers/spatial_sound_2d/spatial_sound_2d_server_sw.h"
#include "drivers/gles2/rasterizer_gles2.h"

static int event_id = 0;

class InputModifierStateSetter {
private:
	InputModifierState &mod;

public:
	InputModifierStateSetter(InputModifierState &m)
		: mod(m) {}
	void set_shift(bool v) { mod.shift = v; }
	void set_control(bool v) { mod.control = v; }
	void set_alt(bool v) { mod.alt = v; }
	void set_metakey(bool v) { mod.meta = v; }
};

class InputEventKeySetter : public InputModifierStateSetter {
private:
	InputEvent &event;

public:
	InputEventKeySetter(InputEvent &ev)
		: InputModifierStateSetter(ev.key.mod), event(ev) {
		event.type = InputEvent::KEY;
		event.device = 0;
	}
	void set_pressed(bool v) { event.key.pressed = v; }
	void set_scancode(uint32_t v) { event.key.scancode = v; }
	void set_unicode(uint32_t v) { event.key.unicode = v; }
	void set_echo(bool v) { event.key.echo = v; }
};

class InputEventMouseMotionSetter : public InputModifierStateSetter {
private:
	InputEvent &event;

public:
	InputEventMouseMotionSetter(InputEvent &ev)
		: InputModifierStateSetter(ev.mouse_motion.mod), event(ev) {
		event.type = InputEvent::MOUSE_MOTION;
		event.device = 0;
	}
	void set_button_mask(int v) { event.mouse_motion.button_mask = v; }
	void set_position(const Vector2 &v) {
		event.mouse_motion.x = v.x;
		event.mouse_motion.y = v.y;
	}
	void set_global_position(const Vector2 &v) {
		event.mouse_motion.global_x = v.x;
		event.mouse_motion.global_y = v.y;
	}
	void set_speed(const Vector2 &v) {
		event.mouse_motion.speed_x = v.x;
		event.mouse_motion.speed_y = v.y;
	}
	void set_relative(const Vector2 &v) {
		event.mouse_motion.relative_x = v.x;
		event.mouse_motion.relative_y = v.y;
	}
};

class InputEventMouseButtonSetter : public InputModifierStateSetter {
private:
	InputEvent &event;

public:
	InputEventMouseButtonSetter(InputEvent &ev)
		: InputModifierStateSetter(ev.mouse_button.mod), event(ev) {
		event.type = InputEvent::MOUSE_BUTTON;
		event.device = 0;
	}
	void set_button_mask(int v) { event.mouse_button.button_mask = v; }
	void set_position(const Vector2 &v) {
		event.mouse_button.x = v.x;
		event.mouse_button.y = v.y;
	}
	void set_global_position(const Vector2 &v) {
		event.mouse_button.global_x = v.x;
		event.mouse_button.global_y = v.y;
	}
	void set_pressed(bool v) { event.mouse_button.pressed = v; }
	void set_button_index(int v) { event.mouse_button.button_index = v; }
	void set_doubleclick(bool v) { event.mouse_button.doubleclick = v; }
};

class InputModifierRef {
private:
	InputModifierStateSetter setter;

public:
	InputModifierRef(InputModifierStateSetter &s)
		: setter(s) {}
	InputModifierStateSetter *operator->() { return &setter; }
};

template <typename T>
class InputEventRef {
private:
	InputEvent event;
	T setter;
	InputModifierRef mod;

public:
	InputEventRef()
		: setter(event), mod(setter) { event.ID = ++event_id; };
	void instance() {}
	operator InputEvent() { return event; }
	operator InputModifierRef() { return mod; }
	T *operator->() { return &setter; }
};

#define INPUT_MODIFIER_REF InputModifierRef
#define INPUT_EVENT_REF(t) InputEventRef<t##Setter>

#include "core/globals.h"
#define PROJECT_SETTINGS \
	Globals *project_settings = Globals::get_singleton();

#elif VERSION_MAJOR == 3

#define VIDEO_DRIVER_COUNT 2
#include "import/joypad_linux.h"
#include "drivers/gles3/rasterizer_gles3.h"
#define FRT_DL_SKIP
#include "drivers/gles2/rasterizer_gles2.h"
typedef AudioDriverManager AudioDriverManagerSW;
typedef AudioDriver AudioDriverSW;
#define set_mouse_pos set_mouse_position
#define get_mouse_pos get_mouse_position
#define get_mouse_speed get_last_mouse_speed
#define has has_setting
#define FRT_MOCK_GODOT_INPUT_MODIFIER_STATE
#define INPUT_MODIFIER_REF Ref<InputEventWithModifiers>
#define INPUT_EVENT_REF(t) Ref<t>
#define joystick_linux JoypadLinux

#include "core/project_settings.h"
#define PROJECT_SETTINGS \
	ProjectSettings *project_settings = ProjectSettings::get_singleton();

#else
#error "unhandled godot version"
#endif

#include "frt.h"
#include "bits/mouse_virtual.h"

using namespace frt;

namespace frt {
extern const char *perfmon_filename;
extern const char *extract_resource_name;
}

static class PerfMon {
private:
	FILE *f;
	char sep;
	void init_separator() {
		char s[32];
		snprintf(s, sizeof(s), "%f", 2.5);
		s[sizeof(s) - 1] = '\0';
		sep = strchr(s, ',') ? ';' : ',';
	}

public:
	PerfMon()
		: f(0) {}
	~PerfMon() { cleanup(); }
	bool is_valid() { return f; }
	void init() {
		if (!perfmon_filename)
			return;
		if (!(f = fopen(perfmon_filename, "w")))
			return;
		init_separator();
		Performance *p = Performance::get_singleton();
		fprintf(f, "time/ticks_ms");
		for (int i = 0; i < Performance::MONITOR_MAX; i++)
			fprintf(f, "%c%s", sep,
					p->get_monitor_name(Performance::Monitor(i)).ascii().get_data());
		fprintf(f, "\n");
	}
	void iteration(uint32_t t) {
		if (!f)
			return;
		Performance *p = Performance::get_singleton();
		fprintf(f, "%u", (unsigned int)t);
		for (int i = 0; i < Performance::MONITOR_MAX; i++)
			fprintf(f, "%c%f", sep,
					p->get_monitor(Performance::Monitor(i)));
		fprintf(f, "\n");
	}
	void cleanup() {
		if (f)
			fclose(f);
		f = 0;
	}
} perfmon;

class OS_FRT : public OS_Unix, public Runnable {
private:
	App *app;
	Param *disable_meta_keys_param;
	Env *env;
	Vec2 screen_size;
	ContextGL *context_gl;
	VisualServer *visual_server;
	VideoMode current_videomode;
	int current_video_driver;
	List<String> args;
	MainLoop *main_loop;
#ifdef PULSEAUDIO_ENABLED
	AudioDriverPulseAudio driver_pulseaudio;
#endif
#ifdef ALSA_ENABLED
	AudioDriverALSA driver_alsa;
#endif
	AudioDriverDummy driver_dummy;
	int audio_driver_index;
	Point2 mouse_pos;
	Point2 last_mouse_pos;
	bool last_mouse_pos_valid;
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
#ifdef JOYDEV_ENABLED
	joystick_linux *joystick;
#endif
	MouseVirtual mouse_virtual;
	uint64_t last_click;

public:
	int get_video_driver_count() const { return VIDEO_DRIVER_COUNT; }
	int get_current_video_driver() const { return current_video_driver; }
	const char *get_video_driver_name(int driver) const {
		if (driver == VIDEO_DRIVER_GLES3)
			return "GLES3";
		else
			return "GLES2";
	}
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
	bool _check_internal_feature_support(const String &feature) {
		if (current_video_driver == VIDEO_DRIVER_GLES3 && feature == "etc2")
			return true;
		return feature == "pc" || feature == "etc";
	}
#if VERSION_MAJOR >= 3
	String get_config_path() const {
		if (has_environment("XDG_CONFIG_HOME"))
			return get_environment("XDG_CONFIG_HOME");
		if (has_environment("HOME"))
			return get_environment("HOME").plus_file(".config");
		return ".";
	}
	String get_data_path() const {
		if (has_environment("XDG_DATA_HOME"))
			return get_environment("XDG_DATA_HOME");
		if (has_environment("HOME"))
			return get_environment("HOME").plus_file(".local/share");
		return get_config_path();
	}
	String get_cache_path() const {
		if (has_environment("XDG_CACHE_HOME"))
			return get_environment("XDG_CACHE_HOME");
		if (has_environment("HOME"))
			return get_environment("HOME").plus_file(".cache");
		return get_config_path();
	}
#endif
	void extract_resource_fatal(const char *msg) {
		fatal("failed extracting resource '%s': %s.",
			   extract_resource_name, msg);
	}
	void extract_resource_if_requested() {
		const char *s = extract_resource_name;
		if (!s)
			return;
		if (!isalnum(*s))
			extract_resource_fatal("invalid name");
		for (int i = 0; s[i]; i++)
			if (!isalnum(*s) && !strchr(".-_", *s))
				extract_resource_fatal("invalid name");
		String name = "res://frt/";
		name += s;
		if (!FileAccess::exists(name))
			extract_resource_fatal("not found");
		FileAccess *in = FileAccess::open(name, FileAccess::READ);
		if (!in)
			extract_resource_fatal("failed opening resource");
		int len = in->get_len();
		uint8_t *buf = new uint8_t[len]; // memory "leak" is fine here
		if (!buf)
			extract_resource_fatal("failed allocating memory");
		int n = in->get_buffer(buf, len);
		if (n != len)
			extract_resource_fatal("failed reading resource");
		in->close();
		FileAccess *out = FileAccess::open(s, FileAccess::WRITE);
		if (!out)
			extract_resource_fatal("failed opening output file");
		out->store_buffer(buf, len);
		out->close();
		exit(0);
	}
	void get_project_frt_params() {
		PROJECT_SETTINGS
		String name;
		const int n = app->get_n_of_params();
		for (int i = 0; i < n; i++) {
			Param *p = app->get_param(i);
			if (p->source == Param::CommandLine)
				continue;
			name = "frt/";
			name += p->name;
			if (!project_settings->has(name))
				continue;
			p->source = Param::ProjectSettings;
			Value &v = p->value;
			switch (v.t) {
				case Value::Bool:
					v.u.b = bool(project_settings->get(name));
					break;
				case Value::Int:
					v.u.i = int(project_settings->get(name));
					break;
				case Value::Float:
					v.u.f = float(project_settings->get(name));
					break;
				case Value::String: {
					String s = String(project_settings->get(name));
					v.u.s = strdup(s.ascii());
					// TODO: keep track and dealloc string copy
				} break;
			}
		}
	}
	bool disable_meta_keys() {
		return disable_meta_keys_param->value.u.b;
	}
#if VERSION_MAJOR == 2
	void
#else
	Error
#endif
	initialize(const VideoMode &desired, int video_driver, int audio_driver) {
		get_project_frt_params();
		extract_resource_if_requested();
		args = OS::get_singleton()->get_cmdline_args();
		current_videomode = desired;
		main_loop = 0;
		Vec2 view(current_videomode.width, current_videomode.height);
		int gl_version = video_driver == VIDEO_DRIVER_GLES3 ? 3 : 2;
		context_gl = env->video->create_the_gl_context(gl_version, view);
		context_gl->initialize();
#if VERSION_MAJOR == 2
		rasterizer = memnew(RasterizerGLES2);
		visual_server = memnew(VisualServerRaster(rasterizer));
		if (get_render_thread_mode() != RENDER_THREAD_UNSAFE)
			visual_server = memnew(VisualServerWrapMT(
					visual_server, get_render_thread_mode() == RENDER_SEPARATE_THREAD));
#else
		if (video_driver == VIDEO_DRIVER_GLES3) {
			RasterizerGLES3::register_config();
			RasterizerGLES3::make_current();
			current_video_driver = VIDEO_DRIVER_GLES3;
		} else {
			RasterizerGLES2::register_config();
			RasterizerGLES2::make_current();
			current_video_driver = VIDEO_DRIVER_GLES2;
		}
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
		ERR_FAIL_COND(!visual_server);
#else
		if (!visual_server)
			return FAILED;
#endif
		visual_server->init();
		physics_server = memnew(PhysicsServerSW);
		physics_server->init();
		physics_2d_server = memnew(Physics2DServerSW);
		physics_2d_server->init();
		input = memnew(InputDefault);
#ifdef JOYDEV_ENABLED
		joystick = memnew(joystick_linux(input));
#endif
		last_click = 0;
#if VERSION_MAJOR == 2
		_ensure_data_dir();
#else
		_ensure_user_data_dir();
		return OK;
#endif
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
#else // VERSION_MAJOR == 3
		for (int i = 0; i < get_audio_driver_count(); i++)
			AudioDriverManager::get_driver(i)->finish();
#endif
		memdelete(visual_server);
#if VERSION_MAJOR == 2
		memdelete(rasterizer);
#endif
		physics_server->finish();
		memdelete(physics_server);
		physics_2d_server->finish();
		memdelete(physics_2d_server);
#ifdef JOYDEV_ENABLED
		memdelete(joystick);
#endif
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
	void set_window_title(const String &title) {
		env->video->set_title(title.utf8().get_data());
	}
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
	String get_name()
#if (VERSION_MAJOR == 3) && (VERSION_MINOR >= 2)
	const
#endif
	{ return "FRT"; }
	void move_window_to_foreground() {}
	void set_cursor_shape(CursorShape shape) {}
	void set_custom_mouse_cursor(const RES&, OS::CursorShape, const Vector2&) {}
	void release_rendering_thread() { context_gl->release_current(); }
	void make_rendering_thread() { context_gl->make_current(); }
	void swap_buffers() { context_gl->swap_buffers(); }
	uint32_t unicode_fallback(int gd_code, bool pressed,
							  const InputModifierState &st) {
		if (st.alt || st.control || st.meta || !pressed)
			return 0;
		if (gd_code >= 'A' && gd_code <= 'Z')
			return st.shift ? gd_code : gd_code + ('a' - 'A');
		if (gd_code >= ' ' && gd_code <= '~')
			return gd_code;
		return 0;
	}
	void get_key_modifier_state(INPUT_MODIFIER_REF mod,
								InputModifierState *st = 0) {
		InputModifierState state;
		if (env->keyboard) {
			env->keyboard->get_modifier_state(state);
		} else {
			state.shift = false;
			state.control = false;
			state.alt = false;
			state.meta = false;
		}
		mod->set_shift(state.shift);
		mod->set_control(state.control);
		mod->set_alt(state.alt);
		mod->set_metakey(state.meta);
		if (st)
			*st = state;
	}
	void process_keyboard_event(int key, bool pressed, uint32_t unicode, bool echo) {
		INPUT_EVENT_REF(InputEventKey) k;
		k.instance();
		InputModifierState st;
		get_key_modifier_state(k, &st);
		k->set_pressed(pressed);
		k->set_scancode(key);
		if (unicode)
			k->set_unicode(unicode);
		else
			k->set_unicode(unicode_fallback(key, pressed, st));
		k->set_echo(echo);
		input->parse_input_event(k);
	}
	void process_mouse_motion(int x, int y) {
		mouse_pos.x = x;
		mouse_pos.y = y;
		if (!last_mouse_pos_valid) {
			last_mouse_pos = mouse_pos;
			last_mouse_pos_valid = true;
		}
		Vector2 rel = mouse_pos - last_mouse_pos;
		last_mouse_pos = mouse_pos;
		INPUT_EVENT_REF(InputEventMouseMotion) mm;
		mm.instance();
		get_key_modifier_state(mm);
		mm->set_button_mask(mouse_state);
		mm->set_position(mouse_pos);
		mm->set_global_position(mouse_pos);
		mm->set_speed(input->get_mouse_speed());
		mm->set_relative(rel);
		input->set_mouse_pos(mouse_pos);
		input->parse_input_event(mm);
	}
	void process_mouse_button(int index, bool pressed) {
		int bit = (1 << (index - 1));
		if (pressed)
			mouse_state |= bit;
		else
			mouse_state &= ~bit;
		INPUT_EVENT_REF(InputEventMouseButton) mb;
		mb.instance();
		get_key_modifier_state(mb);
		mb->set_button_mask(mouse_state);
		mb->set_position(mouse_pos);
		mb->set_global_position(mouse_pos);
		mb->set_button_index(index);
		mb->set_pressed(pressed);
		if (index == 1 && pressed) {
			const uint64_t t = get_ticks_usec();
			const uint64_t dt = t - last_click;
			if (dt < 300000) {
				last_click = 0;
				mb->set_doubleclick(true);
			} else {
				last_click = t;
			}
		}
		input->parse_input_event(mb);
	}
	struct KeyboardHandler : Keyboard::Handler {
		OS_FRT *instance;
		Keyboard *keyboard;
		void handle_keyboard_key(int gd_code, bool pressed, uint32_t unicode, bool echo) {
			if (!instance->disable_meta_keys()) {
				InputModifierState st;
				keyboard->get_modifier_state(st);
				if (st.meta && instance->handle_meta(gd_code, pressed))
					return;
			}
			instance->process_keyboard_event(gd_code, pressed, unicode, echo);
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
		disable_meta_keys_param = app->get_param("disable_meta_keys");
		keyboard_handler.instance = this;
		keyboard_handler.keyboard = env->keyboard;
		mouse_handler.instance = this;
		mouse_handler.video = env->video;
		if (env->keyboard && !env->mouse) {
			mouse_virtual.probe();
			env->mouse = &mouse_virtual;
		}
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
		perfmon.init();
		while (app->is_running()) {
			app->dispatch_events();
#ifdef JOYDEV_ENABLED
#if VERSION_MAJOR == 2
			event_id = joystick->process_joysticks(event_id);
#else
			joystick->process_joypads();
#endif
#endif
			if (Main::iteration() == true)
				break;
			if (perfmon.is_valid())
				perfmon.iteration(get_ticks_msec());
		};
		perfmon.cleanup();
		main_loop->finish();
	}

	bool is_joy_known(int p_device) {
		return input->is_joy_mapped(p_device);
	}

	String get_joy_guid(int p_device) const {
		return input->get_joy_guid_remapped(p_device);
	}

	OS_FRT() {
		current_video_driver = VIDEO_DRIVER_GLES2;
#ifdef PULSEAUDIO_ENABLED
		AudioDriverManagerSW::add_driver(&driver_pulseaudio);
#endif
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
				if (env->video->provides_quit())
					return false;
				else
					app->quit();
				break;
			default:
				return dispatch_handle_meta(gd_code, pressed);
		}
		return true;
	}
	// Runnable
	void setup_env(Env *env) {
		app = App::instance();
		this->env = env;
		if (!env->video)
			fatal("no video module available.");
		screen_size = env->video->get_screen_size();
	}
	void run_() { run(); }
	int get_exit_code_() { return get_exit_code(); }
};

FRT_REGISTER(OS_FRT)
