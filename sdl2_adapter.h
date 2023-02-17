// sdl2_adapter.h
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2023  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

/*

  KNOWN ISSUES:

  Unicode

  Keyboard/Text SDL2 events don't map very well to Godot input events.
  If a SDL2 keypress event might need to be "translated", that event is saved
  and passed to Godot only when a translation is received.
  This is incomplete and likely to be a source of bugs. Ctrl+... combinations
  for example don't seem to generate a translation. Multiple keypresses
  are also going to have a wrong unicode traslation.
  Still, this implementation seems a good compromise: simple enough,
  yet functional enough.
  The 2D platformer feels responsive (even when adding WASD input mappings
  and pressing multiple keys), and the editor is usable for basic code editing
  (tested with US and IT keymaps).

 */

#include <SDL.h>
#ifdef VULKAN_ENABLED
#include <SDL_vulkan.h>
#endif

namespace frt {

void *(*get_proc_address)(const char *) = SDL_GL_GetProcAddress;

struct SampleProducer {
	virtual void produce_samples(int n_of_frames, int32_t *frames) = 0;
	virtual ~SampleProducer();
};

SampleProducer::~SampleProducer() {
}

void audio_callback(void *userdata, Uint8 *stream, int len);

class Audio {
private:
	SampleProducer *producer_;
	SDL_mutex *mutex_;
	int32_t *samples_;
	int n_of_samples_;
public:
	Audio(SampleProducer *producer) : producer_(producer) {
		mutex_ = 0;
	}
	bool init(int mix_rate, int samples) {
		SDL_AudioSpec desired, obtained;
		memset(&desired, 0, sizeof(desired));
		desired.freq = mix_rate;
		desired.format = AUDIO_S16;
		desired.channels = 2;
		desired.samples = samples;
		desired.callback = audio_callback;
		desired.userdata = this;
		if (SDL_OpenAudio(&desired, &obtained))
			return false;
		mutex_ = SDL_CreateMutex();
		n_of_samples_ = obtained.channels * obtained.samples;
		samples_ = new int32_t[n_of_samples_];
		return true;
	}
	void start() {
		SDL_PauseAudio(SDL_FALSE);
	}
	void lock() {
		SDL_LockMutex(mutex_);
	}
	void unlock() {
		SDL_UnlockMutex(mutex_);
	}
	void finish() {
		SDL_PauseAudio(SDL_TRUE);
		SDL_LockAudio();
		// calling of sdl2 callback not expected after pause+lock
		SDL_DestroyMutex(mutex_);
		mutex_ = 0;
		delete[] samples_;
		samples_ = 0;
		SDL_UnlockAudio();
		SDL_CloseAudio();
	}
	void fill_buffer(unsigned char *data, int length) {
		int n_of_samples = length / sizeof(int16_t);
		if (n_of_samples > n_of_samples_) // just in case, it shouldn't happen
			n_of_samples = n_of_samples_;
		const int channels = 2;
		SDL_LockMutex(mutex_);
		producer_->produce_samples(n_of_samples / channels, samples_);
		SDL_UnlockMutex(mutex_);
		int16_t *data16 = (int16_t *)data; // assume alignment is fine
		for (int i = 0; i < n_of_samples; i++)
			data16[i] = samples_[i] >> 16;
	}
};

void audio_callback(void *userdata, Uint8 *stream, int len) {
	Audio *audio = (Audio *)userdata;
	audio->fill_buffer(stream, len);
}

struct ivec2 {
	int x;
	int y;
};

enum HatMask {
	HatUp = 1,
	HatRight = 2,
	HatDown = 4,
	HatLeft = 8
};

enum MouseButton {
	ButtonLeft = 1,
	ButtonRight = 2,
	ButtonMiddle = 3,
	WheelUp = 4,
	WheelDown = 5
};

enum MouseMode {
	MouseVisible,
	MouseHidden,
	MouseCaptured
};

struct EventHandler {
	virtual ~EventHandler();
	virtual void handle_resize_event(ivec2 size) = 0;
	virtual void handle_key_event(int sdl2_code, int unicode, bool pressed) = 0;
	virtual void handle_mouse_motion_event(ivec2 pos, ivec2 dpos) = 0;
	virtual void handle_mouse_button_event(int button, bool pressed, bool doubleclick) = 0;
	virtual void handle_js_status_event(int id, bool connected, const char *name, const char *guid) = 0;
	virtual void handle_js_button_event(int id, int button, bool pressed) = 0;
	virtual void handle_js_axis_event(int id, int axis, float value) = 0;
	virtual void handle_js_hat_event(int id, int mask) = 0;
	virtual void handle_js_vibra_event(int id, uint64_t timestamp) = 0;
	virtual void handle_quit_event() = 0;
	virtual void handle_flush_events() = 0;
};

EventHandler::~EventHandler() {
}

struct InputModifierState {
	bool shift;
	bool alt;
	bool control;
	bool meta;
	InputModifierState() : shift(false), alt(false), control(false), meta(false) {}
};

enum GraphicsAPI {
	API_OpenGL_ES2,
	API_OpenGL_ES3,
	API_Vulkan
};

// TODO: factor out env vars parsing

enum ExitShortcut {
	ES_None,
	ES_ShiftEnter,
	ES_WinQ,
	ES_Esc
};

ExitShortcut parse_exit_shortcut() {
	const char *s = getenv("FRT_EXIT_SHORTCUT");
	if (!s || !strcmp(s, "none"))
		return ES_None;
	else if (!strcmp(s, "shift-enter"))
		return ES_ShiftEnter;
	else if (!strcmp(s, "win-q"))
		return ES_WinQ;
	else if (!strcmp(s, "esc"))
		return ES_Esc;
	warn("invalid FRT_EXIT_SHORTCUT (%s), using: esc", s);
	return ES_Esc;
}

class OS_FRT {
private:
	static const int MAX_JOYSTICKS = 16;
	static const int REQUEST_UNICODE = -1;
	SDL_Window *window_;
	SDL_GLContext context_;
	bool is_vulkan_;
	EventHandler *handler_;
	InputModifierState st_;
	MouseMode mouse_mode_;
	SDL_KeyboardEvent key_ev_;
	int key_unicode_;
	SDL_Joystick *js_[MAX_JOYSTICKS];
	uint64_t rumble_timestamp_[MAX_JOYSTICKS];
	uint32_t rumble_supported_;
	ExitShortcut exit_shortcut_;
	void resize_event(const SDL_Event &ev) {
		ivec2 size;
#ifdef VULKAN_ENABLED
		if (is_vulkan_)
			SDL_Vulkan_GetDrawableSize(window_, &size.x, &size.y);
#endif
		if (!is_vulkan_)
			SDL_GL_GetDrawableSize(window_, &size.x, &size.y);
		handler_->handle_resize_event(size);
	}
	int utf8_to_unicode(const char *s) {
		if ((s[0] & 0x80) == 0)
			return s[0];
		else if ((s[0] & 0xe0) == 0xc0)
			return ((s[0] & 0x1f) << 6) | (s[1] & 0x3f);
		else if ((s[0] & 0xf0) == 0xe0)
			return ((s[0] & 0x0f) << 12) | ((s[1] & 0x3f) << 6) | (s[2] & 0x3f);
		else
			return 0;
	}
	void text_event(const SDL_TextInputEvent &text) {
		if (key_unicode_ != REQUEST_UNICODE)
			return;
		key_unicode_ = utf8_to_unicode(text.text);
		int sdl2_code = key_ev_.keysym.sym;
		handler_->handle_key_event(sdl2_code, key_unicode_, true);
	}
	bool require_unicode(int c) {
		return (c >= 0x20 && c < 0x80) || (c >= 0xa0 && c < 0xff);
	}
	void key_event(const SDL_KeyboardEvent &key) {
		st_.shift = key.keysym.mod & KMOD_SHIFT;
		st_.alt = key.keysym.mod & KMOD_ALT;
		st_.control = key.keysym.mod & KMOD_CTRL;
		st_.meta = key.keysym.mod & KMOD_GUI;
		bool pressed = key.state == SDL_PRESSED;
		int sdl2_code = key.keysym.sym;
		if (exit_shortcut_ != ES_None && pressed) {
			bool quit = false;
			switch (exit_shortcut_) {
			case ES_ShiftEnter:
				quit = st_.shift && (sdl2_code == SDLK_KP_ENTER || sdl2_code == SDLK_RETURN);
				break;
			case ES_WinQ:
				quit = st_.meta && sdl2_code == SDLK_q;
				break;
			case ES_Esc:
				quit = sdl2_code == SDLK_ESCAPE;
				break;
			default:
				break;
			}
			if (quit) {
				handler_->handle_quit_event();
				return;
			}
		}
		if (pressed && !key.repeat && require_unicode(sdl2_code)) {
			key_ev_ = key;
			key_unicode_ = REQUEST_UNICODE;
			return;
		}
		handler_->handle_key_event(sdl2_code, key_unicode_, pressed);
		if (!pressed)
			key_unicode_ = 0;
	}
	void mouse_event(const SDL_Event &ev) {
		int os_button;
		if (ev.type == SDL_MOUSEMOTION) {
			ivec2 pos = { ev.motion.x, ev.motion.y };
			ivec2 dpos = { ev.motion.xrel, ev.motion.yrel };
			handler_->handle_mouse_motion_event(pos, dpos);
		} else if (ev.type == SDL_MOUSEWHEEL) {
			if (ev.wheel.y > 0)
				os_button = WheelUp;
			else if (ev.wheel.y < 0)
				os_button = WheelDown;
			else
				return;
			handler_->handle_mouse_button_event(os_button, true, false);
			handler_->handle_mouse_button_event(os_button, false, false);
		} else { // SDL_MOUSEBUTTONUP, SDL_MOUSEBUTTONDOWN
			switch (ev.button.button) {
			case SDL_BUTTON_LEFT:
				os_button = ButtonLeft;
				break;
			case SDL_BUTTON_MIDDLE:
				os_button = ButtonMiddle;
				break;
			case SDL_BUTTON_RIGHT:
				os_button = ButtonRight;
				break;
			default:
				return;
			}
			handler_->handle_mouse_button_event(os_button, ev.button.state == SDL_PRESSED, ev.button.clicks > 1);
		}
	}
	int get_js_id(int inst_id) {
		SDL_Joystick *js = SDL_JoystickFromInstanceID(inst_id);
		for (int id = 0; id < MAX_JOYSTICKS; id++)
			if (js_[id] == js)
				return id;
		return -1;
	}
	void js_event(const SDL_Event &ev) {
		int id;
		switch (ev.type) {
		case SDL_JOYAXISMOTION: {
			if ((id = get_js_id(ev.jaxis.which)) < 0)
				return;
			int axis = ev.jaxis.axis;
			float value = (float)ev.jaxis.value / 32768.0f;
			handler_->handle_js_axis_event(id, axis, value);
			} break;
		case SDL_JOYHATMOTION: {
			if ((id = get_js_id(ev.jhat.which)) < 0)
				return;
			if (ev.jhat.hat != 0)
				return;
			int mask = 0;
			switch (ev.jhat.value) {
			case SDL_HAT_LEFT:
				mask = HatLeft;
				break;
			case SDL_HAT_LEFTUP:
				mask = HatLeft | HatUp;
				break;
			case SDL_HAT_UP:
				mask = HatUp;
				break;
			case SDL_HAT_RIGHTUP:
				mask = HatRight | HatUp;
				break;
			case SDL_HAT_RIGHT:
				mask = HatRight;
				break;
			case SDL_HAT_RIGHTDOWN:
				mask = HatRight | HatDown;
				break;
			case SDL_HAT_DOWN:
				mask = HatDown;
				break;
			case SDL_HAT_LEFTDOWN:
				mask = HatLeft | HatDown;
				break;
			}
			handler_->handle_js_hat_event(id, mask);
			} break;
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYBUTTONUP: {
			if ((id = get_js_id(ev.jbutton.which)) < 0)
				return;
			int button = ev.jbutton.button;
			bool pressed = ev.jbutton.state == SDL_PRESSED;
			handler_->handle_js_button_event(id, button, pressed);
			} break;
		case SDL_JOYDEVICEADDED: {
			id = ev.jdevice.which;
			if (id >= MAX_JOYSTICKS)
				return;
			const char *name = SDL_JoystickNameForIndex(id);
			char guid[64];
			SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(id), guid, sizeof(guid));
			handler_->handle_js_status_event(id, true, name, guid);
			js_[id] = SDL_JoystickOpen(id);
			rumble_timestamp_[id] = 0;
			rumble_supported_ |= (1 << id);
			} break;
		case SDL_JOYDEVICEREMOVED: {
			if ((id = get_js_id(ev.jdevice.which)) < 0)
				return;
			SDL_JoystickClose(js_[id]);
			js_[id] = 0;
			handler_->handle_js_status_event(id, false, "", "");
			} break;
		}
	}
	void vibra_events() {
		for (int id = 0; id < MAX_JOYSTICKS; id++)
			if (js_[id] && (rumble_supported_ & (1 << id)))
				handler_->handle_js_vibra_event(id, rumble_timestamp_[id]);
	}
public:
	OS_FRT(EventHandler *handler) : handler_(handler) {
		mouse_mode_ = MouseVisible;
		key_unicode_ = 0;
		memset(js_, 0, sizeof(js_));
		exit_shortcut_ = parse_exit_shortcut();
	}
#ifdef VULKAN_ENABLED
	const char *get_vk_surface_extension() {
		const char *extension = getenv("FRT_VK_SURFACE_EXTENSION");
		if (extension)
			return extension;
		const char *driver = SDL_GetCurrentVideoDriver();
		if (!driver)
			driver = "<null>";
		else if (!strcmp(driver, "x11"))
			return "VK_KHR_xlib_surface";
		else if (!strcmp(driver, "wayland"))
			return "VK_KHR_wayland_surface";
		else if (!strcmp(driver, "KMSDRM"))
			return "VK_KHR_display";
		fatal("unknown vk_surface extension for sdl2 driver '%s'", driver);
	}
	void init_context_vulkan(VkInstance vk_instance, int width, int height, VkSurfaceKHR *vk_surface) {
		if (!SDL_Vulkan_CreateSurface(window_, vk_instance, vk_surface))
			fatal("SDL_Vulkan_CreateSurface failed: %s.", SDL_GetError());
	}
#endif
	void init_context_gl() {
		context_ = SDL_GL_CreateContext(window_);
		SDL_GL_MakeCurrent(window_, context_);
	}
	void init_window(GraphicsAPI api, int width, int height, bool resizable, bool borderless, bool always_on_top) {
		setenv("SDL_VIDEO_RPI_OPTIONS", "gravity=center,scale=letterbox,background=1", 0);
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
			fatal("SDL_Init failed: %s.", SDL_GetError());
		is_vulkan_ = api == API_Vulkan;
		int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
		if (is_vulkan_) {
#ifdef VULKAN_ENABLED
			flags |= SDL_WINDOW_VULKAN;
#endif
		} else {
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, api == API_OpenGL_ES2 ? 2 : 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
			flags |= SDL_WINDOW_OPENGL;
		}
		if (resizable)
			flags |= SDL_WINDOW_RESIZABLE;
		if (borderless)
			flags |= SDL_WINDOW_BORDERLESS;
		if (always_on_top)
			flags |= SDL_WINDOW_ALWAYS_ON_TOP;
		if (!(window_ = SDL_CreateWindow("frt2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags)))
			fatal("SDL_CreateWindow failed: %s.", SDL_GetError());
	}
	void init_gl(GraphicsAPI api, int width, int height, bool resizable, bool borderless, bool always_on_top) {
		init_window(api, width, height, resizable, borderless, always_on_top);
		init_context_gl();
	}
	void cleanup() {
		SDL_DestroyWindow(window_);
		SDL_Quit();
	}
	void make_current_gl() {
		SDL_GL_MakeCurrent(window_, context_);
	}
	void release_current_gl() {
		// TODO: add release
	}
	void swap_buffers_gl() {
		SDL_GL_SwapWindow(window_);
	}
	void set_use_vsync_gl(bool enable) {
		SDL_GL_SetSwapInterval(enable ? 1 : 0);
	}
	bool is_vsync_enabled_gl() const {
		return SDL_GL_GetSwapInterval() != 0;
	}
	void dispatch_events() {
		SDL_Event ev;
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_WINDOWEVENT:
				if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
					resize_event(ev);
				break;
			case SDL_TEXTINPUT:
				text_event(ev.text);
				break;
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				key_event(ev.key);
				break;
			case SDL_MOUSEMOTION:
			case SDL_MOUSEWHEEL:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				mouse_event(ev);
				break;
			case SDL_JOYAXISMOTION:
			case SDL_JOYHATMOTION:
			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
			case SDL_JOYDEVICEADDED:
			case SDL_JOYDEVICEREMOVED:
				js_event(ev);
				break;
			case SDL_QUIT:
				handler_->handle_quit_event();
				break;
			}
		}
		vibra_events();
		handler_->handle_flush_events();
	}
	const InputModifierState *get_modifier_state() const {
		return &st_;
	}
	void set_title(const char *title) {
		SDL_SetWindowTitle(window_, title);
	}
	void set_icon(int width, int height, const unsigned char *data) {
		SDL_Surface *icon = SDL_CreateRGBSurfaceWithFormat(0, width, height, 32, SDL_PIXELFORMAT_ABGR8888);
		if (!icon)
			return;
		SDL_LockSurface(icon);
		memcpy(icon->pixels, data, width * height * 4);
		SDL_UnlockSurface(icon);
		SDL_SetWindowIcon(window_, icon);
		SDL_FreeSurface(icon);
	}
	void set_pos(ivec2 pos) {
		SDL_SetWindowPosition(window_, pos.x, pos.y);
	}
	ivec2 get_pos() const {
		ivec2 pos;
		SDL_GetWindowPosition(window_, &pos.x, &pos.y);
		return pos;
	}
	void set_size(ivec2 size) {
		SDL_SetWindowSize(window_, size.x, size.y);
	}
	ivec2 get_size() const {
		ivec2 size;
		SDL_GetWindowSize(window_, &size.x, &size.y);
		return size;
	}
	void set_fullscreen(bool enable) {
		SDL_SetWindowFullscreen(window_, enable ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0); // TODO: hint
	}
	bool is_fullscreen() const {
		return SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	void set_always_on_top(bool enable) {
		// NOT IMPLEMENTED
	}
	bool is_always_on_top() const {
		return SDL_GetWindowFlags(window_) & SDL_WINDOW_ALWAYS_ON_TOP;
	}
	void set_resizable(bool enable) {
		SDL_SetWindowResizable(window_, enable ? SDL_TRUE : SDL_FALSE);
	}
	bool is_resizable() const {
		return SDL_GetWindowFlags(window_) & SDL_WINDOW_RESIZABLE;
	}
	void set_maximized(bool enable) {
		if (enable)
			SDL_MaximizeWindow(window_);
		else
			SDL_RestoreWindow(window_);
	}
	bool is_maximized() const {
		return SDL_GetWindowFlags(window_) & SDL_WINDOW_MAXIMIZED;
	}
	void set_minimized(bool enable) {
		if (enable)
			SDL_MinimizeWindow(window_);
		else
			SDL_RestoreWindow(window_);
	}
	bool is_minimized() const {
		return SDL_GetWindowFlags(window_) & SDL_WINDOW_MINIMIZED;
	}
	bool can_draw() const {
		return !is_minimized();
	}
	void set_mouse_mode(MouseMode mouse_mode) {
		switch (mouse_mode) {
		case MouseVisible:
			SDL_CaptureMouse(SDL_FALSE);
			SDL_ShowCursor(1);
			break;
		case MouseHidden:
			SDL_CaptureMouse(SDL_FALSE);
			SDL_ShowCursor(0);
			break;
		case MouseCaptured:
			SDL_ShowCursor(0);
			SDL_CaptureMouse(SDL_TRUE);
			break;
		}
		mouse_mode_ = mouse_mode;
	}
	MouseMode get_mouse_mode() const {
		return mouse_mode_;
	}
	ivec2 get_screen_size() const {
		ivec2 size = { 1280, 720 };
		SDL_DisplayMode mode;
		if (!SDL_GetCurrentDisplayMode(0, &mode)) {
			size.x = mode.w;
			size.y = mode.h;
		}
		return size;
	}
	float get_screen_refresh_rate() const {
		SDL_DisplayMode mode;
		if (SDL_GetCurrentDisplayMode(0, &mode))
			return 60.0f;
		return (float)mode.refresh_rate;
	}
	int get_screen_dpi() const {
		float dpi;
		if (!SDL_GetDisplayDPI(0, &dpi, 0, 0))
			return (int)dpi;
		return 72;
	}
	void js_vibra(int id, float x, float y, float duration, uint64_t timestamp) {
		int low = (int)(x * 0xffff);
		int high = (int)(y * 0xffff);
		int ms = (int)(duration * 1000);
		rumble_timestamp_[id] = timestamp;
		if (SDL_JoystickRumble(js_[id], low, high, ms))
			rumble_supported_ &= ~(1 << id);
	}
};

} // namespace frt
