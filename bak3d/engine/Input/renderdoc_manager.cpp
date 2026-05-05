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

#include "renderdoc_manager.h"

#include <windows.h>

#include "renderdoc/renderdoc_app.h"

#include "Core/logger.h"

namespace
{
    RENDERDOC_Version rdc_version = eRENDERDOC_API_Version_1_4_0;
    RENDERDOC_API_1_4_0* rdc_api  = nullptr;
    bool pending_capture = false;
}

void RenderDocManager::initialize()
{
    // Try to find the DLL (will succeed if launched via RenderDoc)
    HMODULE mod = GetModuleHandleA("renderdoc.dll");

    if (!mod)
    {
        // Fallback: Manually load from default installation path
        mod = LoadLibraryA("C:\\Program Files\\RenderDoc\\renderdoc.dll");
    }

    if (mod)
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");

        if (RENDERDOC_GetAPI)
        {
            // SUCCESS CHECK: ret == 1 means the pointer was successfully populated
            const int ret = RENDERDOC_GetAPI(rdc_version, (void**)&rdc_api);
            
            if (ret == 1) 
            {
                B3D_LOG_INFO("RenderDoc API v1.4.0 initialized successfully.");
            }
            else
            {
                rdc_api = nullptr;
                B3D_LOG_WARNING("RenderDoc API failed to initialize. Error code: %d.", ret);
            }
        }
    }
    else
    {
        // This is normal when running the app for regular development
        B3D_LOG_INFO("RenderDoc not detected. In-app capture features disabled.");
    }
}

void RenderDocManager::begin_frame()
{
    if (pending_capture)
    {
        rdc_api->StartFrameCapture(nullptr, nullptr);
        B3D_LOG_INFO("Starting capture frame...");
    }
}

void RenderDocManager::end_frame()
{
    if (pending_capture)
    {
        rdc_api->EndFrameCapture(nullptr, nullptr);
        B3D_LOG_INFO("GPU frame captured.Launching RenderDoc UI...");
        launch_renderdoc_ui();
        pending_capture = false;
    }
}

void RenderDocManager::shutdown()
{
    delete rdc_api;
}

void RenderDocManager::trigger_capture()
{
    if (rdc_api != nullptr)
    {
        if (pending_capture)
        {
            B3D_LOG_WARNING("GPU frame is already being captured.");
            return;
        }
        pending_capture = true;
        B3D_LOG_INFO("Capture queued for next frame.");
    }
    else
    {
        B3D_LOG_WARNING("RenderDoc UI not initialized. Aborting capture.");
    }
}

void RenderDocManager::launch_renderdoc_ui()
{
    if (rdc_api != nullptr)
    {
        // Make RenderDoc UI visible if already running
        if (rdc_api->IsTargetControlConnected())
        {
            B3D_LOG_INFO("Bringing RenderDoc to foreground...");
            rdc_api->ShowReplayUI();
        }
        else
        {
            B3D_LOG_INFO("Launching RenderDoc...");
            // Return the PID of the replay UI if successful, 0 if not.
            if (rdc_api->LaunchReplayUI(1, "") == 0)
            {
                B3D_LOG_WARNING("Failed to launch RenderDoc due to miss-matching PID. Aborting launch.");
            }
        }
    }
    else
    {
        B3D_LOG_WARNING("Failed to launch RenderDoc due to missing API reference. Aborting launch.");
    }
}
