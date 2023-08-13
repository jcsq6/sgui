#ifndef VIEWPORT_H
#define VIEWPORT_H

#include "macro.h"
#include "math/vec.h"

#include <GL/glew.h>

SGUI_BEG
struct viewport
{
	ivec2 pos;
	ivec2 size;

	inline void apply() const
	{
		glViewport(pos.x, pos.y, size.x, size.y);
	}
};
SGUI_END

#endif