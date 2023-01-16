// custom_renderer.h
/*
  FRT - A Godot platform targeting single board computers
  Copyright (c) 2017-2023  Emanuele Fornara
  SPDX-License-Identifier: MIT
 */

#ifndef FRT_CUSTOM_RENDERER_H
#define FRT_CUSTOM_RENDERER_H

namespace frt {

struct CustomRenderer {
	virtual ~CustomRenderer();
	virtual void make_current() = 0;
};

CustomRenderer *new_CustomRenderer();

} // namespace frt

#endif // FRT_CUSTOM_RENDERER_H
