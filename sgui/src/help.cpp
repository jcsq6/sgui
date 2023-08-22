#include "help.h"

SGUI_BEG
DETAIL_BEG

vbo &rect_obj_vbo()
{
	static vbo res = []()
	{
		vbo res;
		res.generate();
		res.reserve_data(sizeof(vec2) * 4, GL_STATIC_DRAW);
		return res;
	}();

	return res;
}

const vbo &text_pos_vbo()
{
	static vec2 text_coords[]{
		{ 0, 0 },
		{ 1, 0 },
		{ 1, 1 },
		{ 0, 1 },
	};

	static vbo res = []() {
		vbo res;
		res.generate();
		res.attach_data(text_coords, GL_STATIC_DRAW);
		return res;
	}();

	return res;
}

shader make_shader(const char *vertex, const char *fragment)
{
	shader res;
	res.load_from_memory(vertex, fragment);
	return res;
}

shader &global_color_shader()
{
	static const char *vertex =
		"#version 410 core\n"
		"uniform vec4 SGUI_Color;"
		"uniform mat4 SGUI_Ortho;"
		"uniform mat4 SGUI_Model;"
		"layout (location = " STR(pos_loc) ") in vec2 SGUI_Pos;"
		"void main() { gl_Position = SGUI_Ortho * SGUI_Model * vec4(SGUI_Pos, 0, 1); }";
	static const char *fragment =
		"#version 410 core\n"
		"uniform vec4 SGUI_Color;"
		"out vec4 SGUI_OutColor;"
		"void main() { SGUI_OutColor = SGUI_Color; }";
	static shader res = make_shader(vertex, fragment);
	return res;
}

vao make_shape_vao(const vbo &points, const vbo &text_points)
{
	detail::vao_lock lvao;
	detail::vbo_lock lvbo;

	vao res;
	res.generate();

	res.use();

	points.use();
	glEnableVertexAttribArray(pos_loc);
	glVertexAttribPointer(pos_loc, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

	text_points.use();
	glEnableVertexAttribArray(textPos_loc);
	glVertexAttribPointer(textPos_loc, 2, GL_FLOAT, GL_FALSE, 0, (void *)0);

	return res;
}

DETAIL_END
SGUI_END