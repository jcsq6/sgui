#ifndef WIDGETS_H
#define WIDGETS_H

#include "macro.h"
#include "object.h"
#include "math/vec.h"

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

class button : public virtual rectangle, public virtual colorable, public virtual clickable
{
public:
	button() = default;
	button(vec2 min, vec2 dims, vec4 color) : rectangle(min, dims), colorable(color), clickable() {}

	vec2 min() const override;
	vec2 size() const override;
	bool in_bounds(vec2 loc, vec2 absolute_min) const override;
protected:
	void draw_raw(const window *win, vec2 absolute_min) const override;
	void do_flags(int flags) override;
};

SGUI_END

#endif