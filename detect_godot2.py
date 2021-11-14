# detect_godot2.py
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2021  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

import os
import sys
import platform

def get_opts():
	return [
		('use_llvm', 'Use llvm compiler', 'no'),
		('use_lto', 'Use link time optimization', 'no'),
	]

def get_flags():
	return [
	]

def configure(env):
	env.Append(CPPPATH=['#platform/frt'])
	if (env["use_llvm"] == "yes"):
		env["CC"] = "clang"
		env["CXX"] = "clang++"
		env["LD"] = "clang++"

	if (env["use_lto"] == "yes"):
		# TODO: gcc?
		env.Append(CCFLAGS=['-flto=thin'])
		env.Append(LINKFLAGS=['-fuse-ld=lld', '-flto=thin'])
		env['AR'] = 'llvm-ar'
		env['RANLIB'] = 'llvm-ranlib'

	if platform.machine() != "x86_64":
		env.Append(CCFLAGS=['-march=armv7-a', '-mfpu=neon-vfpv4', '-mfloat-abi=hard'])

	if (env["target"] == "release"):
		env.Append(CCFLAGS=['-O2', '-ffast-math', '-fomit-frame-pointer'])
	elif (env["target"] == "release_debug"):
		env.Append(CCFLAGS=['-O2', '-ffast-math', '-DDEBUG_ENABLED'])
	elif (env["target"] == "debug"):
		env.Append(CCFLAGS=['-g2', '-DDEBUG_ENABLED', '-DDEBUG_MEMORY_ENABLED'])

	env.Append(CCFLAGS=['-D_REENTRANT']) # sdl2
	if (env['builtin_openssl'] == 'no'):
		env.ParseConfig('pkg-config openssl --cflags --libs')
	if (env['builtin_libwebp'] == 'no'):
		env.ParseConfig('pkg-config libwebp --cflags --libs')
	if (env['builtin_freetype'] == 'no'):
		env.ParseConfig('pkg-config freetype2 --cflags --libs')
	if (env['builtin_libpng'] == 'no'):
		env.ParseConfig('pkg-config libpng --cflags --libs')
	if (env['builtin_libtheora'] == 'no'):
		env.ParseConfig('pkg-config theora theoradec --cflags --libs')
	if (env['builtin_libvorbis'] == 'no'):
		env.ParseConfig('pkg-config vorbis vorbisfile --cflags --libs')
	if (env['builtin_opus'] == 'no'):
		env.ParseConfig('pkg-config opus opusfile --cflags --libs')
	if (env['builtin_libogg'] == 'no'):
		env.ParseConfig('pkg-config ogg --cflags --libs')
	env.Append(CPPFLAGS=['-DUNIX_ENABLED', '-DGLES2_ENABLED'])
	env.Append(CPPFLAGS=['-DFRT_ENABLED', '-DFRT_GODOT_VERSION=2', '-D_REENTRANT'])
	env.Append(LIBS=['dl', 'pthread', 'z'])
	if (env["CXX"] == "clang++"):
		env.Append(CPPFLAGS=['-DTYPED_METHOD_BIND'])
		env["CC"] = "clang"
		env["LD"] = "clang++"
