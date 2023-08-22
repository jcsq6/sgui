#ifndef WIDGETS_H
#define WIDGETS_H

#include "macro.h"
#include "object.h"
#include "math/vec.h"

#include "graphics/buffers.h"

#include <functional>

SGUI_BEG

class window;

class widget : public virtual object
{
public:
	virtual ~widget() = default;
};

class clickable : public virtual widget
{
public:
	clickable() : M_on_enter{}, M_on_exit{}, M_on_press{}, M_entered{}, M_pressed{} {}

	virtual ~clickable() = default;

	void set_enter_callback(auto on_enter) { M_on_enter = std::move(on_enter); }
	void set_exit_callback(auto on_exit) { M_on_exit = std::move(on_exit); }
	void set_press_callback(auto on_press) { M_on_press = std::move(on_press); }
	void set_release_callback(auto on_release) { M_on_release = std::move(on_release); }

	bool is_entered() const { return M_entered; }
	bool is_pressed() const { return M_pressed; }

protected:
	std::function<void()> M_on_enter;
	std::function<void()> M_on_exit;
	std::function<void()> M_on_press;
	std::function<void()> M_on_release;

	bool M_entered;
	bool M_pressed;

	friend class window;
};

class colorable : public virtual widget
{
public:
	colorable() : M_col{ 0, 0, 0, 1 } {}
	colorable(vec4 color) : M_col{ color } {}

	virtual ~colorable() = default;

	vec4 color() const { return M_col; }

	void set_color(vec4 color) { M_col = color; }

protected:
	vec4 M_col;
};

class rectangle : public virtual widget
{
public:
	rectangle() = default;
	rectangle(vec2 min, vec2 dims) : M_min{ min }, M_dims{ dims } {}

	virtual ~rectangle() = default;

	bool in_bounds(vec2 loc, vec2 absolute_min) const override;

	vec2 min() const override;
	vec2 max() const { return M_min + M_dims; }
	vec2 center() const { return M_min + M_dims / 2; }
	vec2 size() const override;

	void set_min(vec2 min) { M_min = min; }
	void set_dims(vec2 dims) { M_dims = dims; }

protected:
	vec2 M_min;
	vec2 M_dims;

	void obj_init() override;
};

class rounded_rectangle : protected rectangle
{
public:
	rounded_rectangle() : rectangle(), M_radius{}, M_pt_count{} {}
	rounded_rectangle(vec2 min, vec2 dims, float radius) : rectangle(min, dims), M_radius{ radius }, M_pt_count{}
	{
		verify_radius();
	}

	// will update buffers if object has been initialized... use sparingly
	void set_dims(vec2 new_dims, float new_radius)
	{
		M_dims = new_dims;
		M_radius = new_radius;

		verify_radius();

		if (M_has_init)
			calc_buffer();
	}

	rounded_rectangle(rounded_rectangle &&) = default;
	rounded_rectangle &operator=(rounded_rectangle &&) = default;

	float radius() const { return M_radius; }

	using rectangle::min;
	using rectangle::max;
	using rectangle::center;
	using rectangle::size;
	using rectangle::set_min;

	bool in_bounds(vec2 loc, vec2 absolute_min) const override;

protected:
	void calc_buffer();
	const vao &get_vao() const { return M_vao; }
	const vbo &get_vbo() const { return M_vbo; }
	std::size_t pt_count() const { return M_pt_count; }

	void obj_init() override;
private:
	float M_radius;

	// doesn't take into account rectangle::M_min
	vbo M_vbo;
	vao M_vao;
	std::size_t M_pt_count;

	void verify_radius();
};

class drawable_rectangle : public rectangle, public colorable
{
public:
	static ptr_handle<drawable_rectangle> make()
	{
		return ptr_handle<drawable_rectangle>(new drawable_rectangle());
	}

	static ptr_handle<drawable_rectangle> make(vec2 min, vec2 dims, vec4 color)
	{
		return ptr_handle<drawable_rectangle>(new drawable_rectangle(min, dims, color));
	}

	vec2 min() const override;
	vec2 size() const override;

	bool in_bounds(vec2 loc, vec2 absolute_min) const override;

	void draw_raw(const window *win, vec2 absolute_min) const override;
protected:
	void obj_init() override;

	drawable_rectangle() = default;
	drawable_rectangle(vec2 min, vec2 dims, vec4 color) : rectangle(min, dims), colorable(color) {}
};

class drawable_rounded_rectangle : public rounded_rectangle, public colorable
{
public:
	static ptr_handle<drawable_rounded_rectangle> make()
	{
		return ptr_handle<drawable_rounded_rectangle>(new drawable_rounded_rectangle());
	}

	static ptr_handle<drawable_rounded_rectangle> make(vec2 min, vec2 dims, float radius, vec4 color)
	{
		return ptr_handle<drawable_rounded_rectangle>(new drawable_rounded_rectangle(min, dims, radius, color));
	}

	vec2 min() const override;
	vec2 size() const override;

	bool in_bounds(vec2 loc, vec2 absolute_min) const override;

	void draw_raw(const window *win, vec2 absolute_min) const override;
protected:
	void obj_init() override;

	drawable_rounded_rectangle() = default;
	drawable_rounded_rectangle(vec2 min, vec2 dims, float radius, vec4 color) : rounded_rectangle(min, dims, radius), colorable(color) {}
};

class button : public drawable_rectangle, public clickable
{
public:
	static ptr_handle<button> make()
	{
		return ptr_handle<button>(new button());
	}

	static ptr_handle<button> make(vec2 min, vec2 dims, vec4 color)
	{
		return ptr_handle<button>(new button(min, dims, color));
	}

	vec2 min() const override;
	vec2 size() const override;

	bool in_bounds(vec2 loc, vec2 absolute_min) const override;
protected:
	void draw_raw(const window *win, vec2 absolute_min) const override;
	void obj_init() override;

	button() = default;
	button(vec2 min, vec2 dims, vec4 color) : drawable_rectangle(min, dims, color), clickable() {}
};

class rounded_button : public drawable_rounded_rectangle, public clickable
{
public:
	static ptr_handle<rounded_button> make()
	{
		return ptr_handle<rounded_button>(new rounded_button());
	}

	static ptr_handle<rounded_button> make(vec2 min, vec2 dims, float radius, vec4 color)
	{
		return ptr_handle<rounded_button>(new rounded_button(min, dims, radius, color));
	}

	vec2 min() const override;
	vec2 size() const override;

	bool in_bounds(vec2 loc, vec2 absolute_min) const override;
protected:
	void draw_raw(const window *win, vec2 absolute_min) const override;
	void obj_init() override;

	rounded_button() = default;
	rounded_button(vec2 min, vec2 dims, float radius, vec4 color) : drawable_rounded_rectangle(min, dims, radius, color), clickable() {}
};

SGUI_END

#endif