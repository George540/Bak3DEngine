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

#include <iostream>
#include <filesystem>

#include "renderer.h"

#include "Core/global_settings.h"
#include "Input/event_manager.h"

#include "imgui.h"
#include "stb_image.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "post_processor.h"
#include "renderer_pass.h"
#include "Scene/scene.h"

using namespace std;

// Renderer
GLFWwindow* Renderer::r_window = nullptr;
MultisampleFrameBuffer* Renderer::r_msaa_fbo = nullptr;
FrameBuffer* Renderer::r_fbo = nullptr;

void Renderer::initialize()
{
	r_window = EventManager::get_window();

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		B3D_LOG_ERROR("Failed to initialize GLAD");
		exit(-1);
	}
	B3D_LOG_INFO("Initializing GLAD context...");

	glfwSetFramebufferSizeCallback(r_window, on_framebuffer_size_callback);

	// Tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
	stbi_set_flip_vertically_on_load(true);

	// Somehow, glewInit triggers a glInvalidEnum... Let's ignore it
	glGetError();

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	B3D_LOG_INFO("Enabling depth test...");

	initialize_buffers();
	query_gpu_limitations();

	B3D_LOG_INFO("Ending Renderer Initialization....");
}

void Renderer::begin_frame()
{
	if (GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::AA_MSAA_Enabled))
	{
		if (const int samples_setting = GlobalSettings::get_global_setting_value<int>(GlobalSettingOption::AA_MSAA_Samples);
			r_msaa_fbo->get_samples() != samples_setting)
		{
			r_msaa_fbo->set_samples(samples_setting);
		}
		r_msaa_fbo->bind_object();
	}
	else
	{
		r_fbo->bind_object();
	}

	const auto background_color = GlobalSettings::get_global_setting_value<glm::vec4>(GlobalSettingOption::BackgroundColor);
	glClearColor(background_color.r, background_color.g, background_color.b, background_color.a);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::draw_frame()
{
	RendererPasses::render_pass_debug_geometry();
	RendererPasses::render_pass_base_geometry();
	RendererPasses::render_pass_lighting();

	// Swap buffers
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Back to default framebuffer
	if (GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::AA_MSAA_Enabled))
	{
		r_msaa_fbo->resolve_to(*r_fbo);
		r_msaa_fbo->unbind_object();
	}
	else
	{
		r_fbo->unbind_object();
	}

	RendererPasses::render_pass_post_processing();
}

void Renderer::end_frame()
{
	const auto background_color = GlobalSettings::get_global_setting_value<glm::vec4>(GlobalSettingOption::BackgroundColor);
	glClearColor(background_color.r, background_color.g, background_color.b, background_color.a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::shutdown()
{
	r_window = nullptr;
	r_fbo = nullptr;
	r_msaa_fbo = nullptr;
}

void Renderer::on_framebuffer_size_callback(GLFWwindow* window, const int new_width, const int new_height)
{
	if (new_width == 0 || new_height == 0)
	{
		return;
	}
	r_msaa_fbo->resize(new_width, new_height);
	r_fbo->resize(new_width, new_height);
	PostProcessor::resize(new_width, new_height);
}

void Renderer::initialize_buffers()
{
	r_msaa_fbo = new MultisampleFrameBuffer(0, nullptr, EventManager::get_window_width(), EventManager::get_window_height());
	r_fbo = new FrameBuffer(0, nullptr, EventManager::get_window_width(), EventManager::get_window_height());
}

void Renderer::query_gpu_limitations()
{
	int max_compute_work_group_count[3];
	int max_compute_work_group_size[3];
	int max_compute_work_group_invocations;

	for (int index = 0; index < 3; ++index)
	{
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, index, &max_compute_work_group_count[index]);
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, index, &max_compute_work_group_size[index]);
	}
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &max_compute_work_group_invocations);

	B3D_LOG_WARNING("----------OpenGL Limitations----------");

	B3D_LOG_WARNING("Maximum number of work groups in X dimension %d", max_compute_work_group_count[0]);
	B3D_LOG_WARNING("Maximum number of work groups in Y dimension %d", max_compute_work_group_count[1]);
	B3D_LOG_WARNING("Maximum number of work groups in Z dimension %d", max_compute_work_group_count[2]);

	B3D_LOG_WARNING("Maximum size of a work group in X dimension %d", max_compute_work_group_size[0]);
	B3D_LOG_WARNING("Maximum size of a work group in Y dimension %d", max_compute_work_group_size[1]);
	B3D_LOG_WARNING("Maximum size of a work group in Z dimension %d", max_compute_work_group_size[2]);

	B3D_LOG_WARNING("Number of invocations in a single local work group\n"
			     "that may be dispatched to a compute shader: %d", max_compute_work_group_invocations);

	B3D_LOG_WARNING("----------OpenGL Limitations----------");
}
