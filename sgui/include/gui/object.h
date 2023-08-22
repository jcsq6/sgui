#ifndef OBJECT_H
#define OBJECT_H

#include "macro.h"
#include "math/vec.h"
#include <vector>
#include <algorithm>
#include <memory>

SGUI_BEG

namespace attach_flags
{
	enum attach_flags
	{
		centered = 0x01,
	};
}

template <typename T>
using ptr_handle = std::shared_ptr<T>;

class window;

class object
{
public:
	inline object() : M_parent{}, M_flags{}, M_has_init{} {}
	virtual ~object() = default;

	inline object *parent() const { return M_parent; }
	const std::vector<ptr_handle<object>> children() const { return M_children; }

	void add_child(const ptr_handle<object> &child, int flags = 0)
	{
		add_child(ptr_handle<object>(child), flags);
	}

	void add_child(ptr_handle<object> &&child, int flags = 0)
	{
		child->M_parent = this;
		child->M_flags = flags;
		on_attach(child.get());
		M_children.push_back(std::move(child));
	}

	// draws *this* in reference to absolute_min (second argument) with no setup
	virtual void draw_raw(const window *, vec2) const;

	virtual vec2 size() const;

	// returns minimum point for object in reference to parent's minimum
	virtual vec2 min() const;

	bool in_bounds(vec2 loc) const;

	// for optimization purposes
	// absolute min is the absolute min of the M_parent
	virtual bool in_bounds(vec2 loc, vec2 absolute_min) const;
	
	// returns minimum point for object in reference to greatest-grandparent/parent's minimum
	vec2 absolute_min() const;
	
	// call before use (will call obj_init for this and M_children)
	void setup();
protected:
	object *M_parent;
	std::vector<ptr_handle<object>> M_children;
	int M_flags;
	// useful when a child of object manages buffers or other openGL related objects
	bool M_has_init;
	
	// setup *this* in reference to M_parent according to M_flags, and also initialize class for use with windows
	virtual void obj_init();
	// function in parent class when a child *child* is attached to it
	virtual void on_attach(object *child) const;
};

SGUI_END

#endif