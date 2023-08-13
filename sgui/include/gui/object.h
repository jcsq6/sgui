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
	inline object() : M_parent{} {}
	inline object(object *parent) : M_parent{ parent }
	{
		M_parent->M_children.push_back(this);
	}

	virtual ~object() = default;

	inline object *parent() const { return M_parent; }
	const std::vector<object *> children() const { return M_children; }
	
	void attach(object *parent, int flags = 0)
	{
		detach();

		M_parent = parent;
		do_flags(flags);
		M_parent->M_children.push_back(this);
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
protected:
	object *M_parent;
	std::vector<object *> M_children;
	
	// setup *this* in reference to M_parent according to flags
	virtual void do_flags(int flags) = 0;
};

SGUI_END

#endif