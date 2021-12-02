// sdl2_adapter.h
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2021  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

namespace frt {

struct SDL2SampleProducer {
	virtual void produce_samples(int n_of_frames, int32_t *frames) = 0;
	virtual ~SDL2SampleProducer();
};

SDL2SampleProducer::~SDL2SampleProducer() {
}

void audio_callback(void *userdata, Uint8 *stream, int len);

class SDL2Audio {
private:
	SDL2SampleProducer *producer_;
	Mutex *mutex_;
	int32_t *samples_;
	int n_of_samples_;
public:
	SDL2Audio(SDL2SampleProducer *producer) : producer_(producer) {
		mutex_ = 0;
	}
	Error init(int mix_rate, int latency) {
		SDL_AudioSpec desired, obtained;
		memset(&desired, 0, sizeof(desired));
		desired.freq = mix_rate;
		desired.format = AUDIO_S16;
		desired.channels = 2;
		desired.samples = closest_power_of_2(latency * mix_rate / 1000);
		desired.callback = audio_callback;
		desired.userdata = this;
		if (!SDL_OpenAudio(&desired, &obtained)) {
			mutex_ = Mutex::create();
			n_of_samples_ = obtained.channels * obtained.samples;
			samples_ = new int32_t[n_of_samples_];
			return OK;
		} else {
			return ERR_CANT_OPEN; // TODO: warn?
		}
	}
	void start() {
		SDL_PauseAudio(SDL_FALSE);
	}
	void lock() {
		mutex_->lock();
	}
	void unlock() {
		mutex_->unlock();
	}
	void finish() {
		SDL_PauseAudio(SDL_TRUE);
		SDL_LockAudio();
		// calling of sdl2 callback not expected after pause+lock
		memdelete(mutex_);
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
		mutex_->lock();
		producer_->produce_samples(n_of_samples / channels, samples_);
		mutex_->unlock();
		int16_t *data16 = (int16_t *)data; // assume alignment is fine
		for (int i = 0; i < n_of_samples; i++)
			data16[i] = samples_[i] >> 16;
	}
};

void audio_callback(void *userdata, Uint8 *stream, int len) {
	SDL2Audio *audio = (SDL2Audio *)userdata;
	audio->fill_buffer(stream, len);
}

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
	virtual void handle_resize_event(int width, int height) = 0;
	virtual void handle_key_event(int gd_code, bool pressed) = 0;
	virtual void handle_js_status_event(int id, bool connected, String name, String guid) = 0;
	virtual void handle_js_button_event(int id, int button, bool pressed) = 0;
	virtual void handle_js_axis_event(int id, int axis, float value) = 0;
	virtual void handle_js_hat_event(int id, int value) = 0;
	virtual void handle_quit_event() = 0;
};

SDL2EventHandler::~SDL2EventHandler() {
}

class SDL2OS {
private:
	static const int MAX_JOYSTICKS = 16;
	SDL_Window *window_;
	SDL_GLContext context_;
	SDL2EventHandler *handler_;
	InputModifierState st_;
	SDL_Joystick *js_[MAX_JOYSTICKS];
	void resize_event(const SDL_Event &ev) {
		int width, height;
		SDL_GL_GetDrawableSize(window_, &width, &height);
		handler_->handle_resize_event(width, height);
	}
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
			float value = ev.jaxis.value / 32768.0;
			handler_->handle_js_axis_event(id, axis, value);
			} break;
		case SDL_JOYHATMOTION: {
			if ((id = get_js_id(ev.jhat.which)) < 0)
				return;
			if (ev.jhat.hat != 0)
				return;
			int value = 0;
			switch (ev.jhat.value) {
			case SDL_HAT_LEFT:
				value = InputDefault::HAT_MASK_LEFT;
				break;
			case SDL_HAT_LEFTUP:
				value = InputDefault::HAT_MASK_LEFT | InputDefault::HAT_MASK_UP;
				break;
			case SDL_HAT_UP:
				value = InputDefault::HAT_MASK_UP;
				break;
			case SDL_HAT_RIGHTUP:
				value = InputDefault::HAT_MASK_RIGHT | InputDefault::HAT_MASK_UP;
				break;
			case SDL_HAT_RIGHT:
				value = InputDefault::HAT_MASK_RIGHT;
				break;
			case SDL_HAT_RIGHTDOWN:
				value = InputDefault::HAT_MASK_RIGHT | InputDefault::HAT_MASK_DOWN;
				break;
			case SDL_HAT_DOWN:
				value = InputDefault::HAT_MASK_DOWN;
				break;
			case SDL_HAT_LEFTDOWN:
				value = InputDefault::HAT_MASK_LEFT | InputDefault::HAT_MASK_DOWN;
				break;
			}
			handler_->handle_js_hat_event(id, value);
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
			int id = ev.jdevice.which;
			if (id >= MAX_JOYSTICKS)
				return;
			String name = SDL_JoystickNameForIndex(id);
			char buf[64];
			SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(id), buf, sizeof(buf));
			String guid = buf;
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
	SDL2OS(SDL2EventHandler *handler) : handler_(handler) {
		st_.shift = false;
		st_.alt = false;
		st_.control = false;
		st_.meta = false;
		memset(js_, 0, sizeof(js_));
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
	void get_modifier_state(InputModifierState &state) const {
		state = st_;
	}
	void set_title(const String &title) {
		SDL_SetWindowTitle(window_, title.utf8().get_data());
	}
	void set_pos(int x, int h) {
		SDL_SetWindowPosition(window_, x, h);
	}
	void get_pos(int *x, int *h) const {
		SDL_GetWindowPosition(window_, x, h);
	}
	void set_size(int width, int height) {
		SDL_SetWindowSize(window_, width, height);
	}
	void get_size(int *width, int *height) const {
		SDL_GetWindowSize(window_, width, height);
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
};

} // namespace frt
