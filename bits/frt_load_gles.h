// frt_load_gles.h
/*
 * FRT - A Godot platform targeting single board computers
 * Copyright (c) 2017-2019  Emanuele Fornara
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

// on pi, skip EGL/GLESv2 in /opt/vc/lib
static const char *lib(const char *s) {
#if defined(__arm__) || defined(__aarch64__)
	static char buf[64]; // large enough
	strcpy(buf, "/opt/vc/lib/");
	strcat(buf, s);
	if (access(buf, R_OK) != 0)
		return s;
#if defined(__arm__)
	strcpy(buf, "/usr/lib/arm-linux-gnueabihf/");
#else
	strcpy(buf, "/usr/lib/aarch64-linux-gnu/");
#endif
	strcat(buf, s);
	return buf;
#else // !pi
	return s;
#endif
}

#define FRT_DL_SKIP
#include "dl/gles2.gen.h"
#if FRT_GLES_VERSION == 3
#include "dl/gles3.gen.h"
#endif

static bool frt_load_gles(int version) {
#if FRT_GLES_VERSION == 3
	if (version == 3)
		return frt_load_gles3(lib("libGLESv2.so.2"));
#endif
	return frt_load_gles2(lib("libGLESv2.so.2"));
}
