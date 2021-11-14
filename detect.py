# detect.py
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2021  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

import version
if version.major == 2:
	from detect_godot2 import *
	version_handled = True
else:
	version_handled = False

def get_name():
	return "FRT"

def is_active():
	return True

def can_build():
	if os.name != "posix":
		return False
	if not version_handled:
		print("Error: Godot version not handled by FRT. Aborting.")
		return False
	if os.system("pkg-config --exists sdl2") != 0:
		print("Error: SDL2 not found. Aborting.")
		return False
	return True
