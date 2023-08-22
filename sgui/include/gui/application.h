#ifndef APP_H
#define APP_H

#include "macro.h"
#include "window.h"

SGUI_BEG

constexpr int APP_SUCCESS = 0;
constexpr int APP_FAILURE = 1;

class application : private object
{
public:
	int run();

	inline void set_window(window &win) { M_window = &win; win.M_parent = this; }
private:
	window* M_window;

	friend window;
};

SGUI_END

#endif