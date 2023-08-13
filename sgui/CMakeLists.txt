cmake_minimum_required(VERSION 3.4)

add_library(sgui STATIC  "src/application.cpp" "src/error.cpp" "src/window.cpp" "src/widget.cpp" "src/shaders.cpp" "include/graphics/texture.h" "include/utils/context_lock.h" "src/texture.cpp" "src/help.h" "include/graphics/buffers.h" "src/help.cpp" "include/graphics/viewport.h" "src/object.cpp")

target_include_directories(sgui PUBLIC include)

if(MSVC)
	target_compile_options(sgui PUBLIC $<$<CONFIG:RELEASE>:/O2>)
else()
	target_compile_options(sgui PUBLIC $<$<CONFIG:DEBUG>:-g> $<$<CONFIG:RELEASE>:-O3>)
	target_link_options(sgui PUBLIC $<$<CONFIG:RELEASE>:-s>)
endif()

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glew REQUIRED)
find_package(Freetype REQUIRED)

target_link_libraries(sgui PUBLIC OpenGL::GL GLEW::GLEW glfw Freetype::Freetype stb_image)