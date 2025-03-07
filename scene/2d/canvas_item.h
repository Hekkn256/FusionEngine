/*************************************************************************/
/*  canvas_item.h                                                        */
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
#ifndef CANVAS_ITEM_H
#define CANVAS_ITEM_H

#include "scene/main/node.h"
#include "scene/resources/texture.h"
#include "scene/main/scene_main_loop.h"

class CanvasLayer;
class Viewport;
class Font;

class StyleBox;

class CanvasItem : public Node {

	OBJ_TYPE( CanvasItem, Node );
public:

	enum BlendMode {

		BLEND_MODE_MIX, //default
		BLEND_MODE_ADD,
		BLEND_MODE_SUB,
		BLEND_MODE_MUL,
		BLEND_MODE_PREMULT_ALPHA
	};

private:

	mutable SelfList<Node> xform_change;

	RID canvas_item;
	String group;


	CanvasLayer *canvas_layer;

	float opacity;
	float self_opacity;

	List<CanvasItem*> children_items;
	List<CanvasItem*>::Element *C;

	BlendMode blend_mode;

	bool first_draw;
	bool hidden;
	bool pending_update;
	bool toplevel;
	bool pending_children_sort;
	bool drawing;
	bool block_transform_notify;
	bool behind;

	mutable Transform2D global_transform;
	mutable bool global_invalid;


	void _raise_self();

	void _propagate_visibility_changed(bool p_visible);

	void _set_visible_(bool p_visible);
	bool _is_visible_() const;

	void _update_callback();

	void _enter_canvas();
	void _exit_canvas();

	void _queue_sort_children();
	void _sort_children();



	void _notify_transform(CanvasItem *p_node);

	void _set_on_top(bool p_on_top) { set_draw_behind_parent(!p_on_top); }
	bool _is_on_top() const { return !is_draw_behind_parent_enabled(); }

protected:



	_FORCE_INLINE_ void _notify_transform() { if (!is_inside_tree()) return; _notify_transform(this); if (!block_transform_notify) notification(NOTIFICATION_LOCAL_TRANSFORM_CHANGED); }

	void item_rect_changed();

	void _notification(int p_what);
	static void _bind_methods();
public:


	enum {
		NOTIFICATION_TRANSFORM_CHANGED=SceneTree::NOTIFICATION_TRANSFORM_CHANGED, //unique
		NOTIFICATION_DRAW=30,
		NOTIFICATION_VISIBILITY_CHANGED=31,
		NOTIFICATION_ENTER_CANVAS=32,
		NOTIFICATION_EXIT_CANVAS=33,
		NOTIFICATION_LOCAL_TRANSFORM_CHANGED=35,
		NOTIFICATION_WORLD_2D_CHANGED=36,

	};

	/* EDITOR */

	virtual Variant edit_get_state() const;
	virtual void edit_set_state(const Variant& p_state);
	virtual void edit_set_rect(const Rect2& p_edit_rect);
	virtual void edit_rotate(float p_rot);
	virtual Size2 edit_get_minimum_size() const;

	/* VISIBILITY */

	bool is_visible() const;
	bool is_hidden() const;
	void show();
	void hide();

	void update();

	void set_blend_mode(BlendMode p_blend_mode);
	BlendMode get_blend_mode() const;

	void set_opacity(float p_opacity);
	float get_opacity() const;

	void set_self_opacity(float p_self_opacity);
	float get_self_opacity() const;

	/* DRAWING API */

	void draw_line(const Point2& p_from, const Point2& p_to,const Color& p_color,float p_width=1.0);
	void draw_rect(const Rect2& p_rect, const Color& p_color);
	void draw_circle(const Point2& p_pos, float p_radius, const Color& p_color);
	void draw_texture(const Ref<Texture>& p_texture,const Point2& p_pos);
	void draw_texture_rect(const Ref<Texture>& p_texture, const Rect2& p_rect, bool p_tile=false,const Color& p_modulate=Color(1,1,1));
	void draw_texture_rect_region(const Ref<Texture>& p_texture,const Rect2& p_rect, const Rect2& p_src_rect,const Color& p_modulate=Color(1,1,1));
	void draw_style_box(const Ref<StyleBox>& p_style_box,const Rect2& p_rect);
	void draw_primitive(const Vector<Point2>& p_points, const Vector<Color>& p_colors,const Vector<Point2>& p_uvs, Ref<Texture> p_texture=Ref<Texture>(),float p_width=1);
	void draw_polygon(const Vector<Point2>& p_points, const Vector<Color>& p_colors,const Vector<Point2>& p_uvs=Vector<Point2>(), Ref<Texture> p_texture=Ref<Texture>());
	void draw_colored_polygon(const Vector<Point2>& p_points, const Color& p_color,const Vector<Point2>& p_uvs=Vector<Point2>(), Ref<Texture> p_texture=Ref<Texture>());

	void draw_string(const Ref<Font>& p_font, const Point2& p_pos, const String& p_text,const Color& p_modulate=Color(1,1,1),int p_clip_w=-1);
	float draw_char(const Ref<Font>& p_font,const Point2& p_pos, const String& p_char,const String& p_next="",const Color& p_modulate=Color(1,1,1));

	void draw_set_transform(const Point2& p_offset, float p_rot, const Size2& p_scale);

	/* RECT / TRANSFORM */

	void set_as_toplevel(bool p_toplevel);
	bool is_set_as_toplevel() const;

	void set_draw_behind_parent(bool p_enable);
	bool is_draw_behind_parent_enabled() const;

	CanvasItem *get_parent_item() const;

	virtual Rect2 get_item_rect() const=0;
	virtual Transform2D get_transform() const=0;

	virtual Transform2D get_global_transform() const;
	virtual Transform2D get_global_transform_with_canvas() const;

	CanvasItem *get_toplevel() const;
	_FORCE_INLINE_ RID get_canvas_item() const { return canvas_item; }

	void set_block_transform_notify(bool p_enable);
	bool is_block_transform_notify_enabled() const;


	Transform2D get_canvas_transform() const;
	Transform2D get_viewport_transform() const;
	Rect2 get_viewport_rect() const;
	RID get_viewport_rid() const;
	RID get_canvas() const;
	Ref<World2D> get_world_2d() const;



	CanvasItem();
	~CanvasItem();
};

VARIANT_ENUM_CAST( CanvasItem::BlendMode );

#endif // CANVAS_ITEM_H
