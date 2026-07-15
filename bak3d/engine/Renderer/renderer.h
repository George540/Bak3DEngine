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

#include "buffer.h"

struct GLFWwindow;

/*
 * Static class for managing everything rendering related, whether triggered by code or by the user.
 */
class Renderer
{
private:
	static GLFWwindow* r_window;

	static MultisampleFrameBuffer* r_msaa_fbo;
	static FrameBuffer* r_fbo;
public:
	static void initialize();
	static void shutdown();

	static void begin_frame();
	static void draw_frame();
	static void end_frame();

	static GLFWwindow* get_window() { return r_window; }
	static MultisampleFrameBuffer* get_msaa_frame_buffer() { return r_msaa_fbo; }
	static FrameBuffer* get_frame_buffer() { return r_fbo; }

	static void on_framebuffer_size_callback(GLFWwindow* window, int new_width, int new_height);
private:
	static void initialize_buffers();
	static void query_gpu_limitations();
};