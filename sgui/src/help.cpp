#include "help.h"

#define STR_2(x) #x
#define STR(x) STR_2(x)

SGUI_BEG
DETAIL_BEG

vbo make_rect_vbo()
{
	static vec2 text_coords[]{
		{ 0, 0 },
		{ 1, 0 },
		{ 1, 1 },
		{ 0, 1 },
	};

	vbo v;
	v.generate();
	v.attach_data(text_coords, GL_STATIC_DRAW);

	return v;
}

const vbo &rect_obj_vbo()
{
	static vbo res = make_rect_vbo();
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
		"void main() { gl_Position = SGUI_Model * SGUI_Ortho * vec4(SGUI_Pos, 0, 1); }";
	static const char *fragment =
		"#version 410 core\n"
		"uniform vec4 SGUI_Color;"
		"out vec4 SGUI_OutColor;"
		"void main() { SGUI_OutColor = SGUI_Color; }";
	static shader res = make_shader(vertex, fragment);
	return res;
}

DETAIL_END
SGUI_END