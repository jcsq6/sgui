#include "gui/application.h"
#include "gui/object.h"
#include "utils/error.h"

#include "gui/window.h"

#include "math/mat.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

SGUI_BEG
struct glew_handle
{
	static glew_handle &get_instance()
	{
		static glew_handle instance;
		return instance;
	}

	inline operator bool() const { return res == GLEW_OK; }
private:
	glew_handle()
	{
		res = glewInit();
		if (res != GLEW_OK)
			detail::log_error(error("glew initializatino failed with code " + std::to_string(res) + '.', error_code::glew_initialization_failure));
	}

	GLenum res;
};

struct glfw_handle
{
	static glfw_handle &get_instance()
	{
		static glfw_handle instance;
		return instance;
	}

	inline operator bool() const { return res == GLFW_TRUE; }
private:
	glfw_handle()
	{
		res = glfwInit();
		if (res != GLFW_TRUE)
			detail::log_error(error("GLFW initializatino failed with code " + std::to_string(res) + '.', error_code::glfw_initialization_failure));
	}

	~glfw_handle()
	{
		glfwTerminate();
	}

	int res;
};

int application::run()
{
	if (!glfw_handle::get_instance())
		return APP_FAILURE;

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

	M_window->init();
	M_window->grab_context();
	if (!glew_handle::get_instance())
		return APP_FAILURE;

	M_window->run();

	return APP_SUCCESS;
}

void application::do_flags(int flags)
{
}

SGUI_END