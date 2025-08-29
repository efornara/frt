// frt_godot.cc
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2025  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "frt.h"
#include "sdl2_adapter.h"
#include "sdl2_godot_map.h"
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
	const char *get_name() const override {
		return "SDL2";
	}
	Error init() override {
		mix_rate_ = GLOBAL_GET("audio/mix_rate");
		speaker_mode_ = SPEAKER_MODE_STEREO;
		const int latency = GLOBAL_GET("audio/output_latency");
		const int samples = closest_power_of_2(latency * mix_rate_ / 1000);
		return audio_.init(mix_rate_, samples) ? OK : ERR_CANT_OPEN;
	}
	int get_mix_rate() const override {
		return mix_rate_;
	}
	SpeakerMode get_speaker_mode() const override {
		return speaker_mode_;
	}
	Array get_device_list() override {
		Array list;
		list.push_back(default_audio_device);
		return list;
	}
	String get_device() override {
		return default_audio_device;
	}
	void set_device(String device) override {
	}
	void start() override {
		audio_.start();
	}
	void lock() override {
		audio_.lock();
	}
	void unlock() override {
		audio_.unlock();
	}
	void finish() override {
		audio_.finish();
	}
public: // SampleProducer
	void produce_samples(int n_of_frames, int32_t *frames) override {
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
	int get_video_driver_count() const override {
		return 2;
	}
	int get_current_video_driver() const override {
		return video_driver_;
	}
	const char *get_video_driver_name(int driver) const override {
		if (driver == VIDEO_DRIVER_GLES2)
			return "GLES2";
		else
			return "GLES3";
	}
	bool _check_internal_feature_support(const String &feature) override {
		if (feature == "X11" || feature == "FRT")
			return true;
		if (video_driver_ == VIDEO_DRIVER_GLES3 && feature == "etc2")
			return true;
		return feature == "mobile" || feature == "etc";
	}
	String get_config_path() const override {
		if (has_environment("XDG_CONFIG_HOME"))
			return get_environment("XDG_CONFIG_HOME");
		if (has_environment("HOME"))
			return get_environment("HOME").plus_file(".config");
		return ".";
	}
	String get_data_path() const override {
		if (has_environment("XDG_DATA_HOME"))
			return get_environment("XDG_DATA_HOME");
		if (has_environment("HOME"))
			return get_environment("HOME").plus_file(".local/share");
		return get_config_path();
	}
	String get_cache_path() const override {
		if (has_environment("XDG_CACHE_HOME"))
			return get_environment("XDG_CACHE_HOME");
		if (has_environment("HOME"))
			return get_environment("HOME").plus_file(".cache");
		return get_config_path();
	}
	Error initialize(const VideoMode &desired, int video_driver, int audio_driver) override {
		video_mode_ = desired;
		video_driver_ = video_driver;
		const GraphicsAPI api = video_driver == VIDEO_DRIVER_GLES3 ? API_OpenGL_ES3 : API_OpenGL_ES2;
		os_.init_gl(api, video_mode_.width, video_mode_.height, video_mode_.resizable, video_mode_.borderless_window, video_mode_.always_on_top);
		_set_use_vsync(video_mode_.use_vsync);
		init_video();
		init_audio(audio_driver);
		init_input();
		return OK;
	}
	void set_main_loop(MainLoop *main_loop) override {
		main_loop_ = main_loop;
		input_->set_main_loop(main_loop);
	}
	void delete_main_loop() override {
		if (main_loop_)
			memdelete(main_loop_);
		main_loop_ = 0;
	}
	void finalize() override {
		delete_main_loop();
		cleanup_input();
		cleanup_audio();
		cleanup_video();
		os_.cleanup();
	}
	Point2 get_mouse_position() const override {
		return mouse_pos_;
	}
	int get_mouse_button_state() const override {
		return mouse_state_;
	}
	void set_mouse_mode(OS::MouseMode mode) override {
		os_.set_mouse_mode(map_mouse_mode(mode));
	}
	OS::MouseMode get_mouse_mode() const override {
		return map_mouse_os_mode(os_.get_mouse_mode());
	}
	void set_window_title(const String &title) override {
		os_.set_title(title.utf8().get_data());
	}
	void set_video_mode(const VideoMode &video_mode, int screen) override {
	}
	VideoMode get_video_mode(int screen = 0) const override {
		return video_mode_;
	}
	void get_fullscreen_mode_list(List<VideoMode> *list, int screen) const override {
	}
	Size2 get_window_size() const override {
		return Size2(video_mode_.width, video_mode_.height);
	}
	void set_window_size(const Size2 size) override {
		ivec2 os_size = { (int)size.width, (int)size.height };
		os_.set_size(os_size);
		video_mode_.width = os_size.x;
		video_mode_.height = os_size.y;
	}
	Point2 get_window_position() const override {
		ivec2 pos = os_.get_pos();
		return Point2(pos.x, pos.y);
	}
	void set_window_position(const Point2 &pos) override {
		ivec2 os_pos = { (int)pos.width, (int)pos.height };
		os_.set_pos(os_pos);
	}
	void set_window_fullscreen(bool enable) override {
		os_.set_fullscreen(enable);
		video_mode_.fullscreen = enable;
	}
	bool is_window_fullscreen() const override {
		return os_.is_fullscreen();
	}
	void set_window_always_on_top(bool enable) override {
		os_.set_always_on_top(enable);
		video_mode_.always_on_top = enable;
	}
	bool is_window_always_on_top() const override {
		return os_.is_always_on_top();
	}
	void set_window_resizable(bool enable) override {
		os_.set_resizable(enable);
	}
	bool is_window_resizable() const override {
		return os_.is_resizable();
	}
	void set_window_maximized(bool enable) override {
		os_.set_maximized(enable);
	}
	bool is_window_maximized() const override {
		return os_.is_maximized();
	}
	void set_window_minimized(bool enable) override {
		os_.set_minimized(enable);
	}
	bool is_window_minimized() const override {
		return os_.is_minimized();
	}
	MainLoop *get_main_loop() const override {
		return main_loop_;
	}
	bool can_draw() const override {
		return os_.can_draw();
	}
	void set_cursor_shape(CursorShape shape) override {
	}
	void set_custom_mouse_cursor(const RES &cursor, CursorShape shape, const Vector2 &hotspot) override {
	}
	void make_rendering_thread() override {
		os_.make_current_gl();
	}
	void release_rendering_thread() override {
		os_.release_current_gl();
	}
	void swap_buffers() override {
		os_.swap_buffers_gl();
	}
	void _set_use_vsync(bool enable) override {
		os_.set_use_vsync_gl(enable);
	}
	void set_icon(const Ref<Image> &icon) override {
		if (icon.is_null())
			return;
		Ref<Image> i = icon->duplicate();
		i->convert(Image::FORMAT_RGBA8);
		PoolVector<uint8_t>::Read r = i->get_data().read();
		os_.set_icon(i->get_width(), i->get_height(), r.ptr());
	}
	Size2 get_screen_size(int screen) const override {
		ivec2 size = os_.get_screen_size();
		return Size2(size.x, size.y);
	}
	int get_screen_dpi(int screen) const override {
		return os_.get_screen_dpi();
	}
	float get_screen_refresh_rate(int screen) const override {
		return os_.get_screen_refresh_rate();
	}
public: // EventHandler
	void handle_resize_event(ivec2 size) override {
		video_mode_.width = size.x;
		video_mode_.height = size.y;
	}
	void handle_key_event(int sdl2_code, int unicode, bool pressed) override {
		int code = map_key_sdl2_code(sdl2_code);
		Ref<InputEventKey> key;
		key.instance();
		fill_modifier_state(key);
		key->set_pressed(pressed);
		key->set_scancode(code);
		key->set_physical_scancode(code); // TODO
		key->set_unicode(unicode);
		key->set_echo(false);
		input_->parse_input_event(key);
	}
	void handle_mouse_motion_event(ivec2 pos, ivec2 dpos) override {
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
	void handle_mouse_button_event(int os_button, bool pressed, bool doubleclick) override {
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
	void handle_js_status_event(int id, bool connected, const char *name, const char *guid) override {
		input_->joy_connection_changed(id, connected, name, guid);
	}
	void handle_js_button_event(int id, int button, bool pressed) override {
		input_->joy_button(id, button, pressed ? 1 : 0);
	}
	void handle_js_axis_event(int id, int axis, float value) override {
		input_->joy_axis(id, axis, value);
	}
	void handle_js_hat_event(int id, int os_mask) override {
		int mask = map_hat_os_mask(os_mask);
		input_->joy_hat(id, mask);
	}
	void handle_js_vibra_event(int id, uint64_t timestamp) override {
		uint64_t input_timestamp = input_->get_joy_vibration_timestamp(id);
		if (input_timestamp > timestamp) {
			Vector2 strength = input_->get_joy_vibration_strength(id);
			float duration = input_->get_joy_vibration_duration(id);
			os_.js_vibra(id, strength.x, strength.y, duration, input_timestamp);
		}
	}
	void handle_quit_event() override {
		quit_ = true;
	}
	void handle_flush_events() override {
		input_->flush_buffered_events();
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
