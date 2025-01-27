# CMake Header
project(Euphorbe LANGUAGES C CXX)

cmake_minimum_required(VERSION 3.8)

# Settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Dependency settings
set(IMGUI_STATIC "on")

# Dependencies
add_subdirectory(ThirdParty/volk)
add_subdirectory(ThirdParty/cimgui)
add_subdirectory(ThirdParty/cglm)

# ImGui impl
add_library(imgui_impl STATIC ThirdParty/cimgui/imgui/backends/imgui_impl_win32.h 
							  ThirdParty/cimgui/imgui/backends/imgui_impl_win32.cpp 
							  ThirdParty/imgui_backends/imgui_impl_vulkan.h 
							  ThirdParty/imgui_backends/imgui_impl_vulkan.cpp
							  ThirdParty/imgui_backends/cimgui_impl.h)
target_include_directories(imgui_impl PRIVATE ThirdParty/cimgui/imgui $ENV{VULKAN_SDK}/Include ThirdParty/volk)
target_link_libraries(imgui_impl PUBLIC cimgui)
target_compile_definitions(imgui_impl PRIVATE IMGUI_IMPL_API=extern\ \"C\")
target_compile_definitions(imgui_impl PRIVATE VK_NO_PROTOTYPES)

# Toml C99
add_library(TOML STATIC ThirdParty/tomlc99/toml.h ThirdParty/tomlc99/toml.c)

# Main Projects
add_subdirectory(src)
add_subdirectory(Editor)