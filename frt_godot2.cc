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

// TODO: extract sdl2 adapter
namespace frt {

struct KeyMap {
	int sdl2_code;
	int gd_code;
} keymap[] = {
	{ SDLK_SPACE, ' ' },
	{ SDLK_a, 'A' },
	{ SDLK_b, 'B' },
	{ SDLK_c, 'C' },
	{ SDLK_d, 'D' },
	{ SDLK_e, 'E' },
	{ SDLK_f, 'F' },
	{ SDLK_g, 'G' },
	{ SDLK_h, 'H' },
	{ SDLK_i, 'I' },
	{ SDLK_j, 'J' },
	{ SDLK_k, 'K' },
	{ SDLK_l, 'L' },
	{ SDLK_m, 'M' },
	{ SDLK_n, 'N' },
	{ SDLK_o, 'O' },
	{ SDLK_p, 'P' },
	{ SDLK_q, 'Q' },
	{ SDLK_r, 'R' },
	{ SDLK_s, 'S' },
	{ SDLK_t, 'T' },
	{ SDLK_u, 'U' },
	{ SDLK_v, 'V' },
	{ SDLK_w, 'W' },
	{ SDLK_x, 'X' },
	{ SDLK_y, 'Y' },
	{ SDLK_z, 'Z' },
	{ SDLK_0, '0' },
	{ SDLK_1, '1' },
	{ SDLK_2, '2' },
	{ SDLK_3, '3' },
	{ SDLK_4, '4' },
	{ SDLK_5, '5' },
	{ SDLK_6, '6' },
	{ SDLK_7, '7' },
	{ SDLK_8, '8' },
	{ SDLK_9, '9' },
	{ SDLK_F1, KEY_F1 },
	{ SDLK_F2, KEY_F2 },
	{ SDLK_F3, KEY_F3 },
	{ SDLK_F4, KEY_F4 },
	{ SDLK_F5, KEY_F5 },
	{ SDLK_F6, KEY_F6 },
	{ SDLK_F7, KEY_F7 },
	{ SDLK_F8, KEY_F8 },
	{ SDLK_F9, KEY_F9 },
	{ SDLK_F10, KEY_F10 },
	{ SDLK_F11, KEY_F11 },
	{ SDLK_F12, KEY_F12 },
	{ SDLK_UP, KEY_UP },
	{ SDLK_DOWN, KEY_DOWN },
	{ SDLK_LEFT, KEY_LEFT },
	{ SDLK_RIGHT, KEY_RIGHT },
	{ SDLK_TAB, KEY_TAB },
	{ SDLK_BACKSPACE, KEY_BACKSPACE },
	{ SDLK_INSERT, KEY_INSERT },
	{ SDLK_DELETE, KEY_DELETE },
	{ SDLK_HOME, KEY_HOME },
	{ SDLK_END, KEY_END },
	{ SDLK_PAGEUP, KEY_PAGEUP },
	{ SDLK_PAGEDOWN, KEY_PAGEDOWN },
	{ SDLK_RETURN, KEY_RETURN },
	{ SDLK_ESCAPE, KEY_ESCAPE },
	{ SDLK_LCTRL, KEY_CONTROL },
	{ SDLK_RCTRL, KEY_CONTROL },
	{ SDLK_LALT, KEY_ALT },
	{ SDLK_RALT, KEY_ALT },
	{ SDLK_LSHIFT, KEY_SHIFT },
	{ SDLK_RSHIFT, KEY_SHIFT },
	{ SDLK_LGUI, KEY_META },
	{ SDLK_RGUI, KEY_META },
	{ SDLK_KP_0, KEY_KP_0 },
	{ SDLK_KP_1, KEY_KP_1 },
	{ SDLK_KP_2, KEY_KP_2 },
	{ SDLK_KP_3, KEY_KP_3 },
	{ SDLK_KP_4, KEY_KP_4 },
	{ SDLK_KP_5, KEY_KP_5 },
	{ SDLK_KP_6, KEY_KP_6 },
	{ SDLK_KP_7, KEY_KP_7 },
	{ SDLK_KP_8, KEY_KP_8 },
	{ SDLK_KP_9, KEY_KP_9 },
	{ SDLK_KP_MULTIPLY, KEY_KP_MULTIPLY },
	{ SDLK_KP_MINUS, KEY_KP_SUBTRACT },
	{ SDLK_KP_PLUS, KEY_KP_ADD },
	{ SDLK_KP_PERIOD, KEY_KP_PERIOD },
	{ SDLK_KP_ENTER, KEY_KP_ENTER },
	{ SDLK_KP_DIVIDE, KEY_KP_DIVIDE },
	{ 0, 0 },
};

struct SDL2EventHandler {
	virtual ~SDL2EventHandler();
	virtual void handle_key_event(int gd_code, bool pressed) = 0;
	virtual void handle_quit_event() = 0;
};

SDL2EventHandler::~SDL2EventHandler() {
}

class SDL2Adapter {
private:
	SDL_Window *window_;
	SDL_GLContext context_;
	SDL2EventHandler *handler_;
	InputModifierState st_;
	void key_event(const SDL_Event &ev) {
		st_.shift = ev.key.keysym.mod & KMOD_SHIFT;
		st_.alt = ev.key.keysym.mod & KMOD_ALT;
		st_.control = ev.key.keysym.mod & KMOD_CTRL;
		st_.meta = ev.key.keysym.mod & KMOD_GUI;
		if (ev.key.repeat)
			return;
		bool pressed = ev.key.state == SDL_PRESSED;
		for (int i = 0; keymap[i].sdl2_code; i++) {
			if (keymap[i].sdl2_code == ev.key.keysym.sym) {
				handler_->handle_key_event(keymap[i].gd_code, pressed);
				return;
			}
		}
	}
public:
	SDL2Adapter(SDL2EventHandler *handler) : handler_(handler) {
		st_.shift = false;
		st_.alt = false;
		st_.control = false;
		st_.meta = false;
	}
	void init(int width, int height) {
		if (SDL_Init(SDL_INIT_VIDEO) < 0)
			fatal("SDL_Init failed.");
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		if (!(window_ = SDL_CreateWindow("frt2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI)))
			fatal("SDL_CreateWindow failed.");
		context_ = SDL_GL_CreateContext(window_);
		SDL_GL_MakeCurrent(window_, context_);
		// TODO: use SDL2 instead of dl
		frt_load_gles2("libGLESv2.so");
	}
	void cleanup() {
		SDL_DestroyWindow(window_);
		SDL_Quit();
	}
	void make_current() {
		SDL_GL_MakeCurrent(window_, context_);
	}
	void release_current() {
		// TODO: add release
	}
	void swap_buffers() {
		SDL_GL_SwapWindow(window_);
	}
	void dispatch_events() {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				key_event(ev);
				break;
			case SDL_QUIT:
				//handler_->handle_quit_event(); // audio bug
				exit(1);
				break;
			}
		}
	}
	void get_modifier_state(InputModifierState &state) const {
		state = st_;
	}
};

} // namespace frt

namespace frt {

class Godot2_OS : public OS_Unix, public SDL2EventHandler {
private:
	MainLoop *main_loop_;
	VideoMode video_mode_;
	bool quit_;
	SDL2Adapter sdl2_;
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
	Godot2_OS() : sdl2_(this) {
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
		sdl2_.init(video_mode_.width, video_mode_.height);
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
		sdl2_.cleanup();
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
		sdl2_.make_current();
	}
	void release_rendering_thread() {
		sdl2_.release_current();
	}
	void swap_buffers() {
		sdl2_.swap_buffers();
	}
	void handle_key_event(int gd_code, bool pressed) {
		InputEvent event;
		event.ID = ++event_id_;
		event.type = InputEvent::KEY;
		event.device = 0;
		sdl2_.get_modifier_state(event.key.mod);
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
				sdl2_.dispatch_events();
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
