/*************************************************************************/
/*  ray_cast.h                                                           */
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
#ifndef RAY_CAST_H
#define RAY_CAST_H

#include "scene/3d/spatial.h"

class RayCast3D : public Node3D {

	OBJ_TYPE(RayCast3D,Node3D);


	bool enabled;
	bool collided;
	ObjectID against;
	int against_shape;
	Vector3 collision_point;
	Vector3 collision_normal;

	Vector3 cast_to;

	RID exception;
	Set<RID> exceptions;

protected:

	void _notification(int p_what);
	static void _bind_methods();
public:

	void set_enabled(bool p_enabled);
	bool is_enabled() const;

	void set_cast_to(const Vector3& p_point);
	Vector3 get_cast_to() const;

	bool is_colliding() const;
	Object *get_collider() const;
	int get_collider_shape() const;
	Vector3 get_collision_point() const;
	Vector3 get_collision_normal() const;

	RayCast3D();
};

#endif // RAY_CAST_H
