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

#include "Objects/mesh.h"

/*
 * Struct that holds minimal vertex data for building a mesh object
 */
struct MeshGeometry
{
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
};

/*
 * Mesh factory utility class for generating primitives and subsequently more complex shapes
 */
class MeshFactory
{
public:
    static MeshGeometry build_cube_geometry();
    static MeshGeometry build_plane_geometry(int subdivisions = 1);
    static MeshGeometry build_sphere_geometry(int rings = 16, int segments = 32);
    static MeshGeometry build_cylinder_geometry(int segments = 16);
    static MeshGeometry build_torus_geometry(int major_segments = 16, int minor_segments = 8, float minor_radius = 0.25f);
};
