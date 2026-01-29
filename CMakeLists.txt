cmake_minimum_required(VERSION 3.16)
project(SystemCleaner LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
	imgui
	GIT_REPOSITORY https://github.com/ocornut/imgui.git
	GIT_TAG v1.92.2b-docking
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
	glfw
	GIT_REPOSITORY https://github.com/glfw/glfw.git
	GIT_TAG 3.4
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
	bs_thread_pool
	GIT_REPOSITORY https://github.com/bshoshany/thread-pool.git
	GIT_TAG v4.1.0
)
FetchContent_MakeAvailable(bs_thread_pool)

FetchContent_Declare(
	stb
	GIT_REPOSITORY https://github.com/nothings/stb.git
	GIT_TAG master
)
FetchContent_MakeAvailable(stb)

add_executable(SystemCleaner WIN32
	source/main.cpp
	${imgui_SOURCE_DIR}/imgui.cpp
	${imgui_SOURCE_DIR}/imgui_draw.cpp
	${imgui_SOURCE_DIR}/imgui_widgets.cpp
	${imgui_SOURCE_DIR}/imgui_tables.cpp

	${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
	${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

if (MSVC)
    target_link_options(SystemCleaner PRIVATE "/ENTRY:mainCRTStartup")
endif()

set(GUI_DIR ${CMAKE_SOURCE_DIR}/source/gui)
set(APP_DIR ${CMAKE_SOURCE_DIR}/source/app)
set(UTILS_DIR ${CMAKE_SOURCE_DIR}/source/utils)
set(CORE_DIR ${CMAKE_SOURCE_DIR}/source/core)
set(COMMON_DIR ${CMAKE_SOURCE_DIR}/source/common)
set(WIDGETS_DIR ${GUI_DIR}/widgets)

target_include_directories(SystemCleaner PRIVATE
	${imgui_SOURCE_DIR}
	${imgui_SOURCE_DIR}/backends

	${CMAKE_SOURCE_DIR}/source

	${bs_thread_pool_SOURCE_DIR}/include

	${stb_SOURCE_DIR}
)

target_sources(SystemCleaner PRIVATE
	${GUI_DIR}/gui.cpp
	${GUI_DIR}/gui.hpp

	${APP_DIR}/app.cpp
	${APP_DIR}/app.hpp

	${WIDGETS_DIR}/title_bar.cpp
	${WIDGETS_DIR}/title_bar.hpp
	${WIDGETS_DIR}/cleaner_panel.cpp
	${WIDGETS_DIR}/cleaner_panel.hpp

	${CORE_DIR}/system_cleaner.cpp
	${CORE_DIR}/system_cleaner.hpp
	${CORE_DIR}/task_manager.cpp
	${CORE_DIR}/task_manager.hpp
	${CORE_DIR}/texture_manager.cpp
	${CORE_DIR}/texture_manager.hpp
	${CORE_DIR}/window.cpp
	${CORE_DIR}/window.hpp

	${UTILS_DIR}/custom_widgets.hpp
	${UTILS_DIR}/dialogs.cpp
	${UTILS_DIR}/dialogs.hpp
	${UTILS_DIR}/filesystem.cpp
	${UTILS_DIR}/filesystem.hpp
	${UTILS_DIR}/path_validator.cpp
	${UTILS_DIR}/path_validator.hpp

	${COMMON_DIR}/cleaner_info.hpp
	${COMMON_DIR}/constants.hpp
	${COMMON_DIR}/id_generator.hpp
	${COMMON_DIR}/scoped_guards.hpp
	${COMMON_DIR}/types.hpp
)

target_link_libraries(SystemCleaner PRIVATE
	glfw
	opengl32
)

target_compile_definitions(SystemCleaner PRIVATE
	PROJECT_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
	PROJECT_VERSION="1.0"
)