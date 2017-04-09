// test_modules.cpp
/*
 * FRT - A Godot platform targeting single board computers
 * Copyright (c) 2017  Emanuele Fornara
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "frt.h"

struct Type1 : public frt::Module {
	virtual void hello() = 0;
};

struct Type1Impl1 : public Type1 {
	// Module
	const char *get_id() const { return "type1_impl1"; }
	bool probe() { return true; }
	void cleanup() {}
	// Type1
	void hello() { printf("Hello, World!\n"); }
};

FRT_REGISTER(Type1Impl1)

#include "bits/frt_app_impl.h"

int main(int argc, char *argv[]) {
	frt::Registry *frt = frt::Registry::instance();
	const char *type1_modules[] = {
		"missing",
		"type1_impl1",
		"not_probed",
		0
	};
	void **ctx;
	ctx = frt->get_context("type1");
	assert(ctx);
	*ctx = malloc(10);
	ctx = frt->get_context("type1");
	free(*ctx);
	*ctx = 0;
	frt::Module *module = frt->probe(type1_modules);
	assert(module);
	assert(!strcmp(module->get_id(), "type1_impl1"));
	Type1 *type1 = (Type1 *)module;
	type1->hello();
	type1->cleanup();
}
