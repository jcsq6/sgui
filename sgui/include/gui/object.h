#ifndef OBJECT_H
#define OBJECT_H

#include "macro.h"
#include "math/vec.h"
#include <vector>
#include <algorithm>

SGUI_BEG

namespace attach_flags
{
	enum attach_flags
	{
		centered = 0x01,
	};
}

class window;

class object
{
public:
	inline object() : M_parent{}, M_flags{} {}
	inline object(object *parent) : M_parent{ parent }, M_flags{}
	{
		M_parent->M_children.push_back(this);
	}

	virtual ~object() = default;

	inline object *parent() const { return M_parent; }
	const std::vector<object *> children() const { return M_children; }
	
	void attach(object &parent, int flags = 0)
	{
		detach();

		M_flags = flags;
		M_parent = &parent;
		parent.M_children.push_back(this);
		parent.on_attach(this);
	}

	void detach()
	{
		if (M_parent)
		{
			if (auto loc = std::find(M_parent->M_children.begin(), M_parent->M_children.end(), this); loc != M_parent->M_children.end())
				M_parent->M_children.erase(loc);
			M_parent = nullptr;
		}
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
	
	// call before use (will call do_flags for this and M_children)
	void setup();
protected:
	object *M_parent;
	std::vector<object *> M_children;
	int M_flags;
	
	// setup *this* in reference to M_parent according to M_flags
	virtual void do_flags();
	// function in parent class when a child *child* is attached to it
	virtual void on_attach(object *child) const;
};

SGUI_END

#endif