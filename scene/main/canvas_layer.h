/*************************************************************************/
/*  canvas_layer.h                                                       */
/*************************************************************************/
/*                       This file is part of:                           */
/*                           GODOT ENGINE                                */
/*                    http://www.godotengine.org                         */
/*************************************************************************/
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                 */
/*                                                                       */
/* Permission is hereby granted, free of charge, to any person obtaining */
/* a copy of this software and associated documentation files (the       */
/* "Software"), to deal in the Software without restriction, including   */
/* without limitation the rights to use, copy, modify, merge, publish,   */
/* distribute, sublicense, and/or sell copies of the Software, and to    */
/* permit persons to whom the Software is furnished to do so, subject to */
/* the following conditions:                                             */
/*                                                                       */
/* The above copyright notice and this permission notice shall be        */
/* included in all copies or substantial portions of the Software.       */
/*                                                                       */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,       */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF    */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.*/
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY  */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,  */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE     */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                */
/*************************************************************************/
#ifndef CANVAS_LAYER_H
#define CANVAS_LAYER_H

#include "scene/main/node.h"
#include "scene/resources/world_2d.h"


class Viewport;
class CanvasLayer : public Node {

	OBJ_TYPE( CanvasLayer, Node );

	bool locrotscale_dirty;
	Vector2 ofs;
	Vector2 scale;
	real_t rot;
	int layer;
	Transform2D transform;
	Ref<World2D> canvas;
	RID viewport;
	Viewport *vp;


	void _set_rotationd(real_t p_rotation);
	real_t _get_rotationd() const;

	void _update_xform();
	void _update_locrotscale();

protected:

	void _notification(int p_what);
	static void _bind_methods();
public:

	void set_layer(int p_xform);
	int get_layer() const;

	void set_transform(const Transform2D& p_xform);
	Transform2D get_transform() const;

	void set_offset(const Vector2& p_offset);
	Vector2 get_offset() const;

	void set_rotation(real_t p_rotation);
	real_t get_rotation() const;

	void set_scale(const Vector2& p_scale);
	Vector2 get_scale() const;

	Ref<World2D> get_world_2d() const;

	Size2 get_viewport_size() const;

	RID get_viewport() const;

	CanvasLayer();
};

#endif // CANVAS_LAYER_H
