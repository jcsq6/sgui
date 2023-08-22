#include "gui/text.h"
#include "gui/window.h"

#include "graphics/buffers.h"
#include "graphics/shaders.h"

#include "utils/error.h"

#include "help.h"

#include <stdexcept>
#include <algorithm>

#include <ft2build.h>
#include FT_FREETYPE_H

SGUI_BEG

DETAIL_BEG
struct library_handle
{
	FT_Library library;

	library_handle() : library{}
	{
		if (FT_Init_FreeType(&library))
			detail::log_error(error("Could not initialize freetype.", error_code::freetype_initialization_failure));
	}
	~library_handle()
	{
		FT_Done_FreeType(library);
	}
};
DETAIL_END

detail::library_handle &get_library()
{
	static detail::library_handle lib;
	return lib;
}

font::face_handle::face_handle() : face{}, size{ default_height } {}

void font::face_handle::load(detail::library_handle &lib, const std::string &file_name)
{
	if (FT_New_Face(lib.library, file_name.data(), 0, &face))
		detail::log_error(error("Could not load font " + file_name + '.', error_code::freetype_font_failure));
}
void font::face_handle::load(detail::library_handle &lib, const void *data, std::size_t size)
{
	if (FT_New_Memory_Face(lib.library, reinterpret_cast<const FT_Byte *>(data), static_cast<FT_Long>(size), 0, &face))
		detail::log_error(error("Could not load font.", error_code::freetype_font_failure));
}

font::face_handle::~face_handle()
{
	FT_Done_Face(face);
}

font::face_handle::face_handle(face_handle &&other) noexcept : face{ other.face }, size{ other.size }
{
	other.face = nullptr;
	other.size = default_height;
}
font::face_handle &font::face_handle::operator=(face_handle &&other) noexcept
{
	FT_Done_Face(face);
	face = other.face;
	size = other.size;
	other.face = nullptr;
	other.size = default_height;

	return *this;
}

void font::face_handle::resize()
{
	FT_Set_Pixel_Sizes(face, 0, size);
}

void font::load(const std::string &file_name, unsigned int height)
{
	M_chars.clear();

	face.load(get_library(), file_name);

	face.size = height;
	face.resize();
}

void font::load(const void *data, std::size_t size, unsigned int height)
{
	M_chars.clear();

	face.load(get_library(), data, size);

	face.size = height;
	face.resize();
}

font::character::character(const font *_font, uint32_t c) : text{}, offset{}, advance{}, height{}
{
	load(_font, c);
}

font::character::character(character &&other) noexcept : text{ std::move(other.text) }, offset{ other.offset }, advance{ other.advance }, height{ other.height }
{
	other.offset = {};
	other.height = {};
	other.advance = {};
}

font::character &font::character::operator=(font::character &&other) noexcept
{
	text.destroy();

	text = std::move(other.text);
	offset = other.offset;
	advance = other.advance;
	height = other.height;

	other.offset = {};
	other.height = {};
	other.advance = {};

	return *this;
}

void font::character::load(const font *_font, uint32_t c)
{
	height = _font->get_character_height();

	FT_Face face = _font->face.face;
	if (FT_Load_Char(face, c, FT_LOAD_RENDER))
	{
		detail::log_error(error("Couldn't load character", error_code::freetype_invalid_character));
		return;
	}
	text.load(GL_RGBA, face->glyph->bitmap.buffer, face->glyph->bitmap.width, face->glyph->bitmap.rows, 1);
	offset.x = face->glyph->bitmap_left;
	offset.y = face->glyph->bitmap_top;
	advance = face->glyph->advance.x;

	text.set_parameter(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	text.set_parameter(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	text.set_parameter(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	text.set_parameter(GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	static vec4 transparent{0, 0, 0, 0};

	text.set_parameter(GL_TEXTURE_BORDER_COLOR, value(transparent));
}

font::character::character(const font::character &other)
{
	throw std::runtime_error("You shouldn't be able to get here... For some reason the copy constructor was called for character");
}
font::character &font::character::operator=(const font::character &other)
{
	throw std::runtime_error("You shouldn't be able to get here... For some reason the copy assignment was called for character");
}

namespace text_detail
{
	shader &get_shader()
	{
		static std::string vertex_source = 
			"#version 410 core\n"
			"uniform mat4 SGUI_Ortho;"
			"uniform mat4 SGUI_Model;"
			"out vec2 SGUI_VertTextPos;"
			"layout (location = " STR(pos_loc) ") in vec2 SGUI_Pos;"
			"layout (location = " STR(textPos_loc) ") in vec2 SGUI_TextPos;"
			"void main() {"
			"	gl_Position = SGUI_Ortho * SGUI_Model * vec4(SGUI_Pos, 0.0, 1.0);"
			"	SGUI_VertTextPos = SGUI_TextPos;"
			"}";
		static std::string fragment_source =
			"#version 410 core\n"
			"uniform vec4 SGUI_Color;"
			"uniform sampler2D SGUI_Texture;"
			"out vec4 SGUI_OutColor;"
			"in vec2 SGUI_VertTextPos;"
			"void main() {"
			"	SGUI_OutColor = vec4(SGUI_Color.xyz, SGUI_Color.w * texture(SGUI_Texture, SGUI_VertTextPos).r);"
			"}";
		static shader res = []()
		{
			shader res;
			res.load_from_memory(vertex_source, fragment_source);
			return res;
		}();
		return res;
	}

	class char_obj
	{
	public:
		void draw()
		{
			model = translate(M_loc);

			if (M_angle != 0)
			{
				if (M_origin != vec3{})
				{
					model *= translate(M_origin);
					model *= rot(M_angle, M_axis);
					model *= translate(-M_origin);
				}
				else
					model *= rot(M_angle, M_axis);
			}

			glDisable(GL_CULL_FACE);

			static vao buffer = detail::make_shape_vao(detail::rect_obj_vbo(), detail::text_pos_vbo());

			static auto &program = text_detail::get_shader();
			program.set_uniform("SGUI_Model", model);
			program.set_uniform("SGUI_Texture", *M_texture);

			program.bind();

			{
				auto pos_vbo = buffer.get_attribute(pos_loc);
				auto data = pos_vbo. template get_data<vec2>(GL_WRITE_ONLY);

				data[0] = {};
				data[1].x = M_sz.x;
				data[1].y = 0;
				data[2] = M_sz;
				data[3].x = 0;
				data[3].y = M_sz.y;
			}

			buffer.use();
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
		
		mat4 model;
		vec3 M_origin;
		vec3 M_axis;
		vec3 M_loc;
		vec2 M_sz;
		float M_angle;
		const texture *M_texture;
	};
}

void text::draw_raw(const window *win, vec2 absolute_min) const
{
	detail::blend_lock lock;
	detail::cull_face_lock clock;
	detail::shader_lock slock;
	detail::vao_lock vlock;
	detail::fbo_lock flock;

	static auto &program = text_detail::get_shader();
	program.set_uniform("SGUI_Color", M_col);
	program.set_uniform("SGUI_Ortho", win->ortho());

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	auto *cur = M_font->at(M_data.front());

	// remove first character's horizontal offset
	auto origin = M_origin + absolute_min;
	origin.x -= cur->offset.x * M_scale.x;

	text_detail::char_obj cspr{.model = {}, .M_origin = {}, .M_axis = { 0, 0, 1 }, .M_loc = {}, .M_sz = {}, .M_angle = M_angle };

	auto rot_origin = vec3(M_rot_origin, 0);

	for (uint32_t c : M_data)
	{
		cur = M_font->at(c);

		cspr.M_texture = &cur->text;

		vec2 sz(cur->text.get_width(), cur->text.get_height());
		vec3 cur_loc = { origin, 0 };
		cur_loc.x += cur->offset.x * M_scale.x;
		cur_loc.y += (cur->offset.y - sz.y) * M_scale.y;

		origin.x += (cur->advance >> 6) * M_scale.x;

		cspr.M_loc = cur_loc;
		cspr.M_sz = sz * M_scale;

		cspr.M_origin = rot_origin - cur_loc;

		cspr.draw();
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

bound text::get_local_rect() const
{
	update_bounds();

	bound res = M_bound;

	res.dims *= M_scale;
	res.min *= M_scale;

	return res;
}

void text::update_bounds() const
{
	if (!M_data_changed)
		return;

	M_data_changed = false;

	bound res{};

	if (M_data.empty())
	{
		M_bound = res;
		return;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	vec2 max{ -M_font->at(M_data.front())->offset.x, 0 };

	auto end = M_data.end() - 1;

	font::character const *cur;

	for (auto it = M_data.begin(); it != end; ++it)
	{
		cur = M_font->at(*it);
		max.x += cur->advance >> 6;

		if (float pot = (float)cur->offset.y - cur->text.get_height(); pot < res.min.y)
			res.min.y = pot;
		if (cur->offset.y > max.y)
			max.y = (float)cur->offset.y;
	}

	cur = M_font->at(*end);
	max.x += cur->offset.x + cur->text.get_width();

	if (float pot = (float)cur->offset.y - cur->text.get_height(); pot < res.min.y)
		res.min.y = pot;
	if (cur->offset.y > max.y)
		max.y = (float)cur->offset.y;

	res.dims = max - res.min;

	M_bound = res;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
}

void text::do_flags()
{
	if (!M_parent)
		return;

	if (M_flags & attach_flags::centered)
	{
		auto bound = get_local_rect();
		auto parent_dims = M_parent->size();

		// M_bound.dims.x * M_scale.x = .75 * parent_dims.x
		if (bound.dims.x > parent_dims.y || bound.dims.y > parent_dims.y)
		{
			if (bound.dims.x > parent_dims.x)
				M_scale.x = .75f * parent_dims.x / M_bound.dims.x;
			if (bound.dims.y > parent_dims.y)
				M_scale.y = .75f * parent_dims.y / M_bound.dims.y;
			
			bound.min = M_bound.min * M_scale;
			bound.dims = M_bound.dims * M_scale;
		}

		// min + dims / 2 = size / 2
		// min = (size - dims) / 2
		auto min = (M_parent->size() - bound.dims) / 2.f;
		M_origin = min - bound.min;
	}
}

SGUI_END