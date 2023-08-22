#ifndef TEXT_h
#define TEXT_H
#include "math/vec.h"
#include "graphics/texture.h"

#include "gui/widget.h"

#include <string>
#include <unordered_map>

struct FT_FaceRec_;

SGUI_BEG

DETAIL_BEG
struct library_handle;
DETAIL_END

class window;

struct bound
{
	vec2 min;
	vec2 dims;

	vec2 max() const
	{
		return min + dims;
	}
};

class font
{
public:
	static constexpr unsigned int default_height = 48;

	font() : M_chars(256) {}

	font(const std::string &file_name, unsigned int height) : font()
	{
		load(file_name, height);
	}
	font(const void *data, std::size_t size, unsigned int height) : font()
	{
		load(data, size, height);
	}

	void load(const std::string &file_name, unsigned int height);
	void load(const void *data, std::size_t size, unsigned int height);

	unsigned int get_character_height() const { return face.size; }

private:
	friend class text;

	struct face_handle
	{
		FT_FaceRec_ *face;
		unsigned int size;

		face_handle();
		~face_handle();

		void load(detail::library_handle &lib, const std::string &file_name);
		void load(detail::library_handle &lib, const void *data, std::size_t size);

		face_handle(face_handle &&other) noexcept;
		face_handle &operator=(face_handle &&other) noexcept;

		void resize();
	};

	face_handle face;

	struct character
	{
		character() noexcept : text{}, offset{}, advance{}, height{} {}

		character(const font *_font, uint32_t c);
		~character() = default;

		character(character &&other) noexcept;
		character &operator=(character &&other) noexcept;

		void load(const font *_font, uint32_t c);

		texture text;

		ivec2 offset;
		unsigned int advance;

		unsigned int height;
	};

	character const *at(uint32_t c) const
	{
		if (!M_chars.count(c))
			return &M_chars.emplace(c, character{ this, c }).first->second;
		return &M_chars.at(c);
	}

	// mutable to allow potential addition of new characters in draw function
	mutable std::unordered_map<uint32_t, character> M_chars;
};

class text : public colorable
{
public:
	static ptr_handle<text> make()
	{
		return ptr_handle<text>(new text());
	}

	static ptr_handle<text> make(font &_font)
	{
		return ptr_handle<text>(new text(_font));
	}

	static ptr_handle<text> make(std::basic_string_view<char> txt, font &_font)
	{
		return ptr_handle<text>(new text(txt, _font));
	}

	static ptr_handle<text> make(std::basic_string_view<wchar_t> txt, font &_font)
	{
		return ptr_handle<text>(new text(txt, _font));
	}

	static ptr_handle<text> make(std::basic_string_view<uint32_t> txt, font &_font)
	{
		return ptr_handle<text>(new text(txt, _font));
	}

	void set_string(std::basic_string_view<char> txt)
	{
		M_data.assign(txt.begin(), txt.end());
		M_data_changed = true;
	}

	void set_string(std::basic_string_view<wchar_t> txt)
	{
		M_data.assign(txt.begin(), txt.end());
		M_data_changed = true;
	}

	void set_string(std::basic_string_view<uint32_t> txt)
	{
		M_data.assign(txt.begin(), txt.end());
		M_data_changed = true;
	}

	const std::basic_string<uint32_t> &get_string() const
	{
		return M_data;
	}

	void clear()
	{
		M_data.clear();
		M_data_changed = true;
	}

	template <typename... Ts>
	void insert(Ts &&...args)
	{
		M_data.insert(std::forward<Ts>(args)...);
		M_data_changed = true;
	}

	template <typename... Ts>
	void erase(Ts &&...args)
	{
		M_data.erase(std::forward<Ts>(args)...);
		M_data_changed = true;
	}

	void push_back(uint32_t c)
	{
		M_data.push_back(c);
		M_data_changed = true;
	}

	void pop_back()
	{
		M_data.pop_back();
		M_data_changed = true;
	}

	template <typename... Ts>
	void append(Ts &&...args)
	{
		M_data.append(std::forward<Ts>(args)...);
		M_data_changed = true;
	}

	template <typename... Ts>
	void replace(Ts &&...args)
	{
		M_data.replace(std::forward<Ts>(args)...);
		M_data_changed = true;
	}

	template <typename... Ts>
	void resize(Ts... args)
	{
		M_data.resize(args...);
		M_data_changed = true;
	}

	void set_text_origin(vec3 origin) { M_origin = origin; }
	vec3 get_text_origin() const { return M_origin; }

	void set_scale(vec2 scale) { M_scale = scale; }
	vec2 get_scale() const { return M_scale; }

	// rot_origin is local to the object, not the world
	void set_rot_origin(vec3 origin) { M_rot_origin = origin; }
	vec3 get_rot_origin() const { return M_rot_origin; }

	void set_angle(float angle) { M_angle = angle; }
	float get_angle() const { return M_angle; }

	void set_font(font &_font) noexcept { M_font = &_font; }
	font const *get_font() const noexcept { return M_font; }

	/// @brief get's rect with local bounds of the text with the origin as (0,0). The minimum of the returned rect is not neccessarily (0,0).
	///		   To get a desired minimum, set the origin to (DesiredMin) - (get_local_rect().min)
	/// @return bound including the min of the text bounds, and the dimensions
	bound get_local_rect() const;

	void draw_raw(const window *win, vec2 absolute_min) const override;
private:
	std::basic_string<uint32_t> M_data;
	// M_bound doesn't take into account M_scale
	mutable bound M_bound;
	vec2 M_origin;
	vec2 M_rot_origin;
	vec2 M_scale;
	float M_angle;
	font *M_font;
	mutable bool M_data_changed;

	void update_bounds() const;

	void obj_init() override;

protected:
	text() :
		M_origin{},
		M_scale{ 1, 1 },
		M_rot_origin{},
		M_font{},
		M_angle{},
		M_data_changed{ true }
	{
	}
	text(font &_font) :
		M_origin{},
		M_scale{ 1, 1 },
		M_rot_origin{},
		M_font{ &_font },
		M_angle{},
		M_data_changed{ true }
	{
	}
	text(std::basic_string_view<char> txt, font &_font) :
		M_origin{},
		M_scale{ 1, 1 },
		M_rot_origin{},
		M_font{ &_font },
		M_angle{},
		M_data{ txt.begin(), txt.end() },
		M_data_changed{ true }
	{
	}

	text(std::basic_string_view<wchar_t> txt, font &_font) :
		M_origin{},
		M_scale{ 1, 1 },
		M_rot_origin{},
		M_font{ &_font },
		M_angle{},
		M_data{ txt.begin(), txt.end() },
		M_data_changed{ true }
	{
	}

	text(std::basic_string_view<uint32_t> txt, font &_font) :
		M_origin{},
		M_scale{ 1, 1 },
		M_rot_origin{},
		M_font{ &_font },
		M_angle{},
		M_data{ txt.begin(), txt.end() },
		M_data_changed{ true }
	{
	}
};
SGUI_END
#endif