# detect.py
#
# FRT - A Godot platform targeting single board computers
# Copyright (c) 2017-2025  Emanuele Fornara
# SPDX-License-Identifier: MIT
#

import os
import sys
import platform
import version

def get_name():
	return "FRT"

def is_active():
	return True

def can_build():
	import os
	if os.name != "posix":
		return False
	return True

def get_opts():
	from SCons.Variables import BoolVariable
	return [
		BoolVariable('use_llvm', 'Use llvm compiler', False),
		BoolVariable('use_lto', 'Use link time optimization', False),
		BoolVariable('use_static_cpp', 'Link libgcc and libstdc++ statically', False),
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
	if env['use_llvm'] or not env['lto'] or env['lto'] == 'none':
		return
	env.Append(CCFLAGS=['-flto'])
	env.Append(LINKFLAGS=['-flto'])
	env['AR'] = 'gcc-ar'
	env['RANLIB'] = 'gcc-ranlib'
	env.extra_suffix += '.lto'

def configure_target(env):
	if env['target'] == 'release':
		env.Append(CCFLAGS=['-O3', '-ffast-math', '-fomit-frame-pointer'])
	elif env['target'] == 'release_debug':
		env.Append(CCFLAGS=['-O2'])
	elif env['target'] == 'debug':
		env.Append(CCFLAGS=['-g2'])

def configure_misc(env):
	env.Append(CPPPATH=['#platform/frt'])
	env.Append(CPPFLAGS=['-DUNIX_ENABLED', '-DGLES2_ENABLED', '-DGLES_ENABLED', '-DJOYDEV_ENABLED'])
	env.Append(CPPFLAGS=['-DFRT_ENABLED'])
	env.Append(CFLAGS=['-std=gnu11']) # for libwebp (maybe more in the future)
	env.Append(LIBS=['pthread', 'z', 'dl'])
	if env['use_static_cpp']:
		env.Append(LINKFLAGS=['-static-libgcc', '-static-libstdc++'])
	env['ENV']['PATH'] = os.getenv('PATH')
	env['ENV']['LD_LIBRARY_PATH'] = os.getenv('LD_LIBRARY_PATH')

def configure(env):
	configure_compiler(env)
	configure_lto(env)
	configure_target(env)
	configure_misc(env)
