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

#include <array>
#include <vector>
#include <variant>
#include <assimp/mesh.h>

#include <glad/glad.h>
#include <glm/glm.hpp>

/*
 * ============================ ENGINE DEFINITIONS ==========================
 * Engine declarations and definitions for different types, settings, constants and more.
 * ==========================================================================
 */

constexpr static int MAX_LOG_ENTRIES = 64;
constexpr static float POST_PROCESS_COLORING_SLIDER_CLAMP = 10.0f;

using GlobalSettingValueType = std::variant<
    bool,
    int,
    float,
    uint32_t,
    glm::vec3,
    glm::vec4
>;

enum class GlobalSettingOption : uint32_t
{
    Resources_ForceFail,
    GridRendering,
    AxisRendering,
    BackgroundColor,
    Light_Enabled,
    Light_Type,
    Light_HorizontalRotation,
    Light_VerticalRotation,
    Light_Scaling,
    Light_OriginDistance,
    Light_Color,
    Light_Intensity,
    Light_Point_Attenuation_Radius,
    Light_Spot_ConeAngle_Size,
    Light_Spot_ConeAngle_Inner_CutOff,
    Light_Spot_ConeAngle_Outer_CutOff,
    AA_MSAA_Enabled,
    AA_MSAA_Samples,
    PostProcessing_Enabled,
    PostProcess_ColorGrading_Invert,
    PostProcess_ColorGrading_Grayscale,
    PostProcess_ColorGrading_Brightness,
    PostProcess_ColorGrading_Contrast,
    PostProcess_ColorGrading_Hue,
    PostProcess_ColorGrading_Saturation,
    PostProcess_ColorGrading_Temperature,
    PostProcess_ColorGrading_VignetteIntensity,
    PostProcess_ColorGrading_VignetteColor,
    PostProcess_KernelEffect_SharpenIntensity,
    PostProcess_KernelEffect_SobelIntensity,
    PostProcess_KernelEffect_EmbossIntensity,
    PostProcess_KernelEffect_BoxBlurIntensity,
    PostProcess_KernelEffect_LaplacianIntensity,
    Max
};

enum class SceneObjectType : uint32_t
{
    Grid,
    Axis,
    Light,
    Model,
    ParticleSystem,
    Max
};

/*
 * ========================= RENDERING DEFINITIONS ==========================
 * Definitions for different rendering structures and constant primitive data.
 * ==========================================================================
 */
constexpr auto MAX_BONE_INFLUENCE = 4;

static constexpr GLsizei FLOAT_SIZE = sizeof(float);
static constexpr GLsizei VEC3_SIZE = sizeof(glm::vec3);
static constexpr GLsizei VEC4_SIZE = sizeof(glm::vec4);
static constexpr GLsizei UINT_SIZE = sizeof(GLuint);
static constexpr GLsizei INT_SIZE = sizeof(GLint);
static constexpr GLsizei MAT4_SIZE = sizeof(glm::mat4);

struct Vertex
{
    // position
    glm::vec3 position;
    // normal
    glm::vec3 normal;
    // texCoords
    glm::vec2 tex_coords;
    // tangent
    glm::vec3 tangent;
    // bitangent
    glm::vec3 bitangent;
    // color
    glm::vec3 color;
    // use diffuse texture or not
    bool useDiffuseTexture = true;
    // use specular texture or not
    bool useSpecularTexture = true;
    // use normal texture or not
    bool useNormalsTexture = true;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];

    bool operator<(const Vertex& other) const
    {
        // This is tedious but necessary for std::set
        if (position != other.position) return position.x < other.position.x; // Simplified
        if (normal != other.normal) return normal.x < other.normal.x;
        if (tex_coords != other.tex_coords) return tex_coords.x < other.tex_coords.x;
        if (tangent != other.tangent) return tangent.x < other.tangent.x;
        if (bitangent != other.bitangent) return bitangent.x < bitangent.x;
        if (color != other.color) return color.x < other.color.x;
        // No need for the rest
        return false;
    }

};

struct Edge
{
    GLuint v1_index;
    GLuint v2_index;

    Edge(unsigned int a, unsigned int b) : v1_index(a), v2_index(b)
    {
        if (v1_index > v2_index)
        {
            std::swap(v1_index, v2_index); // Ensure consistent ordering
        }
    }

    bool operator<(const Edge& other) const
    {
        return v1_index < other.v1_index || (v1_index == other.v1_index && v2_index < other.v2_index);
    }
};

struct Face
{
    std::vector<GLuint> indices;

    explicit Face(const aiFace& face)
    {
        for (unsigned int i = 0; i < face.mNumIndices; ++i)
        {
            indices.push_back(face.mIndices[i]);
        }
        // Sort to ensure {1, 2, 3} and {3, 1, 2} are treated as the same face
        std::ranges::sort(indices);
    }

    bool operator<(const Face& other) const
    {
        return indices < other.indices;
    }
};


// Unique cube vertices (8 total)
static const std::vector<glm::vec3> CUBE_VERTICES_SOLID =
{
    {-0.5f, -0.5f, -0.5f}, // 0
    { 0.5f, -0.5f, -0.5f}, // 1
    { 0.5f,  0.5f, -0.5f}, // 2
    {-0.5f,  0.5f, -0.5f}, // 3
    {-0.5f, -0.5f,  0.5f}, // 4
    { 0.5f, -0.5f,  0.5f}, // 5
    { 0.5f,  0.5f,  0.5f}, // 6
    {-0.5f,  0.5f,  0.5f}  // 7
};

// Indices for the Element Buffer Object (EBO) - 12 triangles (36 indices)
static const std::vector<GLuint> CUBE_INDICES_SOLID =
{
    0, 1, 2, 2, 3, 0,  // Front face
    1, 5, 6, 6, 2, 1,  // Right face
    5, 4, 7, 7, 6, 5,  // Back face
    4, 0, 3, 3, 7, 4,  // Left face
    3, 2, 6, 6, 7, 3,  // Top face
    4, 5, 1, 1, 0, 4   // Bottom face
};

// Vector of glm::vec3 for cube vertices
static const std::vector<glm::vec3> CUBE_VERTICES_EDGED =
{
    {-0.5f, -0.5f, -0.5f},  // 0
    { 0.5f, -0.5f, -0.5f},  // 1
    { 0.5f,  0.5f, -0.5f},  // 2
    {-0.5f,  0.5f, -0.5f},  // 3
    {-0.5f, -0.5f,  0.5f},  // 4
    { 0.5f, -0.5f,  0.5f},  // 5
    { 0.5f,  0.5f,  0.5f},  // 6
    {-0.5f,  0.5f,  0.5f}   // 7
};

// Vector of GLuint for cube indices (edges)
static const std::vector<GLuint> CUBE_INDICES_EDGED =
{
    0, 1,  1, 2,  2, 3,  3, 0,  // Bottom face edges
    4, 5,  5, 6,  6, 7,  7, 4,  // Top face edges
    0, 4,  1, 5,  2, 6,  3, 7   // Vertical edges
};

static const std::vector<glm::vec3> CUBE_VERTICES_WIREFRAME =
{
    {-0.5f, -0.5f, -0.5f},  // 0 bottom-left-back
    { 0.5f, -0.5f, -0.5f},  // 1 bottom-right-back
    { 0.5f,  0.5f, -0.5f},  // 2 top-right-back
    {-0.5f,  0.5f, -0.5f},  // 3 top-left-back
    {-0.5f, -0.5f,  0.5f},  // 4 bottom-left-front
    { 0.5f, -0.5f,  0.5f},  // 5 bottom-right-front
    { 0.5f,  0.5f,  0.5f},  // 6 top-right-front
    {-0.5f,  0.5f,  0.5f},  // 7 top-left-front
};

static const std::vector<GLuint> CUBE_INDICES_WIREFRAME =
{
    0, 1,  1, 2,  2, 3,  3, 0,  // back face
    4, 5,  5, 6,  6, 7,  7, 4,  // front face
    0, 4,  1, 5,  2, 6,  3, 7   // connecting edges
};

// Unique vertices for the line (only position)
static const std::vector<glm::vec3> LINE_VERTICES =
{
    {-1.0f, 0.0f, 0.0f}, // 0: Start point
    {1.0f, 0.0f, 0.0f}   // 1: End point
};

// Indices for the Element Buffer Object (EBO)
static const std::vector<GLuint> LINE_INDICES =
{
    0, 1  // Single line connecting two points
};

struct AxisVertex
{
    glm::vec3 position;
    glm::vec3 color;
};

static const std::vector<AxisVertex> AXIS_VERTICES =
{
    {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}, // X red line
    {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},

    {{0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}}, // Y green line
    {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},

    {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}}, // Z blue line
    {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}}
};

static const std::vector<GLuint> AXIS_INDICES =
{
    0, 1,  // X-axis (Red)
    2, 3,  // Y-axis (Green)
    4, 5   // Z-axis (Blue)
};

// Quad vertex data: vec4(position.xy, texCoord.xy)
static const std::vector<glm::vec4> QUAD_VERTICES =
{
    {0.0f, 0.0f, 0.0f, 0.0f},  // Bottom-left  (pos: 0,0, tex: 0,0)
    {1.0f, 0.0f, 1.0f, 0.0f},  // Bottom-right (pos: 1,0, tex: 1,0)
    {1.0f, 1.0f, 1.0f, 1.0f},  // Top-right    (pos: 1,1, tex: 1,1)
    {0.0f, 1.0f, 0.0f, 1.0f}   // Top-left     (pos: 0,1, tex: 0,1)
};

// Quad element indices (EBO)
static const std::vector<GLuint> QUAD_INDICES =
{
    0, 1, 2,  // First triangle (bottom-left, bottom-right, top-right)
    2, 3, 0   // Second triangle (top-right, top-left, bottom-left)
};

/*
 * ======================== POST PROCESS DEFINITIONS ========================
 * Definitions and constant for different post process rendering structures.
 * ==========================================================================
 */

enum class KernelEffectType : uint32_t
{
    Sharpen,
    Sobel,
    Emboss,
    BoxBlur,
    Laplacian,
    Max
};
constexpr std::array STANDARD_SHARPEN =
{
    -1.0f, -1.0f, -1.0f,
    -1.0f,  9.0f, -1.0f,
    -1.0f, -1.0f, -1.0f
};

constexpr std::array SOBEL_EDGE_DETECTION =
{
    -1.0f, -1.0f, -1.0f,
    -1.0f,  8.0f, -1.0f,
    -1.0f, -1.0f, -1.0f
};

constexpr std::array EMBOSS =
{
    -2.0f, -1.0f,  0.0f,
    -1.0f,  1.0f,  1.0f,
     0.0f,  1.0f,  2.0f
};

constexpr std::array BOX_BLUR =
{
    1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f,
    1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f,
    1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f
};

constexpr std::array LAPLACIAN_SHARPEN =
{
    0.0f,  1.0f,  0.0f,
    1.0f, -4.0f,  1.0f,
    0.0f,  1.0f,  0.0f
};
