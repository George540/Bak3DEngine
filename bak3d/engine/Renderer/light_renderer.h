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

#include "Scene/Objects/camera.h"
#include "Scene/Objects/light.h"

/*
 * Lights handler but on the GPU/Rendering side.
 * There is no light scene object processing here.
 * It is purely for uploading data to the GPU.
 */
class LightRenderer
{
    static constexpr GLuint LIGHTS_SSBO_BINDING = 15;

public:
    static void initialize();
    static void shutdown();
    static void update_and_upload_data(const std::vector<Light*>& active_lights, const Frustum& frustum);

    static int get_visible_light_count() { return m_visible_light_count; }

private:
    static std::unique_ptr<ShaderStorageBuffer> m_lights_ssbo;
    static std::vector<LightGPUData> m_staging_lights;
    static int m_visible_light_count;
};
