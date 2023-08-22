#ifndef HELP_H
#define HELP_H
#include "macro.h"
#include "graphics/buffers.h"
#include "graphics/shaders.h"

#define pos_loc 0
#define color_loc 1
#define textPos_loc 2

#define STR_2(x) #x
#define STR(x) STR_2(x)

SGUI_BEG
DETAIL_BEG

vbo &rect_obj_vbo();
const vbo &text_pos_vbo();

vao make_shape_vao(const vbo &points, const vbo &text_points);

shader &global_color_shader();

DETAIL_END
SGUI_END

#endif