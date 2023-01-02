// frt_godot4.cc
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2023  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "frt.h"
#include "sdl2_adapter.h"

#include "servers/display_server.h"
#ifdef VULKAN_ENABLED
//#include "servers/rendering/renderer_rd/renderer_compositor_rd.h"
#endif
#ifdef GLES3_ENABLED
#include "drivers/gles3/rasterizer_gles3.h"
#endif

#include "drivers/unix/os_unix.h"
#include "main/main.h"

namespace frt {

struct OSEventHandler {
	virtual ~OSEventHandler();
	virtual void handle_quit_event() = 0;
};

OSEventHandler::~OSEventHandler() {
}

OSEventHandler *os_event_handler = nullptr; // TODO: is it possible to pass context to create?

class Godot4_DisplayServer : public DisplayServer, public EventHandler {
private:
	OS_FRT os_;
	OSEventHandler *os_event_handler_ = os_event_handler;
	ObjectID instance_id_;
public:
	Godot4_DisplayServer(const String &rendering_driver, WindowMode mode, VSyncMode vsync_mode, uint32_t flags, const Vector2i *position, const Vector2i &resolution, Error &error) : os_(this) {
		os_.init(API_OpenGL_ES3, resolution.width, resolution.height, false, false, false);
		window_set_vsync_mode(vsync_mode, MAIN_WINDOW_ID);
		frt_resolve_symbols_gles3(get_proc_address);
		RasterizerGLES3::make_current();
		error = OK;
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
	static DisplayServer *create_func(const String &rendering_driver, WindowMode mode, VSyncMode vsync_mode, uint32_t flags, const Vector2i *position, const Vector2i &resolution, Error &error) {
		DisplayServer *display_server = memnew(Godot4_DisplayServer(rendering_driver, mode, vsync_mode, flags, position, resolution, error));
		if (error != OK)
			warn("display server creation failed.");
		return display_server;
	}
	static void register_display_server() {
		register_create_function("frt", create_func, get_rendering_drivers_func);
	}
public: // DisplayServer
	bool has_feature(Feature feature) const override {
		switch (feature) {
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
	}
	void window_set_window_event_callback(const Callable &callable, WindowID window) override {
	}
	void window_set_input_event_callback(const Callable &callable, WindowID window) override {
	}
	void window_set_input_text_callback(const Callable &callable, WindowID window) override {
	}
	void window_set_drop_files_callback(const Callable &callable, WindowID window) override {
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
	void window_set_mode(WindowMode mode, WindowID window) override {
	}
	WindowMode window_get_mode(WindowID window) const override {
		return WINDOW_MODE_WINDOWED;
	}
	bool window_is_maximize_allowed(WindowID window) const override {
		return false;
	}
	void window_set_flag(WindowFlags flag, bool enabled, WindowID window) override {
	}
	bool window_get_flag(WindowFlags flag, WindowID window) const override {
		return false;
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
	}
	void handle_key_event(int sdl2_code, int unicode, bool pressed) override {
		if (pressed)
			return;
		switch (sdl2_code) {
		case SDLK_ESCAPE:
			os_event_handler_->handle_quit_event();
			break;
		case SDLK_v:
			os_.set_use_vsync(!os_.is_vsync_enabled());
			break;
		}
	}
	void handle_mouse_motion_event(ivec2 pos, ivec2 dpos) override {
	}
	void handle_mouse_button_event(int button, bool pressed, bool doubleclick) override {
	}
	void handle_js_status_event(int id, bool connected, const char *name, const char *guid) override {
	}
	void handle_js_button_event(int id, int button, bool pressed) override {
	}
	void handle_js_axis_event(int id, int axis, float value) override {
	}
	void handle_js_hat_event(int id, int mask) override {
	}
	void handle_quit_event() override {
		os_event_handler_->handle_quit_event();
	}
	void handle_flush_events() override {
		Input::get_singleton()->flush_buffered_events();
	}
};

class Godot4_OS : public OS_Unix, public OSEventHandler {
private:
	MainLoop *main_loop_ = nullptr;
	bool quit_ = false;
public:
	Godot4_OS() {
		os_event_handler = this;
		Godot4_DisplayServer::register_display_server();
	}
	void run() {
		if (main_loop_) {
			main_loop_->initialize();
			while (!quit_ && !Main::iteration())
				DisplayServer::get_singleton()->process_events();
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
