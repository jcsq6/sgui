#include "gui/widget.h"
#include "gui/window.h"

#include "help.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

SGUI_BEG

bool rectangle::in_bounds(vec2 loc, vec2 absolute_min) const
{
	auto min = M_min + absolute_min;
	auto M_max = min + M_dims;
	return (loc.x < M_max.x && loc.x > min.x) && (loc.y < M_max.y && loc.y > min.y);
}

vec2 rectangle::min() const
{
	return M_min;
}

vec2 rectangle::size() const
{
	return M_dims;
}

bool button::in_bounds(vec2 loc, vec2 absolute_min) const
{
	return rectangle::in_bounds(loc, absolute_min);
}

vec2 button::size() const
{
	return rectangle::size();
}

vec2 button::min() const
{
	return M_min;
}

void button::draw_raw(const window *win, vec2 absolute_min) const
{
	static vao v = detail::make_shape_vao(detail::rect_obj_vbo(), detail::text_pos_vbo());
	static auto &program = detail::global_color_shader();

	if (!win)
		return;

	auto min = absolute_min + M_min;

	{
		auto pos_vbo = v.get_attribute(pos_loc);
		auto data = pos_vbo. template get_data<vec2>(GL_WRITE_ONLY);

		auto max = min + M_dims;
		data[0] = min;
		data[1].x = max.x;
		data[1].y = min.y;
		data[2] = max;
		data[3].x = min.x;
		data[3].y = max.y;
	}

	glDisable(GL_CULL_FACE);

	program.set_uniform("SGUI_Color", M_col);
	program.set_uniform("SGUI_Ortho", win->ortho());
	program.set_uniform("SGUI_Model", identity());

	program.bind();

	v.use();
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	for (auto child : M_children)
		child->draw_raw(win, min);
}

void button::do_flags()
{
	if (!M_parent)
		return;

	if (M_flags & attach_flags::centered)
	{
		// M_min + M_dims / 2 = size() / 2
		// M_min = (size() - M_dims) / 2
		M_min = (M_parent->size() - M_dims) / 2.f;
	}
}

SGUI_END