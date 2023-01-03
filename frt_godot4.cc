// frt_godot4.cc
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2023  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "frt.h"
#include "sdl2_adapter.h"
#include "sdl2_godot_map_4.h"

#ifdef VULKAN_ENABLED
//#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"
#endif
#ifdef GLES3_ENABLED
#include "drivers/gles3/rasterizer_gles3.h"
#endif

#include "servers/audio_server.h"
#include "core/config/project_settings.h"

#include "drivers/unix/os_unix.h"
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
		mix_rate_ = GLOBAL_GET("audio/driver/mix_rate");
		speaker_mode_ = SPEAKER_MODE_STEREO;
		const int latency = GLOBAL_GET("audio/driver/output_latency");
		const int samples = closest_power_of_2(latency * mix_rate_ / 1000);
		return audio_.init(mix_rate_, samples) ? OK : ERR_CANT_OPEN;
	}
	int get_mix_rate() const override {
		return mix_rate_;
	}
	SpeakerMode get_speaker_mode() const override {
		return speaker_mode_;
	}
	PackedStringArray get_device_list() override {
		PackedStringArray list;
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

// global pointers for create functions and callables
struct OSEventHandler *os_event_handler_ = nullptr;
class Godot4_DisplayServer *display_server_ = nullptr;

struct OSEventHandler {
	virtual ~OSEventHandler();
	virtual void handle_quit_event() = 0;
};

OSEventHandler::~OSEventHandler() {
}

class Godot4_DisplayServer : public DisplayServer, public EventHandler {
private:
	OS_FRT os_;
	Callable rect_changed_callback_;
	Callable input_event_callback_;
	ObjectID instance_id_;
	Input *input_ = nullptr;
	Point2i mouse_pos_ = Point2i(-1, -1);
	int mouse_state_ = 0;
	bool resizable_;
	bool borderless_;
	bool always_on_top_;
	void fill_modifier_state(Ref<InputEventWithModifiers> st) {
		const InputModifierState *os_st = os_.get_modifier_state();
		st->set_shift_pressed(os_st->shift);
		st->set_alt_pressed(os_st->alt);
		st->set_ctrl_pressed(os_st->control);
		st->set_meta_pressed(os_st->meta);
	}
	void invoke_callback_1(Callable &callback, Variant &arg0) {
		if (!callback.is_valid())
			return;
		const Variant *args[] = { &arg0 };
		Variant ret;
		Callable::CallError err;
		callback.callp(args, 1, ret, err);
	}
public: // DisplayServer (implicit)
	Godot4_DisplayServer(const String &rendering_driver, WindowMode mode, VSyncMode vsync_mode, uint32_t flags, const Vector2i *position, const Vector2i &resolution, Error &error) : os_(this) {
		resizable_ = !(flags & WINDOW_FLAG_RESIZE_DISABLED_BIT);
		borderless_ = flags & WINDOW_FLAG_BORDERLESS_BIT;
		always_on_top_ = flags & WINDOW_FLAG_ALWAYS_ON_TOP_BIT;
		os_.init(API_OpenGL_ES3, resolution.width, resolution.height, resizable_, borderless_, always_on_top_);
		window_set_mode(mode, MAIN_WINDOW_ID);
		window_set_vsync_mode(vsync_mode, MAIN_WINDOW_ID);
		frt_resolve_symbols_gles3(get_proc_address);
		RasterizerGLES3::make_current();
		input_ = Input::get_singleton();
		input_->set_event_dispatch_function(dispatch_events_func);
		error = OK;
	}
	void dispatch_events(const Ref<InputEvent> &event) {
		Variant arg0 = event;
		invoke_callback_1(input_event_callback_, arg0);
	}
	static Vector<String> get_rendering_drivers_func() {
		Vector<String> res;
#ifdef VULKAN_ENABLED
//		res.push_back("vulkan");
#endif
#ifdef GLES3_ENABLED
		res.push_back("opengl3");
#endif
		return res;
	}
	static void dispatch_events_func(const Ref<InputEvent> &event) {
		display_server_->dispatch_events(event);
	}
	static DisplayServer *create_func(const String &rendering_driver, WindowMode mode, VSyncMode vsync_mode, uint32_t flags, const Vector2i *position, const Vector2i &resolution, Error &error) {
		display_server_ = memnew(Godot4_DisplayServer(rendering_driver, mode, vsync_mode, flags, position, resolution, error));
		if (error != OK)
			warn("display server creation failed.");
		return display_server_;
	}
	static void register_display_server() {
		register_create_function("frt", create_func, get_rendering_drivers_func);
	}
public: // DisplayServer
	bool has_feature(Feature feature) const override {
		switch (feature) {
		case FEATURE_MOUSE:
		case FEATURE_ICON:
		case FEATURE_SWAP_BUFFERS:
			return true;
		default:
			return false;
		}
	}
	String get_name() const override {
		return "FRT";
	}
	int get_screen_count() const override {
		return 1;
	}
	Point2i screen_get_position(int screen) const override {
		return Point2i();
	}
	Size2i screen_get_size(int screen) const override {
		return Size2i(1280, 720);
	}
	Rect2i screen_get_usable_rect(int screen) const override {
		return Rect2i(Point2i(), Size2i(1280, 720));
	}
	int screen_get_dpi(int screen) const override {
		return 72;
	}
	float screen_get_refresh_rate(int screen) const override {
		return 60.0f;
	}
	Vector<WindowID> get_window_list() const override {
		Vector<WindowID> res;
		res.push_back(MAIN_WINDOW_ID);
		return res;
	}
	WindowID get_window_at_screen_position(const Point2i &position) const override {
		return MAIN_WINDOW_ID;
	}
	void window_attach_instance_id(ObjectID instance, WindowID window) override {
		instance_id_ = instance;
	}
	ObjectID window_get_attached_instance_id(WindowID window) const override {
		return instance_id_;
	}
	void window_set_rect_changed_callback(const Callable &callable, WindowID window) override {
		rect_changed_callback_ = callable;
	}
	void window_set_window_event_callback(const Callable &callable, WindowID window) override {
	}
	void window_set_input_event_callback(const Callable &callable, WindowID window) override {
		input_event_callback_ = callable;
	}
	void window_set_input_text_callback(const Callable &callable, WindowID window) override {
	}
	void window_set_drop_files_callback(const Callable &callable, WindowID window) override {
	}
	Point2i mouse_get_position() const override {
		return mouse_pos_;
	}
	::MouseButton mouse_get_button_state() const override {
		return (::MouseButton)mouse_state_;
	}
	void mouse_set_mode(MouseMode mode) override {
		os_.set_mouse_mode(map_mouse_mode(mode));
	}
	MouseMode mouse_get_mode() const override {
		return map_mouse_os_mode(os_.get_mouse_mode());
	}
	void window_set_title(const String &title, WindowID window) override {
		os_.set_title(title.utf8().get_data());
	}
	int window_get_current_screen(WindowID window) const override {
		return 0;
	}
	void window_set_current_screen(int screen, WindowID window) override {
	}
	Point2i window_get_position(WindowID window) const override {
		ivec2 pos = os_.get_pos();
		return Point2i(pos.x, pos.y);
	}
	Point2i window_get_position_with_decorations(WindowID window) const override {
		ivec2 pos = os_.get_pos();
		return Point2i(pos.x, pos.y);
	}
	void window_set_position(const Point2i &position, WindowID window) override {
		ivec2 os_pos = { position.width, position.height };
		os_.set_pos(os_pos);
	}
	void window_set_transient(WindowID window, WindowID parent) override {
	}
	void window_set_max_size(const Size2i size, WindowID window) override {
	}
	Size2i window_get_max_size(WindowID window) const override {
		return Size2i();
	}
	void window_set_min_size(const Size2i size, WindowID window) override {
	}
	Size2i window_get_min_size(WindowID window) const override {
		return Size2i();
	}
	void window_set_size(const Size2i size, WindowID window) override {
		ivec2 os_size = { size.width, size.height };
		os_.set_size(os_size);
	}
	Size2i window_get_size(WindowID window) const override {
		ivec2 os_size = os_.get_size();
		return Size2i(os_size.x, os_size.y);
	}
	Size2i window_get_size_with_decorations(WindowID window) const override {
		ivec2 os_size = os_.get_size();
		return Size2i(os_size.x, os_size.y);
	}
	// TODO: update sdl2_adapter? wait?
	void window_set_mode(WindowMode mode, WindowID window) override {
		switch (mode) {
		case WINDOW_MODE_WINDOWED:
			if (os_.is_fullscreen())
				os_.set_fullscreen(false);
			else if (os_.is_maximized())
				os_.set_maximized(false);
			else if (os_.is_minimized())
				os_.set_minimized(false);
			break;
		case WINDOW_MODE_MINIMIZED:
			os_.set_minimized(true);
			break;
		case WINDOW_MODE_MAXIMIZED:
			os_.set_maximized(true);
			break;
		case WINDOW_MODE_FULLSCREEN:
		case WINDOW_MODE_EXCLUSIVE_FULLSCREEN:
			os_.set_fullscreen(true);
			break;
		}
	}
	WindowMode window_get_mode(WindowID window) const override {
		if (os_.is_fullscreen())
			return WINDOW_MODE_FULLSCREEN;
		else if (os_.is_maximized())
			return WINDOW_MODE_MAXIMIZED;
		else if (os_.is_minimized())
			return WINDOW_MODE_MINIMIZED;
		else
			return WINDOW_MODE_WINDOWED;
	}
	bool window_is_maximize_allowed(WindowID window) const override {
		return resizable_;
	}
	void window_set_flag(WindowFlags flag, bool enabled, WindowID window) override {
		// NOT IMPLEMENTED
	}
	bool window_get_flag(WindowFlags flag, WindowID window) const override {
		switch (flag) {
		case WINDOW_FLAG_RESIZE_DISABLED:
			return !resizable_;
		case WINDOW_FLAG_BORDERLESS:
			return borderless_;
		case WINDOW_FLAG_ALWAYS_ON_TOP:
			return always_on_top_;
		default:
			return false;
		}
	}
	void window_request_attention(WindowID window) override {
	}
	void window_move_to_foreground(WindowID window) override {
	}
	bool window_can_draw(WindowID window) const override {
		return true;
	}
	bool can_any_window_draw() const override {
		return true;
	}
	void process_events() override {
		os_.dispatch_events();
	}
	void release_rendering_thread() override {
		os_.release_current();
	}
	void make_rendering_thread() override {
		os_.make_current();
	}
	void swap_buffers() override {
		os_.swap_buffers();
	}
	VSyncMode window_get_vsync_mode(WindowID window) const override {
		return os_.is_vsync_enabled() ? VSYNC_ENABLED : VSYNC_DISABLED;
	}
	void window_set_vsync_mode(VSyncMode vsync_mode, WindowID window) override {
		os_.set_use_vsync(vsync_mode != VSYNC_DISABLED);
	}
	void set_icon(const Ref<Image> &icon) override {
		if (icon.is_null())
			return;
		Ref<Image> i = icon->duplicate();
		i->convert(Image::FORMAT_RGBA8);
		os_.set_icon(i->get_width(), i->get_height(), i->get_data().ptr());
	}
public: // EventHandler
	void handle_resize_event(ivec2 size) override {
		ivec2 pos = os_.get_pos();
		Rect2i rect(pos.x, pos.y, size.x, size.y);
		Variant arg0 = rect;
		invoke_callback_1(rect_changed_callback_, arg0);
	}
	void handle_key_event(int sdl2_code, int unicode, bool pressed) override {
		Key code = map_key_sdl2_code(sdl2_code);
		Ref<InputEventKey> key;
		key.instantiate();
		fill_modifier_state(key);
		key->set_window_id(MAIN_WINDOW_ID);
		key->set_pressed(pressed);
		key->set_keycode(code);
		key->set_physical_keycode(code);
		key->set_unicode(unicode);
		key->set_echo(false);
		input_->parse_input_event(key);
	}
	void handle_mouse_motion_event(ivec2 pos, ivec2 dpos) override {
		mouse_pos_.x = pos.x;
		mouse_pos_.y = pos.y;
		Ref<InputEventMouseMotion> mouse_motion;
		mouse_motion.instantiate();
		fill_modifier_state(mouse_motion);
		mouse_motion->set_window_id(MAIN_WINDOW_ID);
		Point2i posi(pos.x, pos.y);
		mouse_motion->set_button_mask((::MouseButton)mouse_state_);
		mouse_motion->set_position(posi);
		mouse_motion->set_global_position(mouse_motion->get_position());
		input_->set_mouse_position(posi);
		mouse_motion->set_velocity(input_->get_last_mouse_velocity());
		Point2i reli(dpos.x, dpos.y);
		mouse_motion->set_relative(reli);
		input_->parse_input_event(mouse_motion);
	}
	void handle_mouse_button_event(int os_button, bool pressed, bool doubleclick) override {
		int button = (int)map_mouse_os_button(os_button);
		int bit = (1 << (button - 1));
		if (pressed)
			mouse_state_ |= bit;
		else
			mouse_state_ &= ~bit;
		Ref<InputEventMouseButton> mouse_button;
		mouse_button.instantiate();
		fill_modifier_state(mouse_button);
		mouse_button->set_window_id(MAIN_WINDOW_ID);
		Point2i posi(mouse_pos_.x, mouse_pos_.y);
		mouse_button->set_position(posi);
		mouse_button->set_global_position(posi);
		mouse_button->set_global_position(mouse_button->get_position());
		mouse_button->set_button_index((::MouseButton)button);
		mouse_button->set_button_mask((::MouseButton)mouse_state_);
		mouse_button->set_double_click(doubleclick);
		mouse_button->set_pressed(pressed);
		input_->parse_input_event(mouse_button);
	}
	void handle_js_status_event(int id, bool connected, const char *name, const char *guid) override {
		input_->joy_connection_changed(id, connected, name, guid);
	}
	void handle_js_button_event(int id, int button, bool pressed) override {
		input_->joy_button(id, (JoyButton)button, pressed ? 1 : 0);
	}
	void handle_js_axis_event(int id, int axis, float value) override {
		input_->joy_axis(id, (JoyAxis)axis, value);
	}
	void handle_js_hat_event(int id, int os_mask) override {
		::HatMask mask = map_hat_os_mask(os_mask);
		input_->joy_hat(id, mask);
	}
	void handle_quit_event() override {
		os_event_handler_->handle_quit_event();
	}
	void handle_flush_events() override {
		input_->flush_buffered_events();
	}
};

class Godot4_OS : public OS_Unix, public OSEventHandler {
private:
	MainLoop *main_loop_ = nullptr;
	bool quit_ = false;
	AudioDriverSDL2 audio_driver_;
public:
	Godot4_OS() {
		os_event_handler_ = this;
		AudioDriverManager::add_driver(&audio_driver_); // TODO: headless with audio?
		Godot4_DisplayServer::register_display_server();
	}
	void run() {
		if (main_loop_) {
			main_loop_->initialize();
			while (!quit_ && !Main::iteration())
				DisplayServer::get_singleton()->process_events(); // no display_server_ because it could be running headless
			main_loop_->finalize();
		}
	}
public: // OS
	void initialize() override {
		OS_Unix::initialize_core();
	}
	void initialize_joypads() override {
	}
	void set_main_loop(MainLoop *main_loop) override {
		main_loop_ = main_loop;
	}
	void delete_main_loop() override {
		if (main_loop_)
			memdelete(main_loop_);
		main_loop_ = nullptr;
	}
	void finalize() override {
		delete_main_loop();
	}
	bool _check_internal_feature_support(const String &feature) override {
		return false;
	}
	MainLoop *get_main_loop() const override {
		return main_loop_;
	}
public: // OSEventHandler
	void handle_quit_event() override {
		quit_ = true;
	}
};

} // namespace frt

#include "frt_lib.h"

extern "C" int frt_godot_main(int argc, char *argv[]) {
	frt::Godot4_OS os;
	Error err = Main::setup(argv[0], argc - 1, &argv[1]);
	if (err != OK)
		return 255;
	if (Main::start())
		os.run();
	Main::cleanup();
	return os.get_exit_code();
}
