#include "gui/window.h"
#include "gui/application.h"

#include "gui/widget.h"

#include "utils/error.h"
#include "utils/context_lock.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

SGUI_BEG

window::~window()
{
	glfwDestroyWindow(M_window);
	M_window = nullptr;
}

void window::grab_context() const
{
	glfwMakeContextCurrent(M_window);
}

void window::draw_raw(const window *, vec2) const
{
	detail::vao_lock vaolock;
	detail::fbo_lock flock;
	detail::cull_face_lock clock;
	detail::shader_lock slock;
	detail::viewport_lock vlock;

	grab_context();
	
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	M_viewport.apply();

	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	for (const auto &w : M_children)
		w->draw_raw(this, {});
	glfwSwapBuffers(M_window);
}

void window::on_attach(object *child) const
{
	if (M_has_init)
		child->setup();
}

void window::handle_children_input(const std::vector<std::shared_ptr<object>> &children, vec2 absolute_min) const
{
	if (M_mouse.loc_changed)
	{
		for (const auto &w : children)
		{
			if (clickable *c = dynamic_cast<clickable *>(w.get()))
			{
				bool in_bounds = c->in_bounds(M_mouse.loc, absolute_min);

				// if the mouse has moved and it's in bounds
				if (in_bounds)
				{
					// if it's just entered
					if (!c->M_entered)
					{
						c->M_entered = true;
						if (c->M_on_enter)
							c->M_on_enter();
					}
				}
				// if it's not in bounds and it's just left
				else if (c->M_entered)
				{
					c->M_entered = false;
					if (c->M_on_exit)
						c->M_on_exit();
				}

				set_clickable_pressed(c);
			}

			if (w->children().size())
				handle_children_input(w->children(), absolute_min + w->min());
		}
	}
	else
	{
		for (const auto &w : children)
		{
			if (clickable *c = dynamic_cast<clickable *>(w.get()))
				set_clickable_pressed(c);

			if (w->children().size())
				handle_children_input(w->children(), absolute_min + w->min());
		}
	}
}

void window::set_clickable_pressed(clickable *c) const
{
	// if it's in bounds
	if (c->M_entered)
	{
		// if it's in bounds and the left button has just been pressed
		if (M_mouse.buttons[GLFW_MOUSE_BUTTON_LEFT].is_initial_press())
		{
			c->M_pressed = true;
			if (c->M_on_press)
				c->M_on_press();
		}
		// if it's in bounds and the left button has just been released
		else if (!M_mouse.buttons[GLFW_MOUSE_BUTTON_LEFT].is_pressed() && c->M_pressed)
		{
			c->M_pressed = false;
			if (c->M_on_release)
				c->M_on_release();
		}
	}
	// if it's not in bounds and the left button is released
	else if (!M_mouse.buttons[GLFW_MOUSE_BUTTON_LEFT].is_pressed())
		c->M_pressed = false;
}

void window::run()
{
	while (!glfwWindowShouldClose(M_window))
	{
		M_mouse.loc_changed = false;

		glfwWaitEvents();

		for (auto &k : M_keys)
		{
			k.second.was_pressed = k.second.pressed;
			k.second.pressed = glfwGetKey(M_window, k.first);
		}

		for (int button = 0; button < GLFW_MOUSE_BUTTON_LAST; ++button)
		{
			auto &b = M_mouse.buttons[button];
			b.was_pressed = b.pressed;
			b.pressed = glfwGetMouseButton(M_window, button);
		}
		
		handle_children_input(M_children, {});

		draw_raw(nullptr, {});
	}
}

vec2 window::size() const
{
	return viewport_size();
}

void window::create()
{
	M_window = glfwCreateWindow(M_viewport.size.x, M_viewport.size.y, M_name.c_str(), nullptr, nullptr);

	if (!M_window)
	{
		detail::log_error(error("Couldn't create window.", error_code::window_creation_failure));
		return;
	}

	M_ortho = ortho_mat(0, M_viewport.size.x, 0, M_viewport.size.y, -1, 1);

	glfwSetCursorPosCallback(M_window, cursor_position_callback);
	glfwSetFramebufferSizeCallback(M_window, framebuffer_callback);
	glfwSetWindowSizeCallback(M_window, windowsize_callback);
	glfwSetWindowUserPointer(M_window, this);
}

void mouse_pos_interp(dvec2 window_size, dvec2 target_size, dvec2 &mouse_pos)
{
	mouse_pos.x = target_size.x / window_size.x * mouse_pos.x;
	mouse_pos.y = target_size.y / -window_size.y * (mouse_pos.y - window_size.y);
}

void window::cursor_position_callback(GLFWwindow *win_handle, double x, double y)
{
	window &win = *reinterpret_cast<window *>(glfwGetWindowUserPointer(win_handle));

	win.M_mouse.loc.x = x;
	win.M_mouse.loc.y = y;

	mouse_pos_interp(win.M_window_size, win.M_viewport.size, win.M_mouse.loc);

	win.M_mouse.loc_changed = true;
}

void window::framebuffer_callback(GLFWwindow *win_handle, int width, int height)
{
	window &win = *reinterpret_cast<window *>(glfwGetWindowUserPointer(win_handle));
	win.M_viewport.size.x = width;
	win.M_viewport.size.y = height;
	win.M_ortho = ortho_mat(0, width, 0, height, -1, 1);
}

void window::windowsize_callback(GLFWwindow *win_handle, int width, int height)
{
	window &win = *reinterpret_cast<window *>(glfwGetWindowUserPointer(win_handle));
	win.M_window_size.x = width;
	win.M_window_size.y = height;
}


SGUI_END