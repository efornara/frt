
import os
import sys


import version

if version.major > 2:
	yes = True
	no = False
else:
	yes = 'yes'
	no = 'no'


def has_wrapper_for(lib):

	if version.major == 2:
		return False
	if version.minor > 2:
		return True
	return False


def is_active():

	return True


def get_name():

	return 'FRT'


def can_build():

	if os.name != 'posix':
		return False
	return True


def get_opts():

	return [
		('frt_arch', 'Architecture (pc/pi1/pi2/pi3/pi4/*)', 'pc'),
		('use_llvm', 'Use llvm compiler', no),
		('use_lto', 'Use link time optimization', no),
		('pulseaudio', 'Detect and use pulseaudio', no),
	]


def get_flags():

	return [
		('tools', no),
	]

def check(env, key):

	if not (key in env):
		return False
	if version.major > 2:
		return env[key]
	else:
		return env[key] == 'yes'

def configure(env):

	import methods

	env.Append(CPPPATH=['#platform/frt'])

	if check(env, 'use_llvm'):
		if 'clang++' not in env['CXX']:
			env['CC'] = 'clang'
			env['CXX'] = 'clang++'
			env['LD'] = 'clang++'
		env.Append(CPPFLAGS=['-DTYPED_METHOD_BIND'])
		env.extra_suffix = '.llvm'

	if check(env, 'use_lto'):
		if check(env, 'use_llvm'):
			env.Append(CCFLAGS=['-flto=thin'])
			env.Append(LINKFLAGS=['-fuse-ld=lld', '-flto=thin'])
			env['AR'] = 'llvm-ar'
			env['RANLIB'] = 'llvm-ranlib'
		else:
			env.Append(CCFLAGS=['-flto'])
			env.Append(LINKFLAGS=['-flto'])

	env.Append(CCFLAGS=['-pipe'])
	env.Append(LINKFLAGS=['-pipe'])

	if os.system('pkg-config --exists alsa') == 0:
		print('Enabling ALSA')
		env.Append(CPPFLAGS=['-DALSA_ENABLED'])
		if has_wrapper_for('alsa'):
			env['alsa'] = True
		else:
			env.ParseConfig('pkg-config alsa --cflags --libs')
	else:
		print('ALSA libraries not found, disabling driver')

	if check(env, 'pulseaudio'):
		if os.system("pkg-config --exists libpulse") == 0:
			print("Enabling PulseAudio")
			env.Append(CPPDEFINES=["PULSEAUDIO_ENABLED"])
			env.ParseConfig("pkg-config --cflags --libs libpulse")
		else:
			print("PulseAudio libraries not found, disabling driver")

	# common
	if not check(env, 'builtin_freetype'):
		env.ParseConfig('pkg-config freetype2 --cflags --libs')
	if not check(env, 'builtin_libpng'):
		env.ParseConfig('pkg-config libpng --cflags --libs')
	if not check(env, 'builtin_zlib'):
		env.ParseConfig('pkg-config zlib --cflags --libs')
	if not check(env, 'builtin_libwebp'):
		env.ParseConfig('pkg-config libwebp --cflags --libs')
	if not check(env, 'builtin_libtheora'):
		env.ParseConfig('pkg-config theora theoradec --cflags --libs')
	if not check(env, 'builtin_libvorbis'):
		env.ParseConfig('pkg-config vorbis vorbisfile --cflags --libs')
	if not check(env, 'builtin_opus'):
		env.ParseConfig('pkg-config opus opusfile --cflags --libs')
	if not check(env, 'builtin_libogg'):
		env.ParseConfig('pkg-config ogg --cflags --libs')
	# 2.1 / 3.0
	if version.major == 2 or (version.major == 3 and version.minor == 0):
		if not check(env, 'builtin_openssl') and check(env, 'module_openssl_enabled'):
			env.ParseConfig('pkg-config openssl --cflags --libs')
	# 3.0+
	if version.major == 3:
		if not check(env, 'builtin_libvpx'):
			env.ParseConfig('pkg-config vpx --cflags --libs')
		if not check(env, 'builtin_bullet'):
			env.ParseConfig('pkg-config bullet --cflags --libs')
		if not check(env, 'builtin_enet'):
			env.ParseConfig('pkg-config libenet --cflags --libs')
		if not check(env, 'builtin_zstd'):
			env.ParseConfig('pkg-config libzstd --cflags --libs')
		if not check(env, 'builtin_pcre2'):
			env.ParseConfig('pkg-config libpcre2-32 --cflags --libs')
	# 3.1+
	if version.major == 3 and version.minor >= 1:
		if not check(env, 'builtin_mbedtls') and check(env, 'module_openssl_enabled'):
			env.Append(LIBS=['mbedtls', 'mbedcrypto', 'mbedx509'])
		if not check(env, 'builtin_wslay') and check(env, 'module_openssl_enabled'):
			env.ParseConfig('pkg-config libwslay --cflags --libs')
		if not check(env, 'builtin_miniupnpc'):
			env.Prepend(CPPPATH=["/usr/include/miniupnpc"])
			env.Append(LIBS=["miniupnpc"])

	# pkg-config returns 0 when the lib exists...
	found_udev = not os.system('pkg-config --exists libudev')
	if found_udev:
		print('Enabling udev support')
		env.Append(CPPFLAGS=['-DUDEV_ENABLED'])
		if has_wrapper_for('udev'):
			env.Append(FRT_MODULES=['include/libudev-so_wrap.c'])
		else:
			env.ParseConfig('pkg-config libudev --cflags --libs')
		env.Append(CPPFLAGS=['-DJOYDEV_ENABLED'])
		env.Append(FRT_MODULES=['include/joypad_linux.cpp'])
	else:
		print('libudev development libraries not found, disabling udev support')

	if version.major == 2:
		if version.minor == 1 and version.patch >=4:
			gen_suffix = 'glsl.gen.h'
		else:
			gen_suffix = 'glsl.h'
		env.Append(BUILDERS={'GLSL120': env.Builder(action=methods.build_legacygl_headers, suffix=gen_suffix, src_suffix='.glsl')})
		env.Append(BUILDERS={'GLSL': env.Builder(action=methods.build_glsl_headers, suffix=gen_suffix, src_suffix='.glsl')})
		env.Append(BUILDERS={'GLSL120GLES': env.Builder(action=methods.build_gles2_headers, suffix=gen_suffix, src_suffix='.glsl')})

	if env['target'] == 'release':
		env.Append(CCFLAGS=['-O3', '-fomit-frame-pointer'])
	elif env['target'] == 'release_debug':
		env.Append(CCFLAGS=['-O3', '-DDEBUG_ENABLED'])
	elif env['target'] == 'debug':
		env.Append(CCFLAGS=['-g2', '-DDEBUG_ENABLED', '-DDEBUG_MEMORY_ENABLED'])
	if env['target'].startswith('release'):
		if version.major == 2 or (version.major == 3 and version.minor == 0):
			env.Append(CCFLAGS=['-ffast-math'])

	if env['frt_arch'] == 'pi1':
		env.Append(CCFLAGS=['-mcpu=arm1176jzf-s', '-mfpu=vfp'])
		env.extra_suffix += '.pi1'
	elif env['frt_arch'] == 'pi2':
		env.Append(CCFLAGS=['-mcpu=cortex-a7', '-mfpu=neon-vfpv4'])
		env.extra_suffix += '.pi2'
	elif env['frt_arch'] == 'pi3':
		env.Append(CCFLAGS=['-mcpu=cortex-a53', '-mfpu=neon-fp-armv8'])
		env.extra_suffix += '.pi3'
	elif env['frt_arch'] == 'pi4':
		env.Append(CCFLAGS=['-mcpu=cortex-a72', '-mfpu=neon-fp-armv8', '-mtune=cortex-a72'])
		env.extra_suffix += '.pi4'
	else:
		env.extra_suffix += '.' + env['frt_arch']

	if env['frt_arch'].startswith('pi'):
		env.Append(CCFLAGS=['-mfloat-abi=hard', '-mlittle-endian', '-munaligned-access'])

	env.Append(CFLAGS=['-std=gnu11']) # for libwebp (maybe more in the future)
	env.Append(CPPFLAGS=['-DFRT_ENABLED', '-DUNIX_ENABLED', '-DGLES2_ENABLED', '-DGLES_ENABLED'])
	env.Append(LIBS=['pthread'])

	env.Append(FRT_MODULES=['envprobe.cpp'])
	env.Append(FRT_MODULES=['video_fbdev.cpp', 'keyboard_linux_input.cpp', 'mouse_linux_input.cpp'])
	env.Append(FRT_MODULES=['video_x11.cpp', 'keyboard_x11.cpp', 'mouse_x11.cpp'])
	if version.major >= 3:
		env.Append(FRT_MODULES=['import/key_mapping_x11_3.cpp'])
	else:
		env.Append(FRT_MODULES=['import/key_mapping_x11_2.cpp'])
	env.Append(FRT_MODULES=['dl/x11.gen.cpp', 'dl/egl.gen.cpp'])
	if os.path.isfile('/opt/vc/include/bcm_host.h'):
		env.Append(FRT_MODULES=['video_bcm.cpp', 'dl/bcm.gen.cpp'])
	if os.path.isfile('/usr/include/gbm.h'):
		env.Append(FRT_MODULES=['video_kmsdrm.cpp', 'dl/gbm.gen.cpp', 'dl/drm.gen.cpp'])
	env.Append(FRT_MODULES=['dl/gles2.gen.cpp'])
	if version.major >= 3:
		env.Append(FRT_MODULES=['dl/gles3.gen.cpp'])
	env.Append(LIBS=['dl'])
