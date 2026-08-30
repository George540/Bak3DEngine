/* ===========================================================================
The MIT License (MIT)

Copyright (c) 2022-2026 George Mavroeidis - GeoGraphics

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
=========================================================================== */

// =====================================================================================
// EventManager class was made by George Mavroeidis using reference from the
// EventManager class from the COMP 371 lab 10. It was only used as an inspiration for
// =====================================================================================

#include <iostream>

#include "event_manager.h"

#include <imgui_impl_glfw.h>
#include <stb_image.h>

#include "imgui.h"
#include "Core/logger.h"

#include "GLFW/glfw3.h"

using namespace std;

// Time
double EventManager::last_frame_time = glfwGetTime();
double EventManager::last_frame_time_fps = glfwGetTime();
float EventManager::frame_time = 0.0f;
float EventManager::frame_time_max = 0.0f;
int EventManager::nb_frames = 0;
int EventManager::frames_per_second = 0;

// Mouse
double EventManager::mouse_pos_x = 0.0;
double EventManager::mouse_pos_y = 0.0;
double EventManager::last_mouse_position_x = 0.0;
double EventManager::last_mouse_position_y = 0.0;
double EventManager::delta_x = 0.0;
double EventManager::delta_y = 0.0;
double EventManager::camera_scroll_offset = 0.0;
float EventManager::mouse_sensitivity = 0.1f;

// Window
GLFWwindow* EventManager::m_window = nullptr;
GLFWmonitor* EventManager::m_monitor = nullptr;
const GLFWvidmode* EventManager::m_vid_mode = nullptr;
int EventManager::m_window_width = 1920; // defaulting to 1920 / 1080 just in case
int EventManager::m_window_height = 1080;
int EventManager::m_viewport_width = 1920;
int EventManager::m_viewport_height = 1080;
bool EventManager::is_camera_looking_enabled = false;
bool EventManager::is_scrolling_enabled = false;

/*
 * Initializes the proper GLFW window settings and handles all inputs.
 */
void EventManager::initialize()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		B3D_LOG_ERROR("Failed to initialize GLFW.");
		return;
	}

	// Prefer modern OpenGL
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// MacOS requires this for core profile
#ifdef PLATFORM_OSX
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// Sensible defaults (more widely supported than 32-bit)
	glfwWindowHint(GLFW_DEPTH_BITS, 24);

	// Open a window and create its OpenGL context Window behavior
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

	// Get primary monitor & video mode
	m_monitor = glfwGetPrimaryMonitor();
	m_vid_mode = glfwGetVideoMode(m_monitor);

	// Match monitor format
	glfwWindowHint(GLFW_RED_BITS, m_vid_mode->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, m_vid_mode->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, m_vid_mode->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, m_vid_mode->refreshRate);

	m_window = glfwCreateWindow(m_vid_mode->width, m_vid_mode->height, "Bak3D Engine", nullptr, nullptr);

	// Fallback by loosening rules. Falling back to older versions will disable modern features such as compute shaders.
	if (!m_window)
	{
		B3D_LOG_WARNING("Modern OpenGL context failed. Attempting fallback...");

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

		glfwWindowHint(GLFW_SAMPLES, 0);
		glfwWindowHint(GLFW_DEPTH_BITS, 24);

		m_window = glfwCreateWindow(m_vid_mode->width, m_vid_mode->height, "Bak3D Engine", nullptr, nullptr);
	}

	// Final failure
	if (!m_window)
	{
		B3D_LOG_ERROR("Failed to create GLFW window with OpenGL context.");
		glfwTerminate();
		return;
	}

	// Make context current
	glfwMakeContextCurrent(m_window);

	// Get actual window size
	glfwGetWindowSize(m_window, &m_window_width, &m_window_height);
	B3D_LOG_INFO("Window created with size %d x %d", m_window_width, m_window_height);

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(m_window, GLFW_STICKY_KEYS, GL_TRUE);

	toggle_vsync(false);

	set_windows_application_icon();

	// Initial time
	last_frame_time = glfwGetTime();
	srand(static_cast<unsigned int>(time(nullptr)));  // NOLINT(cert-msc51-cpp)
	B3D_LOG_INFO("Event Manager Initialization Complete.");
}

void EventManager::shutdown()
{
	// Close OpenGL window and terminate GLFW
	glfwDestroyWindow(m_window);
	glfwTerminate();
	m_window = nullptr;
	B3D_LOG_INFO("Event Manager Termination Complete.");
}

void EventManager::begin_update()
{
	// Update inputs/events
	glfwPollEvents();

	// Update GLFW components
	glfwGetCursorPos(m_window, &mouse_pos_x, &mouse_pos_y);
	glfwGetWindowSize(m_window, &m_window_width, &m_window_height);
}

void EventManager::update()
{
	if (is_camera_looking_enabled)
	{
		delta_x = static_cast<float>(
			mouse_pos_x - last_mouse_position_x
		);

		delta_y = -static_cast<float>(
			mouse_pos_y - last_mouse_position_y
		);
	}
	else
	{
		delta_x = 0.0f;
		delta_y = 0.0f;
	}

	last_mouse_position_x = mouse_pos_x;
	last_mouse_position_y = mouse_pos_y;

	glfwSetScrollCallback(m_window, on_scroll_callback);

	// Frame timing
	const double current_time = glfwGetTime();

	nb_frames++;

	frame_time = static_cast<float>(current_time - last_frame_time);

	if (current_time - last_frame_time_fps >= 1.0)
	{
		frames_per_second = nb_frames;
		nb_frames = 0;
		last_frame_time_fps += 1.0;
	}

	last_frame_time = current_time;
}

void EventManager::end_update()
{
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

float EventManager::get_frame_time()
{
	return frame_time;
}

float EventManager::get_frame_time_max()
{
	return frame_time_max;
}

void EventManager::set_frame_time_max(float ftm)
{
	frame_time_max = ftm;
}

int EventManager::get_frames_per_second()
{
	return frames_per_second;
}

bool EventManager::is_exit_requested()
{
	return glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(m_window);
}

GLFWwindow* EventManager::get_window()
{
	return m_window;
}

GLFWmonitor* EventManager::get_monitor()
{
	return m_monitor;
}

const GLFWvidmode* EventManager::get_vid_mode()
{
	return m_vid_mode;
}

double EventManager::get_mouse_motion_x()
{
	return delta_x;
}

double EventManager::get_mouse_motion_y()
{
	return delta_y;
}

double EventManager::get_camera_scroll_offset()
{
	// One frame event
	const double offset = camera_scroll_offset;
	camera_scroll_offset = 0.0;
	return offset;
}

bool EventManager::is_camera_looking()
{
	return is_camera_looking_enabled;
}

void EventManager::set_camera_looking(const bool enabled)
{
	if (is_camera_looking_enabled == enabled)
	{
		return;
	}

	is_camera_looking_enabled = enabled;
	if (enabled)
	{
		last_mouse_position_x = mouse_pos_x;
		last_mouse_position_y = mouse_pos_y;

		delta_x = 0.0f;
		delta_y = 0.0f;

		disable_mouse_cursor();
	}
	else
	{
		delta_x = 0.0f;
		delta_y = 0.0f;

		enable_mouse_cursor();
	}
}

void EventManager::set_windows_application_icon()
{
	GLFWimage images[1];
	images[0].pixels = stbi_load((string(BAK3D_ASSETS_DIR) + "/editor/bak3d_icon.png").c_str(), &images[0].width, &images[0].height, nullptr, 4);
	if (images[0].pixels)
	{
		glfwSetWindowIcon(m_window, 1, images);
        
		// 4. Free the image data immediately after setting it
		stbi_image_free(images[0].pixels);
		glfwPollEvents();
	}
	else
	{
		B3D_LOG_WARNING("Failed to load icon image 'bak3d_icon.png'. Must be in asset folder '/editor'");
	}
}

void EventManager::enable_mouse_cursor()
{
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void EventManager::disable_mouse_cursor()
{
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void EventManager::toggle_vsync(const bool vsync_enabled)
{
	glfwSwapInterval(vsync_enabled ? 1 : 0);
}

float EventManager::get_random_float(float min, float max)
{
	const auto value = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);  // NOLINT(concurrency-mt-unsafe)

	return min + value * (max - min);
}

void EventManager::on_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Forward to ImGui first so it can update MouseWheel
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

	if (is_scrolling_enabled)
	{
		camera_scroll_offset += yoffset;
	}
}

void EventManager::set_scrolling_enabled(const bool enabled)
{
	is_scrolling_enabled = enabled;
}

bool EventManager::is_key_down(const int key)
{
	return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool EventManager::is_key_moving_forward_down()
{
	return is_key_down(GLFW_KEY_W);
}

bool EventManager::is_key_moving_back_down()
{
	return is_key_down(GLFW_KEY_S);
}

bool EventManager::is_key_moving_right_down()
{
	return is_key_down(GLFW_KEY_D);
}

bool EventManager::is_key_moving_left_down()
{
	return is_key_down(GLFW_KEY_A);
}

bool EventManager::is_key_moving_up_down()
{
	return is_key_down(GLFW_KEY_Q);
}

bool EventManager::is_key_moving_down_down()
{
	return is_key_down(GLFW_KEY_E);
}

bool EventManager::is_delete_key_down()
{
	return is_key_down(GLFW_KEY_DELETE);
}
