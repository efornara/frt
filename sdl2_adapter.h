// sdl2_adapter.h
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2021  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include <SDL.h>
#include "dl/gles2.gen.h"

namespace frt {

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
	virtual void handle_key_event(int sdl2_code, bool pressed) = 0;
	virtual void handle_mouse_motion_event(ivec2 pos, ivec2 dpos) = 0;
	virtual void handle_mouse_button_event(int button, bool pressed, bool doubleclick) = 0;
	virtual void handle_js_status_event(int id, bool connected, const char *name, const char *guid) = 0;
	virtual void handle_js_button_event(int id, int button, bool pressed) = 0;
	virtual void handle_js_axis_event(int id, int axis, float value) = 0;
	virtual void handle_js_hat_event(int id, int mask) = 0;
	virtual void handle_quit_event() = 0;
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

class OS_FRT {
private:
	static const int MAX_JOYSTICKS = 16;
	SDL_Window *window_;
	SDL_GLContext context_;
	EventHandler *handler_;
	InputModifierState st_;
	MouseMode mouse_mode_;
	SDL_Joystick *js_[MAX_JOYSTICKS];
	bool exit_shortcuts_;
	void resize_event(const SDL_Event &ev) {
		ivec2 size;
		SDL_GL_GetDrawableSize(window_, &size.x, &size.y);
		handler_->handle_resize_event(size);
	}
	void key_event(const SDL_Event &ev) {
		st_.shift = ev.key.keysym.mod & KMOD_SHIFT;
		st_.alt = ev.key.keysym.mod & KMOD_ALT;
		st_.control = ev.key.keysym.mod & KMOD_CTRL;
		st_.meta = ev.key.keysym.mod & KMOD_GUI;
		if (ev.key.repeat)
			return;
		bool pressed = ev.key.state == SDL_PRESSED;
		if (exit_shortcuts_ && st_.alt && pressed && ev.key.keysym.sym == SDLK_KP_ENTER)
			fatal("exit_shortcut (alt+enter), disable by setting FRT_NO_EXIT_SHORTCUTS");
		int sdl2_code = ev.key.keysym.sym;
		handler_->handle_key_event(sdl2_code, pressed);
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
			if (exit_shortcuts_ && button == 6 && pressed)
				fatal("exit_shortcut (joystick button #6), disable by setting FRT_NO_EXIT_SHORTCUTS");
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
public:
	OS_FRT(EventHandler *handler) : handler_(handler) {
		mouse_mode_ = MouseVisible;
		memset(js_, 0, sizeof(js_));
		exit_shortcuts_ = !getenv("FRT_NO_EXIT_SHORTCUTS");
	}
	void init(int width, int height, bool resizable, bool borderless, bool always_on_top) {
		if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0)
			fatal("SDL_Init failed.");
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
		int flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI;
		if (resizable)
			flags |= SDL_WINDOW_RESIZABLE;
		if (borderless)
			flags |= SDL_WINDOW_BORDERLESS;
		if (always_on_top)
			flags |= SDL_WINDOW_ALWAYS_ON_TOP;
		if (!(window_ = SDL_CreateWindow("frt2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags)))
			fatal("SDL_CreateWindow failed.");
		context_ = SDL_GL_CreateContext(window_);
		SDL_GL_MakeCurrent(window_, context_);
		frt_resolve_symbols_gles2(SDL_GL_GetProcAddress);
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
			case SDL_WINDOWEVENT:
				if (ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
					resize_event(ev);
				break;
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				key_event(ev);
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
	}
	const InputModifierState *get_modifier_state() const {
		return &st_;
	}
	void set_title(const char *title) {
		SDL_SetWindowTitle(window_, title);
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
		SDL_SetWindowFullscreen(window_, enable ? 1 : 0);
	}
	bool is_fullscreen() const {
		return SDL_GetWindowFlags(window_) & SDL_WINDOW_FULLSCREEN;
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
};

} // namespace frt
