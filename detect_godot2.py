# detect_godot2.py
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2022  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

import os
import sys
import platform

def get_opts():
	return [
		('use_llvm', 'Use llvm compiler', 'no'),
		('use_lto', 'Use link time optimization', 'no'),
		('use_static_cpp', 'Link libgcc and libstdc++ statically', 'no'),
		('frt_std', 'C++ standard for frt itself (no/auto/c++98/...)', 'auto'),
		('frt_arch', 'Architecture (no/arm32v6/arm32v7/arm64v8)', 'no'),
		('frt_cross', 'Cross compilation (no/auto/<triple>)', 'no'),
	]

def get_flags():
	return [
	]

def configure_compiler(env):
	if env['use_llvm'] == 'yes':
		env['CC'] = 'clang'
		env['CXX'] = 'clang++'
		env['LD'] = 'clang++'
		env.extra_suffix += '.llvm'
	else:
		print('Newer GCC compilers not supported in Godot 2. Handle with care.')

def configure_lto(env):
	if env['use_lto'] == 'no':
		return
	if env['use_llvm'] == 'yes':
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
	if env['use_llvm'] == 'yes':
		env.Append(CCFLAGS=['-target', triple])
		env.Append(LINKFLAGS=['-target', triple])
	else:
		env['CC'] = triple + '-gcc'
		env['CXX'] = triple + '-g++'

def configure_glsl_builders(env):
	import methods
	env.Append(BUILDERS={'GLSL120': env.Builder(action=methods.build_legacygl_headers, suffix='glsl.gen.h', src_suffix='.glsl')})
	env.Append(BUILDERS={'GLSL': env.Builder(action=methods.build_glsl_headers, suffix='glsl.gen.h', src_suffix='.glsl')})
	env.Append(BUILDERS={'GLSL120GLES': env.Builder(action=methods.build_gles2_headers, suffix='glsl.gen.h', src_suffix='.glsl')})

def configure_target(env):
	if env['target'] == 'release':
		env.Append(CCFLAGS=['-O2', '-ffast-math', '-fomit-frame-pointer'])
	elif env['target'] == 'release_debug':
		env.Append(CCFLAGS=['-O2', '-ffast-math', '-DDEBUG_ENABLED'])
	elif env['target'] == 'debug':
		env.Append(CCFLAGS=['-g2', '-DDEBUG_ENABLED', '-DDEBUG_MEMORY_ENABLED'])

def configure_deps(env):
	# TODO: triple-pkg-config - everything static, sdl2 overridden anyway
	env.Append(CCFLAGS=['-D_REENTRANT']) # sdl2 - TODO: really needed?
	if env['builtin_openssl'] == 'no':
		env.ParseConfig('pkg-config openssl --cflags --libs')
	if env['builtin_libwebp'] == 'no':
		env.ParseConfig('pkg-config libwebp --cflags --libs')
	if env['builtin_freetype'] == 'no':
		env.ParseConfig('pkg-config freetype2 --cflags --libs')
	if env['builtin_libpng'] == 'no':
		env.ParseConfig('pkg-config libpng --cflags --libs')
	if env['builtin_libtheora'] == 'no':
		env.ParseConfig('pkg-config theora theoradec --cflags --libs')
	if env['builtin_libvorbis'] == 'no':
		env.ParseConfig('pkg-config vorbis vorbisfile --cflags --libs')
	if env['builtin_opus'] == 'no':
		env.ParseConfig('pkg-config opus opusfile --cflags --libs')
	if env['builtin_libogg'] == 'no':
		env.ParseConfig('pkg-config ogg --cflags --libs')

def configure_misc(env):
	env.Append(CPPPATH=['#platform/frt'])
	env.Append(CPPFLAGS=['-DUNIX_ENABLED', '-DGLES2_ENABLED', '-DJOYDEV_ENABLED'])
	env.Append(CPPFLAGS=['-DFRT_ENABLED'])
	env.Append(LIBS=['pthread', 'z'])
	if env['CXX'] == 'clang++':
		env.Append(CPPFLAGS=['-DTYPED_METHOD_BIND'])
		env['CC'] = 'clang'
		env['LD'] = 'clang++'
	if env['use_static_cpp'] == 'yes':
		env.Append(LINKFLAGS=['-static-libgcc', '-static-libstdc++'])

def configure(env):
	configure_compiler(env)
	configure_lto(env)
	configure_arch(env)
	configure_cross(env)
	configure_glsl_builders(env)
	configure_target(env)
	configure_deps(env)
	configure_misc(env)
