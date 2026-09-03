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

#include "mesh_factory.h"

#include <glm/gtc/constants.hpp>

#include "Asset/resource_manager.h"

using namespace std;

namespace
{
    // Adds one solid quad face as 4 unique vertices sharing one flat normal/tangent.
    // Corners must be passed in CCW order as seen from outside the cube to match backface culling order.
    void add_face(vector<Vertex>& verts, vector<GLuint>& indices,
                  const glm::vec3& v0, const glm::vec3& v1,
                  const glm::vec3& v2, const glm::vec3& v3,
                  const glm::vec3& normal)
    {
        const glm::vec3 tangent = glm::normalize(v1 - v0);
        const glm::vec3 bitangent = glm::normalize(v3 - v0);
        const glm::vec3 positions[4] = { v0, v1, v2, v3 };

        const GLuint base = static_cast<GLuint>(verts.size());
        for (int i = 0; i < 4; ++i)
        {
            constexpr glm::vec2 uvs[4] = { {0,0}, {1,0}, {1,1}, {0,1} };
            Vertex v {};
            v.position = positions[i];
            v.normal = normal;
            v.tex_coords = uvs[i];
            v.tangent = tangent;
            v.bitangent = bitangent;
            v.color = glm::vec3(1.0f);
            verts.push_back(v);
        }

        indices.insert(indices.end(), { base, base + 2, base + 1, base + 2, base, base + 3 });
    }
}

MeshGeometry MeshFactory::build_cube_geometry()
{
    vector<Vertex> vertices;
    vector<GLuint> indices;
    vertices.reserve(24);
    indices.reserve(36);

    constexpr float h = 0.5f;

    add_face(vertices, indices, {h,-h,-h}, {h,-h,h}, {h,h,h}, {h,h,-h}, {1,0,0});      // +X
    add_face(vertices, indices, {-h,-h,h}, {-h,-h,-h}, {-h,h,-h}, {-h,h,h}, {-1,0,0}); // -X
    add_face(vertices, indices, {-h,h,-h}, {h,h,-h}, {h,h,h}, {-h,h,h}, {0,1,0});      // +Y
    add_face(vertices, indices, {-h,-h,h}, {h,-h,h}, {h,-h,-h}, {-h,-h,-h}, {0,-1,0}); // -Y
    add_face(vertices, indices, {h,-h,h}, {-h,-h,h}, {-h,h,h}, {h,h,h}, {0,0,1});      // +Z
    add_face(vertices, indices, {-h,-h,-h}, {h,-h,-h}, {h,h,-h}, {-h,h,-h}, {0,0,-1}); // -Z

    assert(vertices.size() == 24 && indices.size() == 36 && "Cube primitive vertex/index count invariant broken");

    return {.vertices = move(vertices), .indices = move(indices) };
}

MeshGeometry MeshFactory::build_plane_geometry(int subdivisions)
{
    subdivisions = glm::max(subdivisions, 1);
    const int verts_per_side = subdivisions + 1;

    vector<Vertex> vertices;
    vector<GLuint> indices;
    vertices.reserve(verts_per_side * verts_per_side);

    for (int z = 0; z <= subdivisions; ++z)
    {
        const float v = static_cast<float>(z) / subdivisions;
        for (int x = 0; x <= subdivisions; ++x)
        {
            const float u = static_cast<float>(x) / subdivisions;

            Vertex vert{};
            vert.position = { u - 0.5f, 0.0f, v - 0.5f };
            vert.normal = { 0.0f, 1.0f, 0.0f };
            vert.tex_coords = { u, v };
            vert.tangent = { 1.0f, 0.0f, 0.0f };
            vert.bitangent = { 0.0f, 0.0f, 1.0f };
            vert.color  = glm::vec3(1.0f);
            vertices.push_back(vert);
        }
    }

    for (int z = 0; z < subdivisions; ++z)
    {
        for (int x = 0; x < subdivisions; ++x)
        {
            const GLuint i0 = z * verts_per_side + x;
            const GLuint i1 = i0 + 1;
            const GLuint i2 = i0 + verts_per_side;
            const GLuint i3 = i2 + 1;
            indices.insert(
            indices.end(),
            {
                    i0, i3, i1,
                    i0, i2, i3
                }
            );
        }
    }

    return {.vertices = move(vertices), .indices = move(indices) };
}

MeshGeometry MeshFactory::build_sphere_geometry(int rings, int segments)
{
    rings = glm::max(rings, 2);
    segments = glm::max(segments, 3);
    const int verts_per_ring = segments + 1;

    vector<Vertex> vertices;
    vector<GLuint> indices;
    vertices.reserve((rings + 1) * verts_per_ring);

    for (int ring = 0; ring <= rings; ++ring)
    {
        const float v = static_cast<float>(ring) / rings;
        const float phi = v * glm::pi<float>(); // 0 = top pole, pi = bottom pole
        const float sin_phi = sinf(phi);
        const float cos_phi = cosf(phi);

        for (int seg = 0; seg <= segments; ++seg)
        {
            const float u = static_cast<float>(seg) / segments;
            const float theta = u * glm::two_pi<float>();
            const float sin_theta = sinf(theta);
            const float cos_theta = cosf(theta);

            // Unit sphere: position and normal are the same vector, only differing by radius.
            const glm::vec3 normal(sin_phi * cos_theta, cos_phi, sin_phi * sin_theta);

            Vertex vert{};
            vert.position = normal * 0.5f;
            vert.normal = normal;
            vert.tex_coords = { u, 1.0f - v };
            vert.tangent = glm::normalize(glm::vec3(-sin_theta, 0.0f, cos_theta));
            vert.bitangent = glm::normalize(glm::cross(normal, vert.tangent));
            vert.color = glm::vec3(1.0f);
            vertices.push_back(vert);
        }
    }

    for (int ring = 0; ring < rings; ++ring)
    {
        for (int seg = 0; seg < segments; ++seg)
        {
            const GLuint i0 = ring * verts_per_ring + seg;
            const GLuint i1 = i0 + 1;
            const GLuint i2 = i0 + verts_per_ring;
            const GLuint i3 = i2 + 1;
            indices.insert(
            indices.end(),
            {
                    i0, i1, i2,
                    i1, i3, i2
                }
            );
        }
    }

    return {.vertices = move(vertices), .indices = move(indices) };
}

MeshGeometry MeshFactory::build_cylinder_geometry(int segments)
{
    segments = glm::max(segments, 3);
    constexpr float half_height = 0.5f;
    constexpr float radius = 0.5f;
    const int side_verts_per_ring = segments + 1;

    vector<Vertex> vertices;
    vector<GLuint> indices;

    for (int row = 0; row < 2; ++row) // 0 = bottom rim, 1 = top rim
    {
        const float y = row == 0 ? -half_height : half_height;
        for (int seg = 0; seg <= segments; ++seg)
        {
            const float u = static_cast<float>(seg) / segments;
            const float theta = u * glm::two_pi<float>();
            const float x = cosf(theta);
            const float z = sinf(theta);

            Vertex vert{};
            vert.position = { x * radius, y, z * radius };
            vert.normal = { x, 0.0f, z };
            vert.tex_coords = { u, static_cast<float>(row) };
            vert.tangent = glm::normalize(glm::vec3(-z, 0.0f, x));
            vert.bitangent = { 0.0f, 1.0f, 0.0f };
            vert.color = glm::vec3(1.0f);
            vertices.push_back(vert);
        }
    }
    for (int seg = 0; seg < segments; ++seg)
    {
        const GLuint i0 = seg;
        const GLuint i1 = i0 + 1;
        const GLuint i2 = i0 + side_verts_per_ring;
        const GLuint i3 = i2 + 1;
        indices.insert(
        indices.end(),
        {
                i0, i2, i1,
                i1, i2, i3
            }
        );
    }

    // Flat normals
    const auto add_cap = [&](float y, const glm::vec3& normal, bool flip_winding)
    {
        const GLuint center_index = static_cast<GLuint>(vertices.size());
        Vertex center{};
        center.position = { 0.0f, y, 0.0f };
        center.normal = normal;
        center.tex_coords = { 0.5f, 0.5f };
        center.tangent = { 1.0f, 0.0f, 0.0f };
        center.bitangent = { 0.0f, 0.0f, 1.0f };
        center.color = glm::vec3(1.0f);
        vertices.push_back(center);

        const GLuint first_rim = center_index + 1;
        for (int seg = 0; seg <= segments; ++seg)
        {
            const float u = static_cast<float>(seg) / segments;
            const float theta = u * glm::two_pi<float>();
            const float x = cosf(theta);
            const float z = sinf(theta);

            Vertex vert{};
            vert.position = { x * radius, y, z * radius };
            vert.normal = normal;
            vert.tex_coords = { x * 0.5f + 0.5f, z * 0.5f + 0.5f }; // planar cap projection
            vert.tangent = { 1.0f, 0.0f, 0.0f };
            vert.bitangent = { 0.0f, 0.0f, 1.0f };
            vert.color = glm::vec3(1.0f);
            vertices.push_back(vert);
        }

        for (int seg = 0; seg < segments; ++seg)
        {
            const GLuint rim0 = first_rim + seg;
            const GLuint rim1 = rim0 + 1;
            if (flip_winding)
            {
                indices.insert(indices.end(), { center_index, rim1, rim0 });
            }
            else
            {
                indices.insert(indices.end(), { center_index, rim0, rim1 });
            }
        }
    };

    add_cap(-half_height, { 0.0f, -1.0f, 0.0f }, false);
    add_cap( half_height, { 0.0f,  1.0f, 0.0f }, true);

    return {.vertices = move(vertices), .indices = move(indices) };
}

MeshGeometry MeshFactory::build_torus_geometry(int major_segments, int minor_segments, float minor_radius)
{
    major_segments = glm::max(major_segments, 3);
    minor_segments = glm::max(minor_segments, 3);
    const int verts_per_ring = minor_segments + 1;

    vector<Vertex> vertices;
    vector<GLuint> indices;
    vertices.reserve((major_segments + 1) * verts_per_ring);

    for (int major = 0; major <= major_segments; ++major)
    {
        constexpr float major_radius = 0.5f;
        const float u = static_cast<float>(major) / major_segments;
        const float theta = u * glm::two_pi<float>(); // angle around the main ring
        const float cos_theta = cosf(theta);
        const float sin_theta = sinf(theta);

        // Point on the tube's own centerline for this angle around the main ring
        const glm::vec3 ring_center(cos_theta * major_radius, 0.0f, sin_theta * major_radius);

        for (int minor = 0; minor <= minor_segments; ++minor)
        {
            const float v = static_cast<float>(minor) / minor_segments;
            const float phi = v * glm::two_pi<float>(); // angle around the tube's cross-section
            const float cos_phi = cosf(phi);
            const float sin_phi = sinf(phi);

            // Points outward from the tube's centerline, not the torus's origin
            const glm::vec3 normal(cos_theta * cos_phi, sin_phi, sin_theta * cos_phi);

            Vertex vert{};
            vert.position = ring_center + normal * minor_radius;
            vert.normal = normal;
            vert.tex_coords = { u, v };
            vert.tangent = glm::normalize(glm::vec3(-sin_theta, 0.0f, cos_theta));
            vert.bitangent = glm::normalize(glm::cross(normal, vert.tangent));
            vert.color = glm::vec3(1.0f);
            vertices.push_back(vert);
        }
    }

    for (int major = 0; major < major_segments; ++major)
    {
        for (int minor = 0; minor < minor_segments; ++minor)
        {
            const GLuint i0 = major * verts_per_ring + minor;
            const GLuint i1 = i0 + 1;
            const GLuint i2 = i0 + verts_per_ring;
            const GLuint i3 = i2 + 1;
            indices.insert(
         indices.end(),
            {
                    i0, i1, i2,
                    i1, i3, i2
                }
            );
        }
    }

    return {.vertices = move(vertices), .indices = move(indices) };
}