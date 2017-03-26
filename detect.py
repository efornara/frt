
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
		('frt_arch', 'Architecture (none/pi1/pi2/pi3)', 'pi1'),
	]


def get_flags():

	return [
	]


def configure(env):

	env.Append(CPPPATH=['#platform/frt'])

	if os.system("pkg-config --exists alsa") == 0:
		print("Enabling ALSA")
		env.Append(CPPFLAGS=["-DALSA_ENABLED"])
		env.ParseConfig('pkg-config alsa --cflags --libs')
	else:
		print("ALSA libraries not found, disabling driver")

	import methods

	env.Append(BUILDERS={'GLSL120': env.Builder(action=methods.build_legacygl_headers, suffix='glsl.h', src_suffix='.glsl')})
	env.Append(BUILDERS={'GLSL': env.Builder(action=methods.build_glsl_headers, suffix='glsl.h', src_suffix='.glsl')})
	env.Append(BUILDERS={'GLSL120GLES': env.Builder(action=methods.build_gles2_headers, suffix='glsl.h', src_suffix='.glsl')})

	if (env["target"] == "release"):
		env.Append(CCFLAGS=['-O2', '-ffast-math', '-fomit-frame-pointer'])
	elif (env["target"] == "release_debug"):
		env.Append(CCFLAGS=['-O2', '-ffast-math', '-DDEBUG_ENABLED'])
	elif (env["target"] == "debug"):
		env.Append(CCFLAGS=['-g2', '-Wall', '-DDEBUG_ENABLED', '-DDEBUG_MEMORY_ENABLED'])

	if (env["frt_arch"] == "pi1"):
		env.Append(CCFLAGS=['-mcpu=arm1176jzf-s', '-mfpu=vfp'])
		env.extra_suffix += ".pi1"
	elif (env["frt_arch"] == "pi2"):
		env.Append(CCFLAGS=['-mcpu=cortex-a7', '-mfpu=neon-vfpv4'])
		env.extra_suffix += ".pi2"
	elif (env["frt_arch"] == "pi3"):
		env.Append(CCFLAGS=['-mcpu=cortex-a53', '-mfpu=neon-fp-armv8'])
		env.extra_suffix += ".pi3"

	if (env["frt_arch"] != "none"):
		env.Append(CCFLAGS=['-mfloat-abi=hard', '-mlittle-endian', '-munaligned-access'])

	env.Append(CPPFLAGS=['-DUNIX_ENABLED', '-DGLES2_ENABLED'])
	env.Append(LIBS=['pthread'])

	if (env["frt_arch"] == "none"):
		env.Append(LIBS=['-lEGL', '-lGLESv2', '-lX11'])
	else:
		env.Append(CCFLAGS=['-I/opt/vc/include/'])
		env.Append(LINKFLAGS=['-L/opt/vc/lib/'])
		env.Append(LIBS=['brcmGLESv2', 'brcmEGL', 'bcm_host'])
