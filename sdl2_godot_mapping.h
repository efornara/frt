// sdl2_godot_mapping.h
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2022  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "core/version.h"

#define FRT_GODOT_VERSION ((((VERSION_MAJOR * 100) + VERSION_MINOR) * 100) + VERSION_PATCH)

#include "core/os/os.h"
#include "core/os/input.h"
#include "core/os/keyboard.h"
#include "main/input_default.h"

namespace frt {

int map_mouse_os_button(int os_button) {
	switch (os_button) {
	case ButtonLeft:
		return BUTTON_LEFT;
	case ButtonRight:
		return BUTTON_RIGHT;
	case ButtonMiddle:
		return BUTTON_MIDDLE;
	case WheelUp:
		return BUTTON_WHEEL_UP;
	case WheelDown:
		return BUTTON_WHEEL_DOWN;
	default:
		fatal("unexpected mouse button: %d", os_button);
	}
}

OS::MouseMode map_mouse_os_mode(MouseMode os_mode) {
	switch (os_mode) {
	case MouseVisible:
		return OS::MOUSE_MODE_VISIBLE;
	case MouseHidden:
		return OS::MOUSE_MODE_HIDDEN;
	case MouseCaptured:
		return OS::MOUSE_MODE_CAPTURED;
	default:
		fatal("unexpected mouse mode: %d", os_mode);
	}
}

MouseMode map_mouse_mode(OS::MouseMode mode) {
	switch (mode) {
	case OS::MOUSE_MODE_VISIBLE:
		return MouseVisible;
	case OS::MOUSE_MODE_HIDDEN:
		return MouseHidden;
	case OS::MOUSE_MODE_CAPTURED:
#if FRT_GODOT_VERSION >= 30000
	case OS::MOUSE_MODE_CONFINED:
#endif
		return MouseCaptured;
	default: // NOT REACHED
		return MouseVisible;
	}
}

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
#if FRT_GODOT_VERSION >= 30000
	{ SDLK_RETURN, KEY_ENTER },
#else
	{ SDLK_RETURN, KEY_RETURN },
#endif
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

int map_key_sdl2_code(int sdl2_code) {
	for (int i = 0; keymap[i].sdl2_code; i++) {
		if (keymap[i].sdl2_code == sdl2_code)
			return keymap[i].gd_code;
	}
	return 0;
}

} // namespace frt
