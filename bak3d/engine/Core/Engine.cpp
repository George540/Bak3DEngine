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

#include "engine.h"

#include "editor.h"
#include "global_settings.h"
#include "logger.h"

#include "Asset/resource_manager.h"
#include "Input/event_manager.h"
#include "Input/renderdoc_manager.h"
#include "Renderer/post_processor.h"
#include "Renderer/renderer.h"
#include "Scene/scene_manager.h"

void Bak3DEngine::Initialize()
{
    RenderDocManager::initialize();
    Logger::initialize();
    GlobalSettings::initialize();
    EventManager::initialize();
    Renderer::initialize();
    ResourceManager::initialize();
    PostProcessor::initialize();
    Bak3DEditor::initialize();
    SceneManager::initialize();

    B3D_LOG_INFO("Engine initialized.");
}

void Bak3DEngine::Update()
{
    do
    {
        EventManager::begin_update();
        
        EventManager::update(); // Update Event Manager - Frame time / input / events processing 
        SceneManager::update();

        RenderDocManager::begin_frame();
        Renderer::begin_frame();
        Renderer::draw_frame(); // Update Renderer - Drawing / Passes / Buffer Objects
        Renderer::end_frame();
        RenderDocManager::end_frame();

        Bak3DEditor::update(); // Update Editor - Panels / Widgets / ImGui context

        EventManager::end_update();
    } while (EventManager::is_exit_requested() == false);
}

void Bak3DEngine::Shutdown()
{
    SceneManager::shutdown();
    Renderer::shutdown();
    Bak3DEditor::shutdown();
    PostProcessor::shutdown();
    ResourceManager::shutdown();
    Renderer::shutdown();
    EventManager::shutdown();
    GlobalSettings::shutdown();
    Logger::shutdown();
    RenderDocManager::shutdown();
}
