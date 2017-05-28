
import os
import sys


def is_active():

	return True


def get_name():

	return "FRT"


def can_build():

	if (os.name != "posix"):
		return False
	return True


def get_opts():

	return [
		('frt_arch', 'Architecture (pc/pi1/pi2/pi3/mali)', 'pc'),
	]


def get_flags():

	return [
		('builtin_freetype', 'no'),
		('builtin_openssl', 'no'),
		('builtin_libpng', 'no'),
		('builtin_zlib', 'no'),
		('tools', 'no'),
	]


def configure(env):

	env.Append(CPPPATH=['#platform/frt'])

	if os.system("pkg-config --exists alsa") == 0:
		print("Enabling ALSA")
		env.Append(CPPFLAGS=["-DALSA_ENABLED"])
		env.ParseConfig('pkg-config alsa --cflags --libs')
	else:
		print("ALSA libraries not found, disabling driver")

	if (env['builtin_freetype'] == 'no'):
		env.ParseConfig('pkg-config freetype2 --cflags --libs')
	if (env['builtin_openssl'] == 'no'):
		env.ParseConfig('pkg-config openssl --cflags --libs')
	if (env['builtin_libpng'] == 'no'):
		env.ParseConfig('pkg-config libpng --cflags --libs')
	if (env['builtin_zlib'] == 'no'):
		env.ParseConfig('pkg-config zlib --cflags --libs')

	import methods

	env.Append(BUILDERS={'GLSL120': env.Builder(action=methods.build_legacygl_headers, suffix='glsl.h', src_suffix='.glsl')})
	env.Append(BUILDERS={'GLSL': env.Builder(action=methods.build_glsl_headers, suffix='glsl.h', src_suffix='.glsl')})
	env.Append(BUILDERS={'GLSL120GLES': env.Builder(action=methods.build_gles2_headers, suffix='glsl.h', src_suffix='.glsl')})

	if (env["target"] == "release"):
		env.Append(CCFLAGS=['-O2', '-ffast-math', '-fomit-frame-pointer'])
	elif (env["target"] == "release_debug"):
		env.Append(CCFLAGS=['-O2', '-ffast-math', '-DDEBUG_ENABLED'])
	elif (env["target"] == "debug"):
		env.Append(CCFLAGS=['-g2', '-DDEBUG_ENABLED', '-DDEBUG_MEMORY_ENABLED'])

	if (env["frt_arch"] == "pi1"):
		env.Append(CCFLAGS=['-mcpu=arm1176jzf-s', '-mfpu=vfp'])
		env.extra_suffix += ".pi1"
	elif (env["frt_arch"] == "pi2"):
		env.Append(CCFLAGS=['-mcpu=cortex-a7', '-mfpu=neon-vfpv4'])
		env.extra_suffix += ".pi2"
	elif (env["frt_arch"] == "pi3"):
		env.Append(CCFLAGS=['-mcpu=cortex-a53', '-mfpu=neon-fp-armv8'])
		env.extra_suffix += ".pi3"
	elif (env["frt_arch"] == "mali"):
		env.extra_suffix += ".mali"

	if (env["frt_arch"].startswith("pi")):
		env.Append(CCFLAGS=['-mfloat-abi=hard', '-mlittle-endian', '-munaligned-access'])

	env.Append(CPPFLAGS=['-DUNIX_ENABLED', '-DGLES2_ENABLED'])
	env.Append(LIBS=['pthread'])

	if (env["frt_arch"] == "pc"):
		env.Append(FRT_MODULES=['video_sdl2.cpp', 'keyboard_sdl2.cpp', 'mouse_sdl2.cpp'])
		env.Append(LIBS=['GLESv2', 'SDL2'])
	elif (env["frt_arch"] == "mali"):
		env.Append(FRT_MODULES=['video_mali.cpp', 'keyboard_linux_input.cpp', 'mouse_linux_input.cpp'])
		env.Append(LIBS=['GLESv2', 'EGL'])
	else:
		env.Append(FRT_MODULES=['video_bcm.cpp', 'keyboard_linux_input.cpp', 'mouse_linux_input.cpp'])
		env.Append(CCFLAGS=['-I/opt/vc/include/'])
		env.Append(LINKFLAGS=['-L/opt/vc/lib/'])
		env.Append(LIBS=['brcmGLESv2', 'brcmEGL', 'bcm_host'])

	import version
	if version.major >= 3:
		env.Append(LIBS=['dl'])
