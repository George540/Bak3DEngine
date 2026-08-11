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

class RendererPasses
{
public:
    static void render_pass_debug_geometry();   // axis / grid gizmos (skipped in debug view)
    static void render_pass_opaque_geometry();  // depth-writing scene geometry (model)
    static void render_pass_sprites();          // basic alpha-blended sprite ParticleSystem (skipped in debug view)
    static void render_pass_transparency();     // WBOIT accumulate + composite (skipped in debug view)
    static void render_pass_post_processing();  // skipped in debug view
    static void render_pass_editor_overlays();  // light gizmo sprite, always drawn last
    static void render_pass_debug_view();       // depth (and future AO/etc) visualization
};
