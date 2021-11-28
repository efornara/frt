// frt_godot2.cc
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2021  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include <SDL.h>
#include "dl/gles2.gen.h"

#include "os/os.h"
#include "os/input.h"
#include "os/keyboard.h"
#include "drivers/unix/os_unix.h"
#include "drivers/gl_context/context_gl.h"
#include "servers/visual_server.h"
#include "servers/visual/visual_server_wrap_mt.h"
#include "servers/visual/rasterizer.h"
#include "servers/physics_server.h"
#include "servers/audio/audio_server_sw.h"
#include "servers/audio/sample_manager_sw.h"
#include "servers/audio/audio_driver_dummy.h"
#include "servers/spatial_sound/spatial_sound_server_sw.h"
#include "servers/spatial_sound_2d/spatial_sound_2d_server_sw.h"
#include "servers/physics/physics_server_sw.h"
#include "servers/physics_2d/physics_2d_server_sw.h"
#include "servers/physics_2d/physics_2d_server_wrap_mt.h"
#include "servers/visual/visual_server_raster.h"
#include "drivers/gles2/rasterizer_gles2.h"
#include "drivers/pulseaudio/audio_driver_pulseaudio.h"
#include "main/main.h"
#include "main/input_default.h"
#include "print_string.h"

#include "frt_utils.h"
#include "sdl2_adapter.h"

namespace frt {

class Godot2_OS : public OS_Unix, public SDL2EventHandler {
private:
	MainLoop *main_loop_;
	VideoMode video_mode_;
	bool quit_;
	SDL2OS os_;
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
	AudioDriverDummy driver_dummy_;
	AudioServerSW *audio_server_;
	SampleManagerMallocSW *sample_manager_;
	SpatialSoundServerSW *spatial_sound_server_;
	SpatialSound2DServerSW *spatial_sound_2d_server_;
	void init_audio() {
		const int audio_driver = 0;
		AudioDriverManagerSW::add_driver(&driver_dummy_);
		AudioDriverManagerSW::get_driver(audio_driver)->set_singleton();
		if (AudioDriverManagerSW::get_driver(audio_driver)->init() != OK)
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
		const int audio_driver = 0;
		spatial_sound_server_->finish();
		memdelete(spatial_sound_server_);
		spatial_sound_2d_server_->finish();
		memdelete(spatial_sound_2d_server_);
		memdelete(audio_server_);
		memdelete(sample_manager_);
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
	void init_input() {
		input_ = memnew(InputDefault);
	}
	void cleanup_input() {
		memdelete(input_);
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
		os_.init(video_mode_.width, video_mode_.height);
		init_video();
		init_audio();
		init_physics();
		init_input();
		_ensure_data_dir();
	}
	void set_main_loop(MainLoop *main_loop) {
		main_loop_ = main_loop;
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
		return Point2();
	}
	int get_mouse_button_state() const {
		return 0;
	}
	void set_window_title(const String &title) {
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
	void handle_key_event(int gd_code, bool pressed) {
		InputEvent event;
		event.ID = ++event_id_;
		event.type = InputEvent::KEY;
		event.device = 0;
		os_.get_modifier_state(event.key.mod);
		event.key.pressed = pressed;
		event.key.scancode = gd_code;
		event.key.unicode = 0;
		event.key.echo = 0;
		input_->parse_input_event(event);
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

extern "C" int frt_main(int argc, char *argv[]) {
	frt::Godot2_OS os;
	Error err = Main::setup(argv[0], argc - 1, &argv[1]);
	if (err != OK)
		return 255;
	if (Main::start())
		os.run();
	Main::cleanup();
	return os.get_exit_code();
}

int main(int argc, char *argv[]) {
	frt_main(argc, argv);
}
