#ifndef WINDOW_H
#define WINDOW_H

#include "macro.h"
#include "object.h"
#include "math/mat.h"
#include "graphics/viewport.h"

#include "codes.h"

#include <string>
#include <unordered_map>

struct GLFWwindow;

SGUI_BEG

class widget;
class application;
class clickable;

class key
{
	bool was_pressed;
	bool pressed;

	friend class window;

public:
	inline key() : was_pressed{ false }, pressed{ false } {}

	inline bool is_pressed() const
	{
		return pressed;
	}

	inline bool is_repeated() const
	{
		return was_pressed && pressed;
	}

	inline bool is_initial_press() const
	{
		return pressed && !was_pressed;
	}
};

class window : public object
{
public:
	inline window(std::string_view name, ivec2 size) : M_ortho{ identity() }, M_name{ name }, M_viewport{ {}, size }, M_window_size{ size }, M_window{}, M_keys(num_keys) {}
	~window();

	void grab_context() const;

	void run();

	const key *get_key(key_code code) const { return &M_keys[static_cast<int>(code)]; }
	const key *get_mouse_button(mouse_code code) const { return M_mouse.buttons + static_cast<int>(code); }

	const mat4 &ortho() const { return M_ortho; }
	const std::string &name() const { return M_name; }
	vec2 size() const override;

	ivec2 viewport_size() const
	{
		return M_viewport.size;
	}
private:
	mat4 M_ortho;
	std::string M_name;
	viewport M_viewport;
	ivec2 M_window_size;
	GLFWwindow *M_window;

	mutable std::unordered_map<int, key> M_keys;
	static constexpr std::size_t num_keys = 122;

	struct mouse_handle
	{
		inline mouse_handle() : loc_changed{true}, buttons{} {}

		dvec2 loc;
		bool loc_changed;

		key buttons[8];

	} M_mouse;

	void init();
	void handle_children_input(const std::vector<object *> &children, vec2 absolute_min) const;
	void set_clickable_pressed(clickable *c) const;

	void draw_raw(const window *, vec2) const override;
	void on_attach(object *child) const override;

	friend application;
	friend object;

	static void cursor_position_callback(GLFWwindow *win_handle, double x, double y);
	static void framebuffer_callback(GLFWwindow *win_handle, int width, int height);
	static void windowsize_callback(GLFWwindow *win_handle, int width, int height);
};
SGUI_END

#endif