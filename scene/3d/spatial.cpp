/*************************************************************************/
/*  spatial.cpp                                                          */
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
#include "spatial.h"
 
#include "scene/main/viewport.h"
#include "message_queue.h"
#include "scene/scene_string_names.h"

/*

 possible algorithms:

 Algorithm 1: (current)

 definition of invalidation: global is invalid

 1) If a node sets a LOCAL, it produces an invalidation of everything above
    a) If above is invalid, don't keep invalidating upwards
 2) If a node sets a GLOBAL, it is converted to LOCAL (and forces validation of everything pending below)

 drawback: setting/reading globals is useful and used very very often, and using affine inverses is slow

---

 Algorithm 2: (no longer current)

 definition of invalidation: NONE dirty, LOCAL dirty, GLOBAL dirty

 1) If a node sets a LOCAL, it must climb the tree and set it as GLOBAL dirty
    a) marking GLOBALs as dirty up all the tree must be done always
 2) If a node sets a GLOBAL, it marks local as dirty, and that's all?

 //is clearing the dirty state correct in this case?

 drawback: setting a local down the tree forces many tree walks often

--

future: no idea

 */



SpatialGizmo::SpatialGizmo() {

}

void Node3D::_notify_dirty() {

	if (!data.ignore_notification && !xform_change.in_list()) {

		get_tree()->xform_change_list.add(&xform_change);
	}
}



void Node3D::_update_local_transform() const {

	data.local_transform.basis.set_euler(data.rotation);
	data.local_transform.basis.scale(data.scale);

	data.dirty&=~DIRTY_LOCAL;
}
void Node3D::_propagate_transform_changed(Node3D *p_origin) {

	if (!is_inside_tree()) {
		return;
	}

//	if (data.dirty&DIRTY_GLOBAL)
//		return; //already dirty

	data.children_lock++;
		
	for (List<Node3D*>::Element *E=data.children.front();E;E=E->next()) {
	
		if (E->get()->data.toplevel_active)
			continue; //don't propagate to a toplevel
		E->get()->_propagate_transform_changed(p_origin);
	}
	

	if (!data.ignore_notification && !xform_change.in_list()) {

		get_tree()->xform_change_list.add(&xform_change);

	}
	data.dirty|=DIRTY_GLOBAL;

	data.children_lock--;
}

void Node3D::_notification(int p_what) {

	switch(p_what) {
		case NOTIFICATION_ENTER_TREE: {

			Node *p = get_parent();
			if (p)
				data.parent=p->cast_to<Node3D>();

			if (data.parent)
				data.C=data.parent->data.children.push_back(this);
			else
				data.C=NULL;

			if (data.toplevel && !get_tree()->is_editor_hint()) {

				if (data.parent) {
					data.local_transform = data.parent->get_global_transform() * get_transform();
					data.dirty=DIRTY_VECTORS; //global is always dirty upon entering a scene
				}
				data.toplevel_active=true;
			}

			data.dirty|=DIRTY_GLOBAL; //global is always dirty upon entering a scene
			_notify_dirty();

			notification(NOTIFICATION_ENTER_WORLD);

		} break;
		case NOTIFICATION_EXIT_TREE: {

			notification(NOTIFICATION_EXIT_WORLD,true);
			if (xform_change.in_list())
				get_tree()->xform_change_list.remove(&xform_change);
			if (data.C)
				data.parent->data.children.erase(data.C);
			data.parent=NULL;
			data.C=NULL;
			data.toplevel_active=false;
		} break;
		case NOTIFICATION_ENTER_WORLD: {

			data.inside_world=true;
			data.viewport=NULL;
			Node *parent = get_parent();
			while(parent && !data.viewport) {
				data.viewport=parent->cast_to<Viewport>();
				parent=parent->get_parent();
			}

			ERR_FAIL_COND(!data.viewport);


			if (get_script_instance()) {

				Variant::CallError err;
				get_script_instance()->call_multilevel(SceneStringNames::_enter_world,NULL,0);
			}
#ifdef TOOLS_ENABLED
			if (get_tree()->is_editor_hint()) {

//				get_scene()->call_group(SceneMainLoop::GROUP_CALL_REALTIME,SceneStringNames::_spatial_editor_group,SceneStringNames::_request_gizmo,this);
				get_tree()->call_group(0,SceneStringNames::_spatial_editor_group,SceneStringNames::_request_gizmo,this);
				if (!data.gizmo_disabled) {

					if (data.gizmo.is_valid())
						data.gizmo->create();
				}
			}
#endif

		} break;
		case NOTIFICATION_EXIT_WORLD: {

#ifdef TOOLS_ENABLED
			if (data.gizmo.is_valid()) {
				data.gizmo->free();
			}
#endif

			if (get_script_instance()) {

				Variant::CallError err;
				get_script_instance()->call_multilevel(SceneStringNames::_exit_world,NULL,0);
			}

			data.viewport=NULL;
			data.inside_world=false;

		} break;


		case NOTIFICATION_TRANSFORM_CHANGED: {
		
#ifdef TOOLS_ENABLED
			if (data.gizmo.is_valid()) {
				data.gizmo->transform();
			}
#endif
		} break;
	
		default: {}
	}
}

void Node3D::set_transform(const Transform3D& p_transform) {

	data.local_transform=p_transform;
	data.dirty|=DIRTY_VECTORS;
	_change_notify("transform/translation");
	_change_notify("transform/rotation");
	_change_notify("transform/scale");
	_propagate_transform_changed(this);

}

void Node3D::set_global_transform(const Transform3D& p_transform) {

	Transform3D xform =
			(data.parent && !data.toplevel_active) ?
			data.parent->get_global_transform().affine_inverse() * p_transform :
			p_transform;

	set_transform(xform);

}


Transform3D Node3D::get_transform() const {

	if (data.dirty & DIRTY_LOCAL) {

		_update_local_transform();
	}
	
	return data.local_transform;
}
Transform3D Node3D::get_global_transform() const {

	ERR_FAIL_COND_V(!is_inside_tree(), Transform3D());

	if (data.dirty & DIRTY_GLOBAL) {

		if (data.dirty & DIRTY_LOCAL) {

			_update_local_transform();
		}

		if (data.parent && !data.toplevel_active) {
		
			data.global_transform=data.parent->get_global_transform() * data.local_transform;
		} else {
		
			data.global_transform=data.local_transform;
		}
		
		data.dirty&=~DIRTY_GLOBAL;
	}
	
	return data.global_transform;
}
#if 0
void Node3D::add_child_notify(Node *p_child) {
/*
	Node3D *s=p_child->cast_to<Node3D>();
	if (!s)
		return;
		
	ERR_FAIL_COND(data.children_lock>0);

	s->data.dirty=DIRTY_GLOBAL; // don't allow global transform to be valid
	s->data.parent=this;
	data.children.push_back(s);
	s->data.C=data.children.back();
*/
}

void Node3D::remove_child_notify(Node *p_child) {
/*
	Node3D *s=p_child->cast_to<Node3D>();
	if (!s)
		return;
	
	ERR_FAIL_COND(data.children_lock>0);
	
	if (s->data.C)
		data.children.erase(s->data.C);
	s->data.parent=NULL;		
	s->data.C=NULL;
*/
}
#endif

Node3D *Node3D::get_parent_spatial() const {

	return data.parent;

}

Transform3D Node3D::get_relative_transform(const Node *p_parent) const {

	if (p_parent==this)
		return Transform3D();

	ERR_FAIL_COND_V(!data.parent,Transform3D());

	if (p_parent==data.parent)
		return get_transform();
	else
		return data.parent->get_relative_transform(p_parent) * get_transform();

}

void Node3D::set_translation(const Vector3& p_translation) {

	data.local_transform.origin=p_translation;
	_propagate_transform_changed(this);

}

void Node3D::set_rotation(const Vector3& p_euler){

	if (data.dirty&DIRTY_VECTORS) {
		data.scale=data.local_transform.basis.get_scale();
		data.dirty&=~DIRTY_VECTORS;
	}

	data.rotation=p_euler;
	data.dirty|=DIRTY_LOCAL;
	_propagate_transform_changed(this);

}
void Node3D::set_scale(const Vector3& p_scale){

	if (data.dirty&DIRTY_VECTORS) {
		data.rotation=data.local_transform.basis.get_euler();
		data.dirty&=~DIRTY_VECTORS;
	}

	data.scale=p_scale;
	data.dirty|=DIRTY_LOCAL;
	_propagate_transform_changed(this);

}

Vector3 Node3D::get_translation() const{

	return data.local_transform.origin;
}
Vector3 Node3D::get_rotation() const{

	if (data.dirty&DIRTY_VECTORS) {
		data.scale=data.local_transform.basis.get_scale();
		data.rotation=data.local_transform.basis.get_euler();
		data.dirty&=~DIRTY_VECTORS;
	}

	return data.rotation;
}
Vector3 Node3D::get_scale() const{

	if (data.dirty&DIRTY_VECTORS) {
		data.scale=data.local_transform.basis.get_scale();
		data.rotation=data.local_transform.basis.get_euler();
		data.dirty&=~DIRTY_VECTORS;
	}

	return data.scale;
}


void Node3D::update_gizmo() {

#ifdef TOOLS_ENABLED
	if (!is_inside_world())
		return;
	if (!data.gizmo.is_valid())
		return;
	if (data.gizmo_dirty)
		return;
	data.gizmo_dirty=true;
	MessageQueue::get_singleton()->push_call(this,"_update_gizmo");
#endif
}

void Node3D::set_gizmo(const Ref<SpatialGizmo>& p_gizmo) {

#ifdef TOOLS_ENABLED

	if (data.gizmo_disabled)
		return;
	if (data.gizmo.is_valid() && is_inside_world())
		data.gizmo->free();
	data.gizmo=p_gizmo;
	if (data.gizmo.is_valid() && is_inside_world()) {

		data.gizmo->create();
		data.gizmo->redraw();
		data.gizmo->transform();
	}

#endif
}

Ref<SpatialGizmo> Node3D::get_gizmo() const {

#ifdef TOOLS_ENABLED

	return data.gizmo;
#else

	return Ref<SpatialGizmo>();
#endif
}

#ifdef TOOLS_ENABLED

void Node3D::_update_gizmo() {

	data.gizmo_dirty=false;
	if (data.gizmo.is_valid()) {
		if (is_visible())
			data.gizmo->redraw();
		else
			data.gizmo->clear();
	}
}


void Node3D::set_disable_gizmo(bool p_enabled) {

	data.gizmo_disabled=p_enabled;
	if (!p_enabled && data.gizmo.is_valid())
		data.gizmo=Ref<SpatialGizmo>();
}

#endif

void Node3D::set_as_toplevel(bool p_enabled) {

	if (data.toplevel==p_enabled)
		return;
	if (is_inside_tree() && !get_tree()->is_editor_hint()) {

		if (p_enabled)
			set_transform(get_global_transform());
		else if (data.parent)
			set_transform(data.parent->get_global_transform().affine_inverse() * get_global_transform());

		data.toplevel=p_enabled;
		data.toplevel_active=p_enabled;

	} else {
		data.toplevel=p_enabled;
	}

}

bool Node3D::is_set_as_toplevel() const{

	return data.toplevel;
}

void Node3D::_set_rotation_deg(const Vector3& p_deg) {

	set_rotation(p_deg * Math_PI / 180.0);
}

Vector3 Node3D::_get_rotation_deg() const {

	return get_rotation() * 180.0 / Math_PI;
}

Ref<World3D> Node3D::get_world() const {

	ERR_FAIL_COND_V(!is_inside_world(),Ref<World3D>());
	return data.viewport->find_world();

}

#ifdef TOOLS_ENABLED
void Node3D::set_import_transform(const Transform3D& p_transform) {
	data.import_transform=p_transform;
}

Transform3D Node3D::get_import_transform() const {

	return data.import_transform;
}
#endif


void Node3D::_propagate_visibility_changed() {

	notification(NOTIFICATION_VISIBILITY_CHANGED);
	emit_signal(SceneStringNames::visibility_changed);
	_change_notify("visibility/visible");
#ifdef TOOLS_ENABLED
	if (data.gizmo.is_valid())
		_update_gizmo();
#endif

	for (List<Node3D*>::Element*E=data.children.front();E;E=E->next()) {

		Node3D *c=E->get();
		if (!c || !c->data.visible)
			continue;
		c->_propagate_visibility_changed();
	}
}


void Node3D::show() {

	if (data.visible)
		return;

	data.visible=true;

	if (!is_inside_tree())
		return;

	if (!data.parent || is_visible()) {

		_propagate_visibility_changed();
	}
}

void Node3D::hide(){

	if (!data.visible)
		return;

	bool was_visible = is_visible();
	data.visible=false;

	if (!data.parent || was_visible) {

		_propagate_visibility_changed();
	}

}
bool Node3D::is_visible() const{

	const Node3D *s=this;

	while(s) {
		if (!s->data.visible) {
			return false;
		}
		s=s->data.parent;
	}

	return true;
}


bool Node3D::is_hidden() const{

	return !data.visible;
}

void Node3D::_set_visible_(bool p_visible) {

	if (p_visible)
		show();
	else
		hide();
}

bool Node3D::_is_visible_() const {

	return !is_hidden();
}


void Node3D::_bind_methods() {

	ObjectTypeDB::bind_method(_MD("set_transform","local"), &Node3D::set_transform);
	ObjectTypeDB::bind_method(_MD("get_transform"), &Node3D::get_transform);
	ObjectTypeDB::bind_method(_MD("set_translation","translation"), &Node3D::set_translation);
	ObjectTypeDB::bind_method(_MD("get_translation"), &Node3D::get_translation);
	ObjectTypeDB::bind_method(_MD("set_rotation","rotation"), &Node3D::set_rotation);
	ObjectTypeDB::bind_method(_MD("get_rotation"), &Node3D::get_rotation);
	ObjectTypeDB::bind_method(_MD("set_scale","scale"), &Node3D::set_scale);
	ObjectTypeDB::bind_method(_MD("get_scale"), &Node3D::get_scale);
	ObjectTypeDB::bind_method(_MD("set_global_transform","global"), &Node3D::set_global_transform);
	ObjectTypeDB::bind_method(_MD("get_global_transform"), &Node3D::get_global_transform);
	ObjectTypeDB::bind_method(_MD("get_parent_spatial"), &Node3D::get_parent_spatial);
	ObjectTypeDB::bind_method(_MD("set_ignore_transform_notification","enabled"), &Node3D::set_ignore_transform_notification);
	ObjectTypeDB::bind_method(_MD("set_as_toplevel","enable"), &Node3D::set_as_toplevel);
	ObjectTypeDB::bind_method(_MD("is_set_as_toplevel"), &Node3D::is_set_as_toplevel);
	ObjectTypeDB::bind_method(_MD("_set_rotation_deg","rotation_deg"), &Node3D::_set_rotation_deg);
	ObjectTypeDB::bind_method(_MD("_get_rotation_deg"), &Node3D::_get_rotation_deg);
	ObjectTypeDB::bind_method(_MD("get_world:World3D"), &Node3D::get_world);

#ifdef TOOLS_ENABLED
	ObjectTypeDB::bind_method(_MD("_update_gizmo"), &Node3D::_update_gizmo);
	ObjectTypeDB::bind_method(_MD("_set_import_transform"), &Node3D::set_import_transform);
	ObjectTypeDB::bind_method(_MD("_get_import_transform"), &Node3D::get_import_transform);
	ADD_PROPERTY( PropertyInfo(Variant::TRANSFORM,"_import_transform",PROPERTY_HINT_NONE,"",PROPERTY_USAGE_NOEDITOR),_SCS("_set_import_transform"),_SCS("_get_import_transform"));
#endif

	ObjectTypeDB::bind_method(_MD("update_gizmo"), &Node3D::update_gizmo);
	ObjectTypeDB::bind_method(_MD("set_gizmo","gizmo:SpatialGizmo"), &Node3D::set_gizmo);
	ObjectTypeDB::bind_method(_MD("get_gizmo:SpatialGizmo"), &Node3D::get_gizmo);

	ObjectTypeDB::bind_method(_MD("show"), &Node3D::show);
	ObjectTypeDB::bind_method(_MD("hide"), &Node3D::hide);
	ObjectTypeDB::bind_method(_MD("is_visible"), &Node3D::is_visible);
	ObjectTypeDB::bind_method(_MD("is_hidden"), &Node3D::is_hidden);

	ObjectTypeDB::bind_method(_MD("_set_visible_"), &Node3D::_set_visible_);
	ObjectTypeDB::bind_method(_MD("_is_visible_"), &Node3D::_is_visible_);

	BIND_CONSTANT( NOTIFICATION_TRANSFORM_CHANGED );
	BIND_CONSTANT( NOTIFICATION_ENTER_WORLD );
	BIND_CONSTANT( NOTIFICATION_EXIT_WORLD );
	BIND_CONSTANT( NOTIFICATION_VISIBILITY_CHANGED );

	//ADD_PROPERTY( PropertyInfo(Variant::TRANSFORM,"transform/global",PROPERTY_HINT_NONE, "", PROPERTY_USAGE_EDITOR ), _SCS("set_global_transform"), _SCS("get_global_transform") );
	ADD_PROPERTYNZ( PropertyInfo(Variant::TRANSFORM,"transform/local",PROPERTY_HINT_NONE,""), _SCS("set_transform"), _SCS("get_transform") );
	ADD_PROPERTY( PropertyInfo(Variant::VECTOR3,"transform/translation",PROPERTY_HINT_NONE,"",PROPERTY_USAGE_EDITOR), _SCS("set_translation"), _SCS("get_translation") );
	ADD_PROPERTY( PropertyInfo(Variant::VECTOR3,"transform/rotation",PROPERTY_HINT_NONE,"",PROPERTY_USAGE_EDITOR), _SCS("_set_rotation_deg"), _SCS("_get_rotation_deg") );
	ADD_PROPERTY( PropertyInfo(Variant::VECTOR3,"transform/rotation_rad",PROPERTY_HINT_NONE,"",0), _SCS("set_rotation"), _SCS("get_rotation") );
	ADD_PROPERTY( PropertyInfo(Variant::VECTOR3,"transform/scale",PROPERTY_HINT_NONE,"",PROPERTY_USAGE_EDITOR), _SCS("set_scale"), _SCS("get_scale") );
	ADD_PROPERTY( PropertyInfo(Variant::BOOL,"visibility/visible"), _SCS("_set_visible_"), _SCS("_is_visible_") );
	//ADD_PROPERTY( PropertyInfo(Variant::TRANSFORM,"transform/local"), _SCS("set_transform"), _SCS("get_transform") );

	ADD_SIGNAL( MethodInfo("visibility_changed" ) );

}


Node3D::Node3D() : xform_change(this)
{

	data.dirty=DIRTY_NONE;
	data.children_lock=0;

	data.ignore_notification=false;
	data.toplevel=false;
	data.toplevel_active=false;
	data.scale=Vector3(1,1,1);
	data.viewport=NULL;
	data.inside_world=false;
	data.visible=true;
#ifdef TOOLS_ENABLED
	data.gizmo_disabled=false;
	data.gizmo_dirty=false;
#endif
	data.parent=NULL;
	data.C=NULL;

}


Node3D::~Node3D() {

}


