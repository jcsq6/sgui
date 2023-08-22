#include "gui/widget.h"
#include "gui/window.h"

#include "help.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

SGUI_BEG

// RECTANGLE

inline bool in_rectangle(vec2 loc, vec2 min, vec2 max)
{
	return (loc.x < max.x && loc.x > min.x) && (loc.y < max.y && loc.y > min.y);
}

bool rectangle::in_bounds(vec2 loc, vec2 absolute_min) const
{
	auto min = M_min + absolute_min;
	auto max = min + M_dims;
	return in_rectangle(loc, min, max);
}

vec2 rectangle::min() const
{
	return M_min;
}

vec2 rectangle::size() const
{
	return M_dims;
}

void rectangle::obj_init()
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

// ROUNDED RECTANGLE

bool rounded_rectangle::in_bounds(vec2 loc, vec2 absolute_min) const
{
	auto min = M_min + absolute_min;
	float rad2 = M_radius * M_radius;
	return
		in_rectangle(loc, { min.x, min.y + M_radius }, { min.x + M_dims.x, min.y + M_dims.y - M_radius }) ||
		in_rectangle(loc, { min.x + M_radius, min.y }, { min.x + M_dims.x - M_radius, min.y + M_dims.y }) ||
		// top left
		distance2(loc, { min.x + M_radius, min.y + M_dims.y - M_radius }) < rad2 ||
		// top right
		distance2(loc, { min.x + M_dims.x - M_radius, min.y + M_dims.y - M_radius }) < rad2 ||
		// bottom left
		distance2(loc, { min.x + M_radius, min.y + M_radius }) < rad2 ||
		// bottom right
		distance2(loc, { min.x + M_dims.x - M_radius, min.y + M_radius }) < rad2;
}

void do_ascending_circle(std::vector<vec2> &pts, vec2 &cur, vec2 center, float end, float dx, float rad2)
{
	for (; cur.x < end; cur.x += dx)
	{
		float xpart = (cur.x - center.x);
		cur.y = std::sqrt(rad2 - xpart * xpart) + center.y;
		pts.push_back(cur);
	}
}

void do_descending_circle(std::vector<vec2> &pts, vec2 &cur, vec2 center, float end, float dx, float rad2)
{
	for (; cur.x > end; cur.x -= dx)
	{
		float xpart = (cur.x - center.x);
		cur.y = -std::sqrt(rad2 - xpart * xpart) + center.y;
		pts.push_back(cur);
	}
}

void rounded_rectangle::calc_buffer()
{
	constexpr unsigned int num_circle_pts = 50;

	float dx = M_radius / num_circle_pts;
	float rad2 = M_radius * M_radius;

	// use vector in case it goes over for floating point reasons. Idk if this is actually a possiblity but whatever
	std::vector<vec2> pts;
	pts.reserve(num_circle_pts * 4 + 4);

	// top left
	vec2 cur = { 0, M_dims.y - M_radius };
	vec2 center = { M_radius, cur.y };

	pts.push_back(cur);
	cur.x += dx;
	do_ascending_circle(pts, cur, center, M_radius, dx, rad2);
	

	// top right
	cur = { M_dims.x - M_radius, M_dims.y };
	center.x = cur.x;

	pts.push_back(cur);
	cur.x += dx;
	do_ascending_circle(pts, cur, center, M_dims.x, dx, rad2);


	// bottom right
	cur = { M_dims.x, M_radius };
	center.y = M_radius;
	
	pts.push_back(cur);
	cur.x -= dx;
	do_descending_circle(pts, cur, center, M_dims.x - M_radius, dx, rad2);

	// bottom left
	cur = { M_radius, 0 };
	center.x = M_radius;

	pts.push_back(cur);
	cur.x -= dx;
	do_descending_circle(pts, cur, center, 0, dx, rad2);

	if (!M_vbo.index())
		M_vbo.generate();
	M_vbo.attach_data(pts, GL_STATIC_DRAW);

	if (!M_vao.index())
		M_vao = detail::make_shape_vao(M_vbo, nullptr);

	M_pt_count = pts.size();
}

void rounded_rectangle::verify_radius()
{
	float min_2 = (M_dims.x < M_dims.y ? M_dims.x : M_dims.y) / 2.f;

	if (M_radius > min_2)
		M_radius = min_2;
	else if (M_radius < 0)
		M_radius = 0;
}

void rounded_rectangle::obj_init()
{
	calc_buffer();
	rectangle::obj_init();
}

// DRAWABLE RECTANGLE

vec2 drawable_rectangle::min() const
{
	return rectangle::min();
}

vec2 drawable_rectangle::size() const
{
	return rectangle::size();
}

bool drawable_rectangle::in_bounds(vec2 loc, vec2 absolute_min) const
{
	return rectangle::in_bounds(loc, absolute_min);
}

void drawable_rectangle::draw_raw(const window *win, vec2 absolute_min) const
{
	static vao v = detail::make_shape_vao(detail::rect_obj_vbo(), nullptr);
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

	for (const auto &child : M_children)
		child->draw_raw(win, min);
}

void drawable_rectangle::obj_init()
{
	rectangle::obj_init();
}

// DRAWABLE ROUNDED RECTANGLE

vec2 drawable_rounded_rectangle::min() const
{
	return rectangle::min();
}

vec2 drawable_rounded_rectangle::size() const
{
	return rectangle::size();
}

bool drawable_rounded_rectangle::in_bounds(vec2 loc, vec2 absolute_min) const
{
	return rectangle::in_bounds(loc, absolute_min);
}

void drawable_rounded_rectangle::draw_raw(const window *win, vec2 absolute_min) const
{
	static auto &program = detail::global_color_shader();

	if (!win)
		return;

	auto min = absolute_min + M_min;

	glDisable(GL_CULL_FACE);

	program.set_uniform("SGUI_Color", M_col);
	program.set_uniform("SGUI_Ortho", win->ortho());
	program.set_uniform("SGUI_Model", translate({ min, 0 }));

	program.bind();

	get_vao().use();
	glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(pt_count()));

	for (const auto &child : M_children)
		child->draw_raw(win, min);
}

void drawable_rounded_rectangle::obj_init()
{
	rectangle::obj_init();
}

// BUTTON

vec2 button::min() const
{
	return rectangle::min();
}

vec2 button::size() const
{
	return rectangle::size();
}

bool button::in_bounds(vec2 loc, vec2 absolute_min) const
{
	return rectangle::in_bounds(loc, absolute_min);
}

void button::draw_raw(const window *win, vec2 absolute_min) const
{
	drawable_rectangle::draw_raw(win, absolute_min);
}

void button::obj_init()
{
	rectangle::obj_init();
}

// ROUNDED BUTTON
vec2 rounded_button::min() const
{
	return rectangle::min();
}

vec2 rounded_button::size() const
{
	return rectangle::size();
}

bool rounded_button::in_bounds(vec2 loc, vec2 absolute_min) const
{
	return rounded_rectangle::in_bounds(loc, absolute_min);
}

void rounded_button::draw_raw(const window *win, vec2 absolute_min) const
{
	drawable_rounded_rectangle::draw_raw(win, absolute_min);
}

void rounded_button::obj_init()
{
	rounded_rectangle::obj_init();
}

SGUI_END