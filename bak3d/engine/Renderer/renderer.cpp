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

#include "Input/event_manager.h"

#include "imgui.h"
#include "stb_image.h"

#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>

#include "debug_scope.h"
#include "post_processor.h"
#include "renderer_pass.h"
#include "Scene/scene.h"

using namespace std;

// Renderer
GLFWwindow* Renderer::r_window = nullptr;
std::unique_ptr<MultisampleFrameBuffer> Renderer::r_msaa_fbo;
std::unique_ptr<FrameBuffer> Renderer::r_fbo;
std::unique_ptr<FrameBuffer> Renderer::r_dbo;
std::unique_ptr<UniformBuffer> Renderer::r_debug_view_ubo;

#ifdef _DEBUG
constexpr bool IS_OPENGL_DEBUG_VERBOSE = false;
#endif

namespace
{
	PagesData m_pages_data = PagesData();
}

void Renderer::initialize()
{
	r_window = EventManager::get_window();

	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		B3D_LOG_ERROR("Failed to initialize GLAD");
		exit(-1);
	}
	B3D_LOG_INFO("Initializing GLAD context...");

#ifdef _DEBUG
	if (IS_OPENGL_DEBUG_VERBOSE)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // callback fires on the same thread/call stack that caused it
		glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
								  GLsizei length, const GLchar* message, const void* userParam)
		{
			// Filter noisy notification-severity spam if desired
			if (severity == GL_DEBUG_SEVERITY_LOW || severity == GL_DEBUG_SEVERITY_MEDIUM)
			{
				B3D_LOG_WARNING("[GL WARNING] type=0x%x id=%u severity=0x%x: %s", type, id, severity, message);
			}
			else if (severity == GL_DEBUG_SEVERITY_HIGH)
			{
				B3D_LOG_ERROR("[GL ERROR] type=0x%x id=%u severity=0x%x: %s", type, id, severity, message);
			}
			else if (GL_DEBUG_SEVERITY_NOTIFICATION)
			{
				B3D_LOG_INFO("[GL LOG] type=0x%x id=%u severity=0x%x: %s", type, id, severity, message);
			}
		}, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif

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
	r_debug_view_ubo->bind();
	r_debug_view_ubo->bind_buffer_sub_data(&m_pages_data, PAGES_DATA_SIZE, 0);
	r_debug_view_ubo->unbind();

	if (GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::AA_MSAA_Enabled))
	{
		if (const int samples_setting = GlobalSettings::get_global_setting_value<int>(GlobalSettingOption::AA_MSAA_Samples);
			r_msaa_fbo->get_samples() != samples_setting)
		{
			r_msaa_fbo->set_samples(samples_setting);
		}
		r_msaa_fbo->bind();
	}
	else
	{
		r_fbo->bind();
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
	const bool show_depth_debug = GlobalSettings::get_global_setting_value<int>(GlobalSettingOption::ViewSelection) == 1;
	if (show_depth_debug)
	{
		r_dbo->bind();
		glClear(GL_DEPTH_BUFFER_BIT);
		RendererPasses::render_pass_base_geometry();
		r_dbo->unbind();
		RendererPasses::render_pass_debug_view();

		// Disable Post Processing when depth testing view is enabled
		if (GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::PostProcessing_Enabled))
		{
			GlobalSettings::set_global_setting<bool>(GlobalSettingOption::PostProcessing_Enabled, false);
		}
	}
	else
	{
		RendererPasses::render_pass_debug_geometry();
		RendererPasses::render_pass_base_geometry();
		RendererPasses::render_pass_lighting();
	}

	// Swap buffers
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	// Back to default framebuffer
	if (GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::AA_MSAA_Enabled))
	{
		DebugScopeGroup scope("MSAA Resolve (Color + Depth/Stencil)");
		r_msaa_fbo->resolve_to(*r_fbo);
		r_msaa_fbo->unbind();
	}
	else
	{
		r_fbo->unbind();
	}

	if (!show_depth_debug)
	{
		RendererPasses::render_pass_post_processing();
	}
}

void Renderer::end_frame()
{

}

void Renderer::shutdown()
{
	r_window = nullptr;
	r_dbo.reset();
	r_fbo.reset();
	r_msaa_fbo.reset();
}

PagesData Renderer::get_pages_data()
{
	return m_pages_data;
}

void Renderer::set_pages_data(const PagesData pages_data)
{
	m_pages_data = pages_data;
}

void Renderer::on_framebuffer_size_callback(GLFWwindow* window, const int new_width, const int new_height)
{
	if (new_width == 0 || new_height == 0)
	{
		return;
	}
	r_msaa_fbo->resize(new_width, new_height);
	r_fbo->resize(new_width, new_height);
	r_dbo->resize(new_width, new_height);
	PostProcessor::resize(new_width, new_height);
}

void Renderer::initialize_buffers()
{
	// Framebuffers
	r_msaa_fbo = std::make_unique<MultisampleFrameBuffer>(
		0, nullptr, EventManager::get_window_width(), EventManager::get_window_height(),
		4, GL_NONE, "Framebuffer_Base_MSAA");

	r_fbo = std::make_unique<FrameBuffer>(
		0, nullptr, EventManager::get_window_width(), EventManager::get_window_height(),
		GL_COLOR_ATTACHMENT0, GL_NONE, /*use_depth_texture=*/true, "Framebuffer_Base");

	r_dbo = std::make_unique<FrameBuffer>(
		0, nullptr, EventManager::get_window_width(), EventManager::get_window_height(),
		GL_DEPTH_ATTACHMENT, GL_NONE, /*use_depth_texture=*/true, "Depth_Buffer");

	// Uniform Buffers
	// @TODO: Replace with struct payload instead of manual size
	r_debug_view_ubo = make_unique<UniformBuffer>(PAGES_DATA_SIZE, nullptr, 2, GL_DYNAMIC_DRAW);
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
