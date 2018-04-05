// frt_app_impl.h
/*
 * FRT - A Godot platform targeting single board computers
 * Copyright (c) 2017-2018  Emanuele Fornara
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace frt {

void fatal(const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	fprintf(stderr, "frt: ");
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(1);
}

void App::register_(Module *module) {
	if (nm >= max_modules)
		return;
	m[nm++] = module;
}

Module *App::get(const char *id) const {
	for (int i = 0; i < nm; i++)
		if (!strcmp(id, m[i]->get_id()))
			return m[i];
	return 0;
}

Module *App::probe(const char *ids[]) {
	for (int i = 0; ids[i]; i++) {
		Module *module = get(ids[i]);
		if (module && module->probe())
			return module;
	}
	return 0;
}

Module *App::probe_single() {
	if (nm != 1)
		return 0;
	if (!m[0]->probe())
		return 0;
	return m[0];
}

void **App::get_context(const char *key) {
	for (int i = 0; i < nc; i++)
		if (!strcmp(key, c[i].key))
			return &c[i].value;
	if (nc >= max_contexts)
		return 0;
	c[nc].key = key;
	c[nc].value = 0;
	return &c[nc++].value;
}

void App::add_dispatcher(EventDispatcher *dispatcher) {
	if (nd >= max_dispatchers)
		return;
	d[nd++] = dispatcher;
}

void App::remove_dispatcher(EventDispatcher *dispatcher) {
	int i;
	for (i = 0; i < nd; i++)
		if (d[i] == dispatcher)
			break;
	if (i == nd)
		return;
	nd--;
	for (; i < nd; i++)
		d[i] = d[i + 1];
}

void App::dispatch_events() {
	for (int i = 0; i < nd; i++)
		d[i]->dispatch_events();
}

App *App::instance() {
	static App *a = 0;
	if (!a)
		a = new App();
	return a;
}

} // namespace frt
