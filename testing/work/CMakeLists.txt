cmake_minimum_required(VERSION 3.4)


set(DO_WINDOWED false)

if(${DO_WINDOWED})
	add_executable(work WIN32 main.cpp)
	target_compile_definitions(work PUBLIC WINDOWED_APP=1)
else()
	add_executable(work main.cpp)
	target_compile_definitions(work PUBLIC WINDOWED_APP=1)
endif()

target_include_directories(work PUBLIC include)

if(MSVC)
	target_compile_options(work PUBLIC $<$<CONFIG:RELEASE>:/O2>)
else()
	target_compile_options(work PUBLIC $<$<CONFIG:DEBUG>:-g> $<$<CONFIG:RELEASE>:-O3>)
	target_link_options(work PUBLIC $<$<CONFIG:RELEASE>:-s>)
endif()

target_link_libraries(work PUBLIC sgui)
target_include_directories(work PUBLIC src)