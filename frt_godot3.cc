// frt_godot3.cc
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2022  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "frt.h"
#include "sdl2_adapter.h"
#include "sdl2_godot_mapping.h"
#include "drivers/gles3/rasterizer_gles3.h"
#define FRT_DL_SKIP
#include "drivers/gles2/rasterizer_gles2.h"

#include "core/print_string.h"
#include "drivers/unix/os_unix.h"
#pragma GCC diagnostic ignored "-Wvolatile"
#include "servers/audio_server.h"
#pragma GCC diagnostic pop
#include "servers/visual_server.h"
#include "servers/visual/visual_server_wrap_mt.h"
#include "servers/visual/rasterizer.h"
#include "servers/visual/visual_server_raster.h"
#include "main/main.h"

namespace frt {

static const char *default_audio_device = "default"; // TODO

class AudioDriverSDL2 : public AudioDriver, public SampleProducer {
private:
	Audio audio_;
	int mix_rate_;
	SpeakerMode speaker_mode_;
public:
	AudioDriverSDL2() : audio_(this) {
	}
public: // AudioDriverSW
	const char *get_name() const FRT_OVERRIDE {
		return "SDL2";
	}
	Error init() FRT_OVERRIDE {
		mix_rate_ = GLOBAL_GET("audio/mix_rate");
		speaker_mode_ = SPEAKER_MODE_STEREO;
		const int latency = GLOBAL_GET("audio/output_latency");
		const int samples = closest_power_of_2(latency * mix_rate_ / 1000);
		return audio_.init(mix_rate_, samples) ? OK : ERR_CANT_OPEN;
	}
	int get_mix_rate() const FRT_OVERRIDE {
		return mix_rate_;
	}
	SpeakerMode get_speaker_mode() const FRT_OVERRIDE {
		return speaker_mode_;
	}
	Array get_device_list() FRT_OVERRIDE {
		Array list;
		list.push_back(default_audio_device);
		return list;
	}
	String get_device() FRT_OVERRIDE {
		return default_audio_device;
	}
	void set_device(String device) FRT_OVERRIDE {
	}
	void start() FRT_OVERRIDE {
		audio_.start();
	}
	void lock() FRT_OVERRIDE {
		audio_.lock();
	}
	void unlock() FRT_OVERRIDE {
		audio_.unlock();
	}
	void finish() FRT_OVERRIDE {
		audio_.finish();
	}
public: // SampleProducer
	void produce_samples(int n_of_frames, int32_t *frames) FRT_OVERRIDE {
		audio_server_process(n_of_frames, frames);
	}
};

class Godot3_OS : public OS_Unix, public EventHandler {
private:
	enum {
		VIDEO_DRIVER_GLES2,
		VIDEO_DRIVER_GLES3
	};
	MainLoop *main_loop_;
	VideoMode video_mode_;
	bool quit_;
	OS_FRT os_;
	int video_driver_;
	VisualServer *visual_server_;
	void init_video() {
		if (video_driver_ == VIDEO_DRIVER_GLES2) {
			frt_resolve_symbols_gles2(get_proc_address);
			RasterizerGLES2::register_config();
			RasterizerGLES2::make_current();
		} else {
			frt_resolve_symbols_gles3(get_proc_address);
			RasterizerGLES3::register_config();
			RasterizerGLES3::make_current();
		}
		visual_server_ = memnew(VisualServerRaster);
		visual_server_->init();
	}
	void cleanup_video() {
		visual_server_->finish();
		memdelete(visual_server_);
	}
	AudioDriverSDL2 audio_driver_;
	void init_audio(int id) {
		AudioDriverManager::initialize(id);
	}
	void cleanup_audio() {
	}
	InputDefault *input_;
	Point2 mouse_pos_;
	int mouse_state_;
	void init_input() {
		input_ = memnew(InputDefault);
		mouse_pos_ = Point2(-1, -1);
		mouse_state_ = 0;
	}
	void cleanup_input() {
		memdelete(input_);
	}
	void fill_modifier_state(Ref<InputEventWithModifiers> st) {
		const InputModifierState *os_st = os_.get_modifier_state();
		st->set_shift(os_st->shift);
		st->set_alt(os_st->alt);
		st->set_control(os_st->control);
		st->set_metakey(os_st->meta);
	}
public:
	Godot3_OS() : os_(this) {
		AudioDriverManager::add_driver(&audio_driver_);
		main_loop_ = 0;
		quit_ = false;
	}
	void run() {
		if (main_loop_) {
			main_loop_->init();
			while (!quit_ && !Main::iteration())
				os_.dispatch_events();
			main_loop_->finish();
		}
	}
public: // OS
	int get_video_driver_count() const FRT_OVERRIDE {
		return 2;
	}
#if FRT_GODOT_VERSION >= 30100
	int get_current_video_driver() const FRT_OVERRIDE {
		return video_driver_;
	}
#endif
	const char *get_video_driver_name(int driver) const FRT_OVERRIDE {
		return driver == VIDEO_DRIVER_GLES3 ? "GLES3" : "GLES2";
	}
	bool _check_internal_feature_support(const String &feature) FRT_OVERRIDE {
		if (video_driver_ == VIDEO_DRIVER_GLES3 && feature == "etc2")
			return true;
		return feature == "mobile" || feature == "etc";
	}
	String get_config_path() const FRT_OVERRIDE {
		if (has_environment("XDG_CONFIG_HOME"))
			return get_environment("XDG_CONFIG_HOME");
		if (has_environment("HOME"))
			return get_environment("HOME").plus_file(".config");
		return ".";
	}
	String get_data_path() const FRT_OVERRIDE {
		if (has_environment("XDG_DATA_HOME"))
			return get_environment("XDG_DATA_HOME");
		if (has_environment("HOME"))
			return get_environment("HOME").plus_file(".local/share");
		return get_config_path();
	}
	String get_cache_path() const FRT_OVERRIDE {
		if (has_environment("XDG_CACHE_HOME"))
			return get_environment("XDG_CACHE_HOME");
		if (has_environment("HOME"))
			return get_environment("HOME").plus_file(".cache");
		return get_config_path();
	}
	Error initialize(const VideoMode &desired, int video_driver, int audio_driver) FRT_OVERRIDE {
		video_mode_ = desired;
		video_driver_ = video_driver;
		const GraphicsAPI api = video_driver == VIDEO_DRIVER_GLES2 ? API_OpenGL_ES2 : API_OpenGL_ES3;
		os_.init(api, video_mode_.width, video_mode_.height, video_mode_.resizable, video_mode_.borderless_window, video_mode_.always_on_top);
		_set_use_vsync(video_mode_.use_vsync);
		init_video();
		init_audio(audio_driver);
		init_input();
#if FRT_GODOT_VERSION < 30300
		_ensure_user_data_dir();
#endif
		return OK;
	}
	void set_main_loop(MainLoop *main_loop) FRT_OVERRIDE {
		main_loop_ = main_loop;
		input_->set_main_loop(main_loop);
	}
	void delete_main_loop() FRT_OVERRIDE {
		if (main_loop_)
			memdelete(main_loop_);
		main_loop_ = 0;
	}
	void finalize() FRT_OVERRIDE {
		delete_main_loop();
		cleanup_input();
		cleanup_audio();
		cleanup_video();
		os_.cleanup();
	}
	Point2 get_mouse_position() const FRT_OVERRIDE {
		return mouse_pos_;
	}
	int get_mouse_button_state() const FRT_OVERRIDE {
		return mouse_state_;
	}
	void set_mouse_mode(OS::MouseMode mode) FRT_OVERRIDE {
		os_.set_mouse_mode(map_mouse_mode(mode));
	}
	OS::MouseMode get_mouse_mode() const FRT_OVERRIDE {
		return map_mouse_os_mode(os_.get_mouse_mode());
	}
	void set_window_title(const String &title) FRT_OVERRIDE {
		os_.set_title(title.utf8().get_data());
	}
	void set_video_mode(const VideoMode &video_mode, int screen) FRT_OVERRIDE {
	}
	VideoMode get_video_mode(int screen = 0) const FRT_OVERRIDE {
		return video_mode_;
	}
	void get_fullscreen_mode_list(List<VideoMode> *list, int screen) const FRT_OVERRIDE {
	}
	Size2 get_window_size() const FRT_OVERRIDE {
		return Size2(video_mode_.width, video_mode_.height);
	}
	void set_window_size(const Size2 size) FRT_OVERRIDE {
		ivec2 os_size = { (int)size.width, (int)size.height };
		os_.set_size(os_size);
		video_mode_.width = os_size.x;
		video_mode_.height = os_size.y;
	}
	Point2 get_window_position() const FRT_OVERRIDE {
		ivec2 pos = os_.get_pos();
		return Point2(pos.x, pos.y);
	}
	void set_window_position(const Point2 &pos) FRT_OVERRIDE {
		ivec2 os_pos = { (int)pos.width, (int)pos.height };
		os_.set_pos(os_pos);
	}
	void set_window_fullscreen(bool enable) FRT_OVERRIDE {
		os_.set_fullscreen(enable);
		video_mode_.fullscreen = enable;
	}
	bool is_window_fullscreen() const FRT_OVERRIDE {
		return os_.is_fullscreen();
	}
	void set_window_always_on_top(bool enable) FRT_OVERRIDE {
		os_.set_always_on_top(enable);
		video_mode_.always_on_top = enable;
	}
	bool is_window_always_on_top() const FRT_OVERRIDE {
		return os_.is_always_on_top();
	}
	void set_window_resizable(bool enable) FRT_OVERRIDE {
		os_.set_resizable(enable);
	}
	bool is_window_resizable() const FRT_OVERRIDE {
		return os_.is_resizable();
	}
	void set_window_maximized(bool enable) FRT_OVERRIDE {
		os_.set_maximized(enable);
	}
	bool is_window_maximized() const FRT_OVERRIDE {
		return os_.is_maximized();
	}
	void set_window_minimized(bool enable) FRT_OVERRIDE {
		os_.set_minimized(enable);
	}
	bool is_window_minimized() const FRT_OVERRIDE {
		return os_.is_minimized();
	}
	MainLoop *get_main_loop() const FRT_OVERRIDE {
		return main_loop_;
	}
	bool can_draw() const FRT_OVERRIDE {
		return true;
	}
	void set_cursor_shape(CursorShape shape) FRT_OVERRIDE {
	}
	void set_custom_mouse_cursor(const RES &cursor, CursorShape shape, const Vector2 &hotspot) FRT_OVERRIDE {
	}
	void make_rendering_thread() FRT_OVERRIDE {
		os_.make_current();
	}
	void release_rendering_thread() FRT_OVERRIDE {
		os_.release_current();
	}
	void swap_buffers() FRT_OVERRIDE {
		os_.swap_buffers();
	}
	void _set_use_vsync(bool enable) FRT_OVERRIDE {
		os_.set_use_vsync(enable);
	}
public: // EventHandler
	void handle_resize_event(ivec2 size) FRT_OVERRIDE {
		video_mode_.width = size.x;
		video_mode_.height = size.y;
	}
	void handle_key_event(int sdl2_code, int unicode, bool pressed) FRT_OVERRIDE {
		int code = map_key_sdl2_code(sdl2_code);
		Ref<InputEventKey> key;
		key.instance();
		fill_modifier_state(key);
		key->set_pressed(pressed);
		key->set_scancode(code);
#if FRT_GODOT_VERSION >= 30400
		key->set_physical_scancode(code); // TODO
#endif
		key->set_unicode(unicode);
		key->set_echo(false);
		input_->parse_input_event(key);
	}
	void handle_mouse_motion_event(ivec2 pos, ivec2 dpos) FRT_OVERRIDE {
		mouse_pos_.x = pos.x;
		mouse_pos_.y = pos.y;
		Ref<InputEventMouseMotion> mouse_motion;
		mouse_motion.instance();
		fill_modifier_state(mouse_motion);
		Point2i posi(pos.x, pos.y);
		mouse_motion->set_button_mask(mouse_state_);
		mouse_motion->set_position(posi);
		mouse_motion->set_global_position(posi);
		input_->set_mouse_position(posi);
		mouse_motion->set_speed(input_->get_last_mouse_speed());
		Point2i reli(dpos.x, dpos.y);
		mouse_motion->set_relative(reli);
		input_->parse_input_event(mouse_motion);
	}
	void handle_mouse_button_event(int os_button, bool pressed, bool doubleclick) FRT_OVERRIDE {
		int button = map_mouse_os_button(os_button);
		int bit = (1 << (button - 1));
		if (pressed)
			mouse_state_ |= bit;
		else
			mouse_state_ &= ~bit;
		Ref<InputEventMouseButton> mouse_button;
		mouse_button.instance();
		fill_modifier_state(mouse_button);
		Point2i posi(mouse_pos_.x, mouse_pos_.y);
		mouse_button->set_position(posi);
		mouse_button->set_global_position(posi);
		mouse_button->set_button_index(button);
		mouse_button->set_button_mask(mouse_state_);
		mouse_button->set_doubleclick(doubleclick);
		mouse_button->set_pressed(pressed);
		input_->parse_input_event(mouse_button);
	}
	void handle_js_status_event(int id, bool connected, const char *name, const char *guid) FRT_OVERRIDE {
		input_->joy_connection_changed(id, connected, name, guid);
	}
	void handle_js_button_event(int id, int button, bool pressed) FRT_OVERRIDE {
		input_->joy_button(id, button, pressed ? 1 : 0);
	}
	void handle_js_axis_event(int id, int axis, float value) FRT_OVERRIDE {
#if FRT_GODOT_VERSION >= 30500
		input_->set_joy_axis(id, axis, value);
#else
		InputDefault::JoyAxis v = { -1, value };
		input_->joy_axis(id, axis, v);
#endif
	}
	void handle_js_hat_event(int id, int os_mask) FRT_OVERRIDE {
		int mask = map_hat_os_mask(os_mask);
		input_->joy_hat(id, mask);
	}
	void handle_quit_event() FRT_OVERRIDE {
		quit_ = true;
	}
};

} // namespace frt

#include "frt_lib.h"

extern "C" int frt_godot_main(int argc, char *argv[]) {
	frt::Godot3_OS os;
	Error err = Main::setup(argv[0], argc - 1, &argv[1]);
	if (err != OK)
		return 255;
	if (Main::start())
		os.run();
	Main::cleanup();
	return os.get_exit_code();
}
