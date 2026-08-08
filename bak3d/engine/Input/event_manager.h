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

#pragma once

#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

struct GLFWwindow;
struct GLFWmonitor;
struct GLFWvidmode;

/*
 * @todo TO BE DEPRECATED AND REMADE:
 * Static class for managing user input bindings and window properties
 */
class EventManager
{
	// Window
	static GLFWwindow* m_window;
	static GLFWmonitor* m_monitor;
	static const GLFWvidmode* m_vid_mode;
	static int m_window_width;
	static int m_window_height;
	static int m_viewport_width;
	static int m_viewport_height;

	// Time
	static double last_frame_time;
	static double last_frame_time_fps;
	static float frame_time;
	static float frame_time_max;
	static int nb_frames;
	static int frames_per_second;

	// Mouse
	static double mouse_pos_x;
	static double mouse_pos_y;
	static double last_mouse_position_x;
	static double last_mouse_position_y;
	static double delta_x;
	static double delta_y;
	static double cam_zoom_distance;
	static double cam_zoom_factor;
	static int last_mouse_left_state;
	static int last_mouse_right_state;
	static int last_mouse_middle_state;
public:
	static void initialize();
	static void shutdown();

	static void begin_update();
	static void update();
	static void end_update();

	static float get_frame_time();
	static float get_frame_time_max();
	static void set_frame_time_max(float ftm);
	static int get_frames_per_second();
	static bool is_exit_requested();

	static GLFWwindow* get_window();
	static GLFWmonitor* get_monitor();
	const static GLFWvidmode* get_vid_mode();
	static int get_window_width() { return m_window_width; }
	static int get_window_height() { return m_window_height; }
	static float get_window_aspect_ratio() { return static_cast<float>(m_window_width) / static_cast<float>(m_window_height); }

	static int get_viewport_width() { return m_viewport_width; }
	static void set_viewport_width(const int viewport_width) { m_viewport_width = viewport_width; }
	static int get_viewport_height() { return m_viewport_height; }
	static void set_viewport_height(const int viewport_height) { m_viewport_height = viewport_height; }
	static float get_viewport_aspect_ratio() { return static_cast<float>(m_viewport_width) / static_cast<float>(m_viewport_height); }

	static double get_mouse_motion_x();
	static double get_mouse_motion_y();
	static double get_camera_scroll_offset();

	static void set_windows_application_icon();

	static void enable_mouse_cursor();
	static void disable_mouse_cursor();
	static void toggle_vsync(bool vsync_enabled);
	static float get_random_float(float min, float max);

	static void on_scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

	static bool is_dragging_enabled;
	static bool is_scrolling_enabled;

	static bool is_using_diffuse_texture;
	static bool is_using_specular_texture;
	static bool is_using_normals_texture;
};

#endif