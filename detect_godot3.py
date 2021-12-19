# detect_godot3.py
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2021  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

import os
import sys
import platform

# TODO factor out common bits

def get_opts():
	return [
		('use_llvm', 'Use llvm compiler', False),
		('use_lto', 'Use link time optimization', False),
		('use_static_cpp', 'Link libgcc and libstdc++ statically', False),
		('frt_std', 'C++ standard for frt itself (no/auto/c++98/...)', 'auto'),
		('frt_arch', 'Architecture (no/arm32v6/arm32v7/arm64v8)', 'no'),
		('frt_cross', 'Cross compilation (no/auto/<triple>)', 'no'),
	]

def get_flags():
	return [
	]

def configure_compiler(env):
	if env['use_llvm']:
		env['CC'] = 'clang'
		env['CXX'] = 'clang++'
		env['LD'] = 'clang++'
		env.extra_suffix += '.llvm'

def configure_lto(env):
	if not env['use_lto']:
		return
	if env['use_llvm']:
		env.Append(CCFLAGS=['-flto=thin'])
		env.Append(LINKFLAGS=['-fuse-ld=lld', '-flto=thin'])
		env['AR'] = 'llvm-ar'
		env['RANLIB'] = 'llvm-ranlib'
	else:
		env.Append(CCFLAGS=['-flto'])
		env.Append(LINKFLAGS=['-flto'])
	env.extra_suffix += '.lto'

def configure_arch(env):
	if env['frt_arch'] == 'arm32v6':
		env.Append(CCFLAGS=['-march=armv6', '-mfpu=vfp', '-mfloat-abi=hard'])
		env.extra_suffix += '.arm32v6'
	elif env['frt_arch'] == 'arm32v7':
		env.Append(CCFLAGS=['-march=armv7-a', '-mfpu=neon-vfpv4', '-mfloat-abi=hard'])
		env.extra_suffix += '.arm32v7'
	elif env['frt_arch'] == 'arm64v8':
		env.Append(CCFLAGS=['-march=armv8-a'])
		env.extra_suffix += '.arm64v8'

def configure_cross(env):
	if env['frt_cross'] == 'no':
		return
	if env['frt_cross'] == 'auto':
		triple = {
			'arm32v7': 'arm-linux-gnueabihf',
			'arm64v8': 'aarch64-linux-gnu',
		}[env['frt_arch']]
	else:
		triple = env['frt_cross']
	if env['use_llvm']:
		env.Append(CCFLAGS=['-target', triple])
		env.Append(LINKFLAGS=['-target', triple])
	else:
		env['CC'] = triple + '-gcc'
		env['CXX'] = triple + '-g++'

def configure_target(env):
	if env['target'] == 'release':
		env.Append(CCFLAGS=['-O2', '-ffast-math', '-fomit-frame-pointer'])
	elif env['target'] == 'release_debug':
		env.Append(CCFLAGS=['-O2', '-ffast-math'])
	elif env['target'] == 'debug':
		env.Append(CCFLAGS=['-g2'])

def configure_deps(env):
	# TODO: triple-pkg-config - everything static, sdl2 overridden anyway
	env.Append(CCFLAGS=['-D_REENTRANT']) # sdl2 - TODO: really needed?
	# all build-in

def configure_misc(env):
	env.Append(CPPPATH=['#platform/frt'])
	env.Append(CPPFLAGS=['-DUNIX_ENABLED', '-DGLES2_ENABLED', '-DGLES_ENABLED', '-DJOYDEV_ENABLED'])
	env.Append(CPPFLAGS=['-DFRT_ENABLED'])
	env.Append(CFLAGS=['-std=gnu11']) # for libwebp (maybe more in the future)
	env.Append(LIBS=['pthread', 'z'])
	if env['CXX'] == 'clang++':
		env['CC'] = 'clang'
		env['LD'] = 'clang++'

def configure(env):
	configure_compiler(env)
	configure_lto(env)
	configure_arch(env)
	configure_cross(env)
	configure_target(env)
	configure_deps(env)
	configure_misc(env)
