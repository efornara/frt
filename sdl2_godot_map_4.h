// sdl2_godot_map_4.h
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2023  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "core/version.h"

#define FRT_GODOT_VERSION ((((VERSION_MAJOR * 100) + VERSION_MINOR) * 100) + VERSION_PATCH)

#include "core/os/os.h"
#include "core/os/keyboard.h"
#include "servers/display_server.h"

namespace frt {

::MouseButton map_mouse_os_button(int os_button) {
	switch (os_button) {
	case ButtonLeft:
		return ::MouseButton::LEFT;
	case ButtonRight:
		return ::MouseButton::RIGHT;
	case ButtonMiddle:
		return ::MouseButton::MIDDLE;
	case WheelUp:
		return ::MouseButton::WHEEL_UP;
	case WheelDown:
		return ::MouseButton::WHEEL_DOWN;
	default:
		fatal("unexpected mouse button: %d", os_button);
	}
}

DisplayServer::MouseMode map_mouse_os_mode(MouseMode os_mode) {
	switch (os_mode) {
	case MouseVisible:
		return DisplayServer::MOUSE_MODE_VISIBLE;
	case MouseHidden:
		return DisplayServer::MOUSE_MODE_HIDDEN;
	case MouseCaptured:
		return DisplayServer::MOUSE_MODE_CAPTURED;
	default:
		fatal("unexpected mouse mode: %d", os_mode);
	}
}

MouseMode map_mouse_mode(DisplayServer::MouseMode mode) {
	switch (mode) {
	case DisplayServer::MOUSE_MODE_VISIBLE:
		return MouseVisible;
	case DisplayServer::MOUSE_MODE_HIDDEN:
		return MouseHidden;
	case DisplayServer::MOUSE_MODE_CAPTURED:
	case DisplayServer::MOUSE_MODE_CONFINED:
		return MouseCaptured;
	default: // NOT REACHED
		return MouseVisible;
	}
}

#if FRT_GODOT_VERSION < 40000
int map_hat_os_mask(int os_mask) {
	int mask = 0;
	if (os_mask & HatUp)
		mask |= InputDefault::HAT_MASK_UP;
	if (os_mask & HatRight)
		mask |= InputDefault::HAT_MASK_RIGHT;
	if (os_mask & HatDown)
		mask |= InputDefault::HAT_MASK_DOWN;
	if (os_mask & HatLeft)
		mask |= InputDefault::HAT_MASK_LEFT;
	return mask;
}
#endif

struct KeyMap {
	int sdl2_code;
	Key gd_code;
} keymap[] = {
	{ SDLK_SPACE, (Key)' ' },
	{ SDLK_a, (Key)'A' },
	{ SDLK_b, (Key)'B' },
	{ SDLK_c, (Key)'C' },
	{ SDLK_d, (Key)'D' },
	{ SDLK_e, (Key)'E' },
	{ SDLK_f, (Key)'F' },
	{ SDLK_g, (Key)'G' },
	{ SDLK_h, (Key)'H' },
	{ SDLK_i, (Key)'I' },
	{ SDLK_j, (Key)'J' },
	{ SDLK_k, (Key)'K' },
	{ SDLK_l, (Key)'L' },
	{ SDLK_m, (Key)'M' },
	{ SDLK_n, (Key)'N' },
	{ SDLK_o, (Key)'O' },
	{ SDLK_p, (Key)'P' },
	{ SDLK_q, (Key)'Q' },
	{ SDLK_r, (Key)'R' },
	{ SDLK_s, (Key)'S' },
	{ SDLK_t, (Key)'T' },
	{ SDLK_u, (Key)'U' },
	{ SDLK_v, (Key)'V' },
	{ SDLK_w, (Key)'W' },
	{ SDLK_x, (Key)'X' },
	{ SDLK_y, (Key)'Y' },
	{ SDLK_z, (Key)'Z' },
	{ SDLK_0, (Key)'0' },
	{ SDLK_1, (Key)'1' },
	{ SDLK_2, (Key)'2' },
	{ SDLK_3, (Key)'3' },
	{ SDLK_4, (Key)'4' },
	{ SDLK_5, (Key)'5' },
	{ SDLK_6, (Key)'6' },
	{ SDLK_7, (Key)'7' },
	{ SDLK_8, (Key)'8' },
	{ SDLK_9, (Key)'9' },
	{ SDLK_F1, Key::F1 },
	{ SDLK_F2, Key::F2 },
	{ SDLK_F3, Key::F3 },
	{ SDLK_F4, Key::F4 },
	{ SDLK_F5, Key::F5 },
	{ SDLK_F6, Key::F6 },
	{ SDLK_F7, Key::F7 },
	{ SDLK_F8, Key::F8 },
	{ SDLK_F9, Key::F9 },
	{ SDLK_F10, Key::F10 },
	{ SDLK_F11, Key::F11 },
	{ SDLK_F12, Key::F12 },
	{ SDLK_UP, Key::UP },
	{ SDLK_DOWN, Key::DOWN },
	{ SDLK_LEFT, Key::LEFT },
	{ SDLK_RIGHT, Key::RIGHT },
	{ SDLK_TAB, Key::TAB },
	{ SDLK_BACKSPACE, Key::BACKSPACE },
	{ SDLK_INSERT, Key::INSERT },
	{ SDLK_DELETE, Key::KEY_DELETE },
	{ SDLK_HOME, Key::HOME },
	{ SDLK_END, Key::END },
	{ SDLK_PAGEUP, Key::PAGEUP },
	{ SDLK_PAGEDOWN, Key::PAGEDOWN },
	{ SDLK_RETURN, Key::ENTER },
	{ SDLK_ESCAPE, Key::ESCAPE },
	{ SDLK_LCTRL, Key::CTRL },
	{ SDLK_RCTRL, Key::CTRL },
	{ SDLK_LALT, Key::ALT },
	{ SDLK_RALT, Key::ALT },
	{ SDLK_LSHIFT, Key::SHIFT },
	{ SDLK_RSHIFT, Key::SHIFT },
	{ SDLK_LGUI, Key::META },
	{ SDLK_RGUI, Key::META },
	{ SDLK_KP_0, Key::KP_0 },
	{ SDLK_KP_1, Key::KP_1 },
	{ SDLK_KP_2, Key::KP_2 },
	{ SDLK_KP_3, Key::KP_3 },
	{ SDLK_KP_4, Key::KP_4 },
	{ SDLK_KP_5, Key::KP_5 },
	{ SDLK_KP_6, Key::KP_6 },
	{ SDLK_KP_7, Key::KP_7 },
	{ SDLK_KP_8, Key::KP_8 },
	{ SDLK_KP_9, Key::KP_9 },
	{ SDLK_KP_MULTIPLY, Key::KP_MULTIPLY },
	{ SDLK_KP_MINUS, Key::KP_SUBTRACT },
	{ SDLK_KP_PLUS, Key::KP_ADD },
	{ SDLK_KP_PERIOD, Key::KP_PERIOD },
	{ SDLK_KP_ENTER, Key::KP_ENTER },
	{ SDLK_KP_DIVIDE, Key::KP_DIVIDE },
	{ 0, Key::NONE },
};

Key map_key_sdl2_code(int sdl2_code) {
	for (int i = 0; keymap[i].sdl2_code; i++) {
		if (keymap[i].sdl2_code == sdl2_code)
			return keymap[i].gd_code;
	}
	return Key::NONE;
}

} // namespace frt
