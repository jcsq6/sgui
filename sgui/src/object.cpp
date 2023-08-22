#include "gui/object.h"
#include "gui/window.h"

#include "utils/context_lock.h"

#include <GLFW/glfw3.h>

SGUI_BEG
void object::draw_raw(const window *, vec2) const
{
}

vec2 object::size() const
{
	return {};
}

vec2 object::min() const
{
	return {};
}


bool object::in_bounds(vec2 loc) const
{
	return in_bounds(loc, absolute_min());
}

bool object::in_bounds(vec2 loc, vec2 absolute_min) const
{
	return false;
}


vec2 object::absolute_min() const
{
	if (M_parent)
		return M_parent->absolute_min() + min();
	return min();
}

void object::obj_init()
{
}

void object::on_attach(object *child) const
{
}

void object::setup()
{
	obj_init();
	M_has_init = true;
	for (const auto &o : M_children)
		o->setup();
}

SGUI_END