// frt.h
/*
 * FRT - A Godot platform targeting single board computers
 * Copyright (c) 2017  Emanuele Fornara
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FRT_H
#define FRT_H

#define FRT_WINDOW_TITLE "Godot / FRT"
#define FRT_VERSION "0.9.2"

class ContextGL;
struct InputModifierState;

namespace frt {

struct Module {
	virtual const char *get_id() const = 0;
	virtual bool probe() = 0;
	virtual void cleanup() = 0;
	virtual bool handle_meta(int gd_code, bool pressed) { return false; }
};

struct EventDispatcher {
	virtual void dispatch_events() = 0;
};

struct EventHandler {
	virtual void handle_event() = 0;
};

struct Value {
	enum Type {
		Bool,
		Int,
		Float,
		String,
	} t;
	union {
		bool b;
		int i;
		float f;
		const char *s;
	} u;
	Value(bool v)
		: t(Bool) { u.b = v; }
	Value(int v)
		: t(Int) { u.i = v; }
	Value(float v)
		: t(Float) { u.f = v; }
	Value(const char *v)
		: t(String) { u.s = v; }
};

struct Param {
	const char *name;
	Value value;
	enum Source {
		Default,
		CommandLine,
		ProjectSettings,
	} source;
	Param(const char *name_, Value value_)
		: name(name_), value(value_), source(Default) {}
};

class App {
public:
	static const int max_modules = 30;
	static const int max_contexts = 10;
	static const int max_dispatchers = 10;
	void register_(Module *module);
	int size() const { return nm; }
	Module *get(int i) const { return m[i]; }
	Module *get(const char *id) const;
	Module *probe(const char *ids[]);
	Module *probe_single();
	Module *probe(const char *id) {
		const char *ids[] = { id, 0 };
		return probe(ids);
	}
	void **get_context(const char *key);
	void add_dispatcher(EventDispatcher *dispatcher);
	void remove_dispatcher(EventDispatcher *dispatcher);
	void dispatch_events();
	void quit() { running = false; }
	bool is_running() const { return running; }
	void parse_args(int *argc, char ***argv);
	int get_n_of_params() const;
	Param *get_param(int i) const;
	Param *get_param(const char *name) const;
	bool get_bool_param(const char *name) const {
		return get_param(name)->value.u.b;
	}
	int get_int_param(const char *name) const {
		return get_param(name)->value.u.i;
	}
	float get_float_param(const char *name) const {
		return get_param(name)->value.u.f;
	}
	const char *get_string_param(const char *name) const {
		return get_param(name)->value.u.s;
	}
	static App *instance();

private:
	App()
		: nm(0), nc(0), nd(0), running(true) {}
	Module *m[max_modules];
	struct {
		const char *key;
		void *value;
	} c[max_contexts];
	EventDispatcher *d[max_dispatchers];
	int nm, nc, nd;
	bool running;
};

struct RegisterModule {
	RegisterModule(Module *module) {
		App::instance()->register_(module);
	}
};

#define FRT_REGISTER(C)    \
	static C C##_instance; \
	static frt::RegisterModule C##_helper(&C##_instance);

struct Vec2 {
	int x;
	int y;
	Vec2(int x_ = 0, int y_ = 0)
		: x(x_), y(y_) {}
};

struct Video : public Module {
	virtual Vec2 get_screen_size() const = 0;
	virtual Vec2 get_view_size() const = 0;
	virtual Vec2 move_pointer(const Vec2 &screen) = 0;
	virtual void show_pointer(bool enable) = 0;
	virtual ContextGL *create_the_gl_context(int version, Vec2 view) = 0;
};

struct Keyboard : public Module {
	struct Handler {
		virtual void handle_keyboard_key(int gd_code, bool pressed) = 0;
	};
	virtual void set_handler(Handler *handler) = 0;
	virtual void get_modifier_state(InputModifierState &state) const = 0;
};

struct Mouse : public Module {
	enum Button {
		Left,
		Middle,
		Right,
		WheelUp,
		WheelDown
	};
	struct Handler {
		virtual void handle_mouse_button(Button button, bool pressed) = 0;
		virtual void handle_mouse_motion(Vec2 pos) = 0;
	};
	virtual Vec2 get_pos() const = 0;
	virtual void set_size(Vec2 size) = 0;
	virtual void set_handler(Handler *handler) = 0;
};

struct Env {
	Video *video;
	Keyboard *keyboard;
	Mouse *mouse;
};

struct EnvProbe : public Module {
	virtual void probe_env(Env *env) = 0;
};

struct Runnable : public Module {
	virtual void setup_env(Env *env) = 0;
	virtual void run_() = 0;
	virtual int get_exit_code_() = 0;
};

} // namespace frt

#ifdef FRT_MOCK_GODOT_GL_CONTEXT

enum Error {
	OK,
	FAILED
};

class ContextGL {
public:
	virtual void release_current() = 0;
	virtual void make_current() = 0;
	virtual void swap_buffers() = 0;
	virtual int get_window_width() = 0;
	virtual int get_window_height() = 0;
	virtual Error initialize() = 0;
	virtual void set_use_vsync(bool use) = 0;
	virtual bool is_using_vsync() const = 0;
};

#endif // FRT_MOCK_GODOT_GL_CONTEXT

#ifdef FRT_MOCK_GODOT_INPUT_MODIFIER_STATE

struct InputModifierState {
	bool shift;
	bool alt;
	bool control;
	bool meta;
};

#endif // FRT_MOCK_GODOT_INPUT_MODIFIER_STATE

#ifdef FRT_TEST
#define FRT_GLES_VERSION 2
#else
#include "version.h"
#define FRT_GLES_VERSION VERSION_MAJOR
#endif // FRT_TEST

#endif // FRT_H
