// sdl2_godot_mapping.h
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2023  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#include "core/version.h"

#define FRT_GODOT_VERSION ((((VERSION_MAJOR * 100) + VERSION_MINOR) * 100) + VERSION_PATCH)

#include "core/os/os.h"
#if FRT_GODOT_VERSION < 40000
#include "core/os/input.h"
#include "main/input_default.h"
#endif
#include "core/os/keyboard.h"

#if FRT_GODOT_VERSION >= 40000
#define KEYGD(x) (int)Key::x
#else
#define KEYGD(x) KEY_ ## x
#endif

namespace frt {

#if FRT_GODOT_VERSION < 40000
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
#endif

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
	{ SDLK_F1, KEYGD(F1) },
	{ SDLK_F2, KEYGD(F2) },
	{ SDLK_F3, KEYGD(F3) },
	{ SDLK_F4, KEYGD(F4) },
	{ SDLK_F5, KEYGD(F5) },
	{ SDLK_F6, KEYGD(F6) },
	{ SDLK_F7, KEYGD(F7) },
	{ SDLK_F8, KEYGD(F8) },
	{ SDLK_F9, KEYGD(F9) },
	{ SDLK_F10, KEYGD(F10) },
	{ SDLK_F11, KEYGD(F11) },
	{ SDLK_F12, KEYGD(F12) },
	{ SDLK_UP, KEYGD(UP) },
	{ SDLK_DOWN, KEYGD(DOWN) },
	{ SDLK_LEFT, KEYGD(LEFT) },
	{ SDLK_RIGHT, KEYGD(RIGHT) },
	{ SDLK_TAB, KEYGD(TAB) },
	{ SDLK_BACKSPACE, KEYGD(BACKSPACE) },
	{ SDLK_INSERT, KEYGD(INSERT) },
#if FRT_GODOT_VERSION >= 40000
	{ SDLK_DELETE, KEYGD(KEY_DELETE) },
#else
	{ SDLK_DELETE, KEYGD(DELETE) },
#endif
	{ SDLK_HOME, KEYGD(HOME) },
	{ SDLK_END, KEYGD(END) },
	{ SDLK_PAGEUP, KEYGD(PAGEUP) },
	{ SDLK_PAGEDOWN, KEYGD(PAGEDOWN) },
#if FRT_GODOT_VERSION >= 30000
	{ SDLK_RETURN, KEYGD(ENTER) },
#else
	{ SDLK_RETURN, KEYGD(RETURN) },
#endif
	{ SDLK_ESCAPE, KEYGD(ESCAPE) },
#if FRT_GODOT_VERSION >= 40000
	{ SDLK_LCTRL, KEYGD(CTRL) },
	{ SDLK_RCTRL, KEYGD(CTRL) },
#else
	{ SDLK_LCTRL, KEYGD(CONTROL) },
	{ SDLK_RCTRL, KEYGD(CONTROL) },
#endif
	{ SDLK_LALT, KEYGD(ALT) },
	{ SDLK_RALT, KEYGD(ALT) },
	{ SDLK_LSHIFT, KEYGD(SHIFT) },
	{ SDLK_RSHIFT, KEYGD(SHIFT) },
	{ SDLK_LGUI, KEYGD(META) },
	{ SDLK_RGUI, KEYGD(META) },
	{ SDLK_KP_0, KEYGD(KP_0) },
	{ SDLK_KP_1, KEYGD(KP_1) },
	{ SDLK_KP_2, KEYGD(KP_2) },
	{ SDLK_KP_3, KEYGD(KP_3) },
	{ SDLK_KP_4, KEYGD(KP_4) },
	{ SDLK_KP_5, KEYGD(KP_5) },
	{ SDLK_KP_6, KEYGD(KP_6) },
	{ SDLK_KP_7, KEYGD(KP_7) },
	{ SDLK_KP_8, KEYGD(KP_8) },
	{ SDLK_KP_9, KEYGD(KP_9) },
	{ SDLK_KP_MULTIPLY, KEYGD(KP_MULTIPLY) },
	{ SDLK_KP_MINUS, KEYGD(KP_SUBTRACT) },
	{ SDLK_KP_PLUS, KEYGD(KP_ADD) },
	{ SDLK_KP_PERIOD, KEYGD(KP_PERIOD) },
	{ SDLK_KP_ENTER, KEYGD(KP_ENTER) },
	{ SDLK_KP_DIVIDE, KEYGD(KP_DIVIDE) },
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

#undef KEYGD
