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

#include "Buffers/data_buffer.h"
#include "Buffers/frame_buffer.h"
#include "Core/global_settings.h"
#include "Scene/Objects/quad.h"

struct GLFWwindow;

/*
 * Static class for managing everything rendering related, whether triggered by code or by the user.
 */
class Renderer
{
private:
	static GLFWwindow* r_window;

	static std::unique_ptr<MultisampleFrameBuffer> r_msaa_fbo;
	static std::unique_ptr<FrameBuffer> r_main_fbo;
	static std::unique_ptr<FrameBuffer> r_dbo;
	static std::unique_ptr<WBOITFrameBuffer> r_wboit_fbo;

	static std::unique_ptr<UniformBuffer> r_debug_view_ubo;
public:
	static void initialize();
	static void shutdown();

	static void begin_frame();
	static void draw_frame();
	static void end_frame();

	static void initialize_screen_quad();
	static void draw_quad();

	static GLFWwindow* get_window() { return r_window; }
	static MultisampleFrameBuffer* get_msaa_frame_buffer() { return r_msaa_fbo.get(); }
	static FrameBuffer* get_main_frame_buffer() { return r_main_fbo.get(); }
	static FrameBuffer* get_debug_view_buffer() { return r_dbo.get(); }
	static WBOITFrameBuffer* get_wboit_frame_buffer() { return r_wboit_fbo.get(); }

	static PagesData get_pages_data();
	static void set_pages_data(PagesData pages_data);

	static void on_framebuffer_size_callback(GLFWwindow* window, int new_width, int new_height);
private:
	static void initialize_buffers();
	static void query_gpu_limitations();
};