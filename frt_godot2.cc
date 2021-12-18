// frt_godot2.cc
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2021  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "frt.h"

#include "sdl2_adapter.h"

#include "os/os.h"
#include "os/input.h"
#include "os/keyboard.h"
#include "main/input_default.h"

#include "sdl2_godot_mapping.h"

#include "drivers/unix/os_unix.h"
#include "drivers/gl_context/context_gl.h"
#include "servers/visual_server.h"
#include "servers/visual/visual_server_wrap_mt.h"
#include "servers/visual/rasterizer.h"
#include "servers/physics_server.h"
#include "servers/audio/audio_server_sw.h"
#include "servers/audio/sample_manager_sw.h"
#include "servers/spatial_sound/spatial_sound_server_sw.h"
#include "servers/spatial_sound_2d/spatial_sound_2d_server_sw.h"
#include "servers/physics/physics_server_sw.h"
#include "servers/physics_2d/physics_2d_server_sw.h"
#include "servers/physics_2d/physics_2d_server_wrap_mt.h"
#include "servers/visual/visual_server_raster.h"
#include "drivers/gles2/rasterizer_gles2.h"
#include "drivers/pulseaudio/audio_driver_pulseaudio.h"
#include "main/main.h"
#include "print_string.h"

namespace frt {

class AudioDriverSDL2 : public AudioDriverSW, public SampleProducer {
private:
	Audio audio_;
	int mix_rate_;
	OutputFormat output_format_;
public:
	AudioDriverSDL2() : audio_(this) {
	}
public: // AudioDriverSW
	const char *get_name() const {
		return "SDL2";
	}
	Error init() {
		mix_rate_ = GLOBAL_DEF("audio/mix_rate", 44100);
		output_format_ = OUTPUT_STEREO;
		const int latency = GLOBAL_DEF("audio/output_latency", 25);
		const int samples = closest_power_of_2(latency * mix_rate_ / 1000);
		return audio_.init(mix_rate_, samples) ? OK : ERR_CANT_OPEN;
	}
	int get_mix_rate() const {
		return mix_rate_;
	}
	OutputFormat get_output_format() const {
		return output_format_;
	}
	void start() {
		audio_.start();
	}
	void lock() {
		audio_.lock();
	}
	void unlock() {
		audio_.unlock();
	}
	void finish() {
		audio_.finish();
	}
public: // SampleProducer
	void produce_samples(int n_of_frames, int32_t *frames) {
		audio_server_process(n_of_frames, frames);
	}
};

class Godot2_OS : public OS_Unix, public EventHandler {
private:
	MainLoop *main_loop_;
	VideoMode video_mode_;
	bool quit_;
	OS_FRT os_;
	RasterizerGLES2 *rasterizer_;
	VisualServer *visual_server_;
	int event_id_;
	void init_video() {
		rasterizer_ = memnew(RasterizerGLES2);
		visual_server_ = memnew(VisualServerRaster(rasterizer_));
		visual_server_->init();
	}
	void cleanup_video() {
		visual_server_->finish();
		memdelete(visual_server_);
		memdelete(rasterizer_);
	}
	AudioDriverSDL2 audio_driver_;
	AudioServerSW *audio_server_;
	SampleManagerMallocSW *sample_manager_;
	SpatialSoundServerSW *spatial_sound_server_;
	SpatialSound2DServerSW *spatial_sound_2d_server_;
	void init_audio() {
		const int driver_id = 0;
		AudioDriverManagerSW::add_driver(&audio_driver_);
		AudioDriverManagerSW::get_driver(driver_id)->set_singleton();
		if (AudioDriverManagerSW::get_driver(driver_id)->init() != OK)
			fatal("audio driver failed to initialize.");
		sample_manager_ = memnew(SampleManagerMallocSW);
		audio_server_ = memnew(AudioServerSW(sample_manager_));
		audio_server_->init();
		spatial_sound_server_ = memnew(SpatialSoundServerSW);
		spatial_sound_server_->init();
		spatial_sound_2d_server_ = memnew(SpatialSound2DServerSW);
		spatial_sound_2d_server_->init();
	}
	void cleanup_audio() {
		spatial_sound_server_->finish();
		memdelete(spatial_sound_server_);
		spatial_sound_2d_server_->finish();
		memdelete(spatial_sound_2d_server_);
		memdelete(sample_manager_);
		audio_server_->finish();
		memdelete(audio_server_);
	}
	PhysicsServer *physics_server_;
	Physics2DServer *physics_2d_server_;
	void init_physics() {
		physics_server_ = memnew(PhysicsServerSW);
		physics_server_->init();
		physics_2d_server_ = memnew(Physics2DServerSW);
		physics_2d_server_->init();
	}
	void cleanup_physics() {
		physics_2d_server_->finish();
		memdelete(physics_2d_server_);
		physics_server_->finish();
		memdelete(physics_server_);
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
	void fill_modifier_state(::InputModifierState &st) {
		const InputModifierState *os_st = os_.get_modifier_state();
		st.shift = os_st->shift;
		st.alt = os_st->alt;
		st.control = os_st->control;
		st.meta = os_st->meta;
	}
public:
	Godot2_OS() : os_(this) {
		main_loop_ = 0;
		quit_ = false;
		event_id_ = 0;
	}
	int get_video_driver_count() const {
		return 1;
	}
	const char *get_video_driver_name(int driver) const {
		return "GLES2";
	}
	VideoMode get_default_video_mode() const {
		return OS::VideoMode(960, 540, false);
	}
	void initialize(const VideoMode &desired, int video_driver, int audio_driver) {
		video_mode_ = desired;
		os_.init(video_mode_.width, video_mode_.height, video_mode_.resizable, video_mode_.borderless_window, video_mode_.always_on_top);
		init_video();
		init_audio();
		init_physics();
		init_input();
		_ensure_data_dir();
	}
	void set_main_loop(MainLoop *main_loop) {
		main_loop_ = main_loop;
		input_->set_main_loop(main_loop);
	}
	void delete_main_loop() {
		if (main_loop_)
			memdelete(main_loop_);
		main_loop_ = 0;
	}
	void finalize() {
		delete_main_loop();
		cleanup_input();
		cleanup_physics();
		cleanup_audio();
		cleanup_video();
		os_.cleanup();
	}
	Point2 get_mouse_pos() const {
		return mouse_pos_;
	}
	int get_mouse_button_state() const {
		return mouse_state_;
	}
	void set_mouse_mode(OS::MouseMode mode) {
		os_.set_mouse_mode(map_mouse_mode(mode));
	}
	OS::MouseMode get_mouse_mode() const {
		return map_mouse_os_mode(os_.get_mouse_mode());
	}
	void set_window_title(const String &title) {
		os_.set_title(title.utf8().get_data());
	}
	void set_video_mode(const VideoMode &video_mode, int screen) {
	}
	VideoMode get_video_mode(int screen = 0) const {
		return video_mode_;
	}
	void get_fullscreen_mode_list(List<VideoMode> *list, int screen) const {
	}
	Size2 get_window_size() const {
		return Size2(video_mode_.width, video_mode_.height);
	}
	void set_window_size(const Size2 size) {
		ivec2 os_size = { size.width, size.height };
		os_.set_size(os_size);
		video_mode_.width = os_size.x;
		video_mode_.height = os_size.y;
	}
	Point2 get_window_position() const {
		ivec2 pos = os_.get_pos();
		return Point2(pos.x, pos.y);
	}
	void set_window_position(const Point2 &pos) {
		ivec2 os_pos = { pos.width, pos.height };
		os_.set_pos(os_pos);
	}
	void set_window_fullscreen(bool enable) {
		os_.set_fullscreen(enable);
		video_mode_.fullscreen = enable;
	}
	bool is_window_fullscreen() const {
		return os_.is_fullscreen();
	}
	void set_window_always_on_top(bool enable) {
		os_.set_always_on_top(enable);
		video_mode_.always_on_top = enable;
	}
	bool is_window_always_on_top() const {
		return os_.is_always_on_top();
	}
	void set_window_resizable(bool enable) {
		os_.set_resizable(enable);
	}
	bool is_window_resizable() const {
		return os_.is_resizable();
	}
	void set_window_maximized(bool enable) {
		os_.set_maximized(enable);
	}
	bool is_window_maximized() const {
		return os_.is_maximized();
	}
	void set_window_minimized(bool enable) {
		os_.set_minimized(enable);
	}
	bool is_window_minimized() const {
		return os_.is_minimized();
	}
	MainLoop *get_main_loop() const {
		return main_loop_;
	}
	bool can_draw() const {
		return true;
	}
	void set_cursor_shape(CursorShape shape) {
	}
	void set_custom_mouse_cursor(const RES &cursor, CursorShape shape, const Vector2 &hotspot) {
	}
	void make_rendering_thread() {
		os_.make_current();
	}
	void release_rendering_thread() {
		os_.release_current();
	}
	void swap_buffers() {
		os_.swap_buffers();
	}
	void handle_resize_event(ivec2 size) {
		video_mode_.width = size.x;
		video_mode_.height = size.y;
	}
	void handle_key_event(int sdl2_code, int unicode, bool pressed) {
		int code = map_key_sdl2_code(sdl2_code);
		InputEvent event;
		event.ID = ++event_id_;
		event.type = InputEvent::KEY;
		event.device = 0;
		fill_modifier_state(event.key.mod);
		event.key.pressed = pressed;
		event.key.scancode = code;
		event.key.unicode = unicode;
		event.key.echo = 0;
		input_->parse_input_event(event);
	}
	void handle_mouse_motion_event(ivec2 pos, ivec2 dpos) {
		mouse_pos_.x = pos.x;
		mouse_pos_.y = pos.y;
		InputEvent event;
		event.ID = ++event_id_;
		event.type = InputEvent::MOUSE_MOTION;
		event.device = 0;
		fill_modifier_state(event.mouse_motion.mod);
		event.mouse_motion.button_mask = mouse_state_;
		event.mouse_motion.x = mouse_pos_.x;
		event.mouse_motion.y = mouse_pos_.y;
		input_->set_mouse_pos(mouse_pos_);
		event.mouse_motion.global_x = mouse_pos_.x;
		event.mouse_motion.global_y = mouse_pos_.y;
		event.mouse_motion.speed_x = input_->get_mouse_speed().x;
		event.mouse_motion.speed_y = input_->get_mouse_speed().y;
		event.mouse_motion.relative_x = dpos.x;
		event.mouse_motion.relative_y = dpos.y;
		input_->parse_input_event(event);
	}
	void handle_mouse_button_event(int os_button, bool pressed, bool doubleclick) {
		int button = map_mouse_os_button(os_button);
		int bit = (1 << (button - 1));
		if (pressed)
			mouse_state_ |= bit;
		else
			mouse_state_ &= ~bit;
		InputEvent event;
		event.ID = ++event_id_;
		event.type = InputEvent::MOUSE_BUTTON;
		event.device = 0;
		fill_modifier_state(event.mouse_button.mod);
		event.mouse_button.button_mask = mouse_state_;
		event.mouse_button.x = mouse_pos_.x;
		event.mouse_button.y = mouse_pos_.y;
		event.mouse_button.global_x = mouse_pos_.x;
		event.mouse_button.global_y = mouse_pos_.y;
		event.mouse_button.button_index = button;
		event.mouse_button.doubleclick = doubleclick;
		event.mouse_button.pressed = pressed;
		input_->parse_input_event(event);
	}
	void handle_js_status_event(int id, bool connected, const char *name, const char *guid) {
		input_->joy_connection_changed(id, connected, name, guid);
	}
	void handle_js_button_event(int id, int button, bool pressed) {
		event_id_ = input_->joy_button(event_id_, id, button, pressed ? 1 : 0);
	}
	void handle_js_axis_event(int id, int axis, float value) {
		InputDefault::JoyAxis v = {-1, value}; // TODO: check if OK
		event_id_ = input_->joy_axis(event_id_, id, axis, v);
	}
	void handle_js_hat_event(int id, int os_mask) {
		int mask = map_hat_os_mask(os_mask);
		event_id_ = input_->joy_hat(event_id_, id, mask);
	}
	void handle_quit_event() {
		quit_ = true;
	}
	void run() {
		if (main_loop_) {
			main_loop_->init();
			while (!quit_ && !Main::iteration())
				os_.dispatch_events();
			main_loop_->finish();
		}
	}
};

} // namespace frt

#include "frt_lib.h"

extern "C" int frt_godot_main(int argc, char *argv[]) {
	frt::Godot2_OS os;
	Error err = Main::setup(argv[0], argc - 1, &argv[1]);
	if (err != OK)
		return 255;
	if (Main::start())
		os.run();
	Main::cleanup();
	return os.get_exit_code();
}
