#ifndef HELP_H
#define HELP_H
#include "macro.h"
#include "graphics/buffers.h"
#include "graphics/shaders.h"

#define pos_loc 0
#define color_loc 1
#define textPos_loc 2

SGUI_BEG
DETAIL_BEG

const vbo &rect_obj_vbo();

shader &global_color_shader();

DETAIL_END
SGUI_END

#endif