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
#include <string_view>
#include <unordered_map>
#include <vector>
#include <variant>
#include <assimp/mesh.h>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Asset/asset_definitions.h"

/*
 * ============================ ENGINE DEFINITIONS ==========================
 * Engine declarations and definitions for different types, settings, constants and more.
 * ==========================================================================
 */

constexpr static float EPSILON_CUSTOM = 0.00001f;

constexpr static std::string_view B3D_LOG_FILE = "B3D_Logs.log";
constexpr static int MAX_LOG_ENTRIES = 128;
constexpr static float POST_PROCESS_COLORING_SLIDER_CLAMP = 10.0f;

/*
 * Uniform Data struct for debug views and other page-related data.
 * Note: These are NOT post-processing data.
 */
struct PagesData
{
    glm::vec4 depth_settings = glm::vec4(0.1f, 1.0f, 0.0f, 0.0f);
    int debug_mode = 2;
    float _padding[3]; 
};
static constexpr GLsizei PAGES_DATA_SIZE = sizeof(PagesData);
// @TODO: Make a sizeof constexpr for every custom stuct that is used as a size

enum class DebugViewMode : int32_t
{
    Lit = 0,
    Unlit = 1,
    Depth = 2,
    ShadowMask = 3,
    AO = 4,
    Count
};

inline const char* to_string(const DebugViewMode debug_view_enum)
{
    switch (debug_view_enum)
    {
        case DebugViewMode::Lit: return "Lit";
        case DebugViewMode::Unlit: return "Unlit";
        case DebugViewMode::Depth: return "Depth";
        case DebugViewMode::ShadowMask: return "Shadows";
        case DebugViewMode::AO: return "AO";
        case DebugViewMode::Count: return "Count";
        default: return "unknown";
    }
}

enum class OverlaysFlags : uint32_t
{
    None = 0,
    WorldGrid = 1 << 0,
    LightIcons = 1 << 1,
    All = WorldGrid | LightIcons
};

// Bitwise AND operator
inline bool operator&(OverlaysFlags a, OverlaysFlags b)
{
    return (static_cast<int32_t>(a) & static_cast<int32_t>(b)) != 0;
}

// Bitwise XOR compound assignment operator
inline OverlaysFlags& operator^=(OverlaysFlags& a, OverlaysFlags b)
{
    a = static_cast<OverlaysFlags>(static_cast<int32_t>(a) ^ static_cast<int32_t>(b));
    return a;
}

// Bitwise OR operator
inline OverlaysFlags operator|(OverlaysFlags a, OverlaysFlags b)
{
    return static_cast<OverlaysFlags>(static_cast<int32_t>(a) | static_cast<int32_t>(b));
}


struct OverlayFlagsDefinition
{
    OverlaysFlags flag;
    const char* label;
};

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
    Vsync,
    DebugGeometry_Enabled,
    VisualMode,
    BackgroundColor,
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
    Camera,
    Debug,
    Light,
    Model,
    Mesh,
    ParticleSystem,
    AdvancedParticleSystem,
    Max
};

/*
 * ========================= RENDERING DEFINITIONS ==========================
 * Definitions for different rendering structures and constant primitive data.
 * ==========================================================================
 */
static constexpr GLuint WORK_GROUP_LOCAL_SIZE = 64;
static constexpr auto MAX_BONE_INFLUENCE = 4;
static constexpr GLint SWIZZLE_MASK[] = { GL_RED, GL_RED, GL_RED, GL_ONE };

static constexpr GLsizei FLOAT_SIZE = sizeof(float);
static constexpr GLsizei VEC2_SIZE = sizeof(glm::vec2);
static constexpr GLsizei VEC3_SIZE = sizeof(glm::vec3);
static constexpr GLsizei VEC4_SIZE = sizeof(glm::vec4);
static constexpr GLsizei UVEC4_SIZE = sizeof(glm::uvec4);
static constexpr GLsizei UINT_SIZE = sizeof(GLuint);
static constexpr GLsizei INT_SIZE = sizeof(GLint);
static constexpr GLsizei MAT4_SIZE = sizeof(glm::mat4);

struct alignas(16) LightGPUData
{
    glm::vec4 position;  // xyz = position,  w = inner_cut_off
    glm::vec4 direction; // xyz = direction, w = outer_cut_off
    glm::vec4 ambient;   // xyz = ambient,   w = radius
    glm::vec4 diffuse;   // xyz = diffuse,   w = intensity
    glm::vec4 specular;  // xyz = specular,  w = PADDING
    
    int32_t type;     // 4 bytes
    float padding[3]; // 12 bytes PADDING
};
static constexpr GLsizei LIGHT_GPU_DATA_SIZE = sizeof(LightGPUData);

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
static constexpr GLsizei VERTEX_SIZE = sizeof(Vertex);

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
static constexpr GLsizei EDGE_SIZE = sizeof(Edge);

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
static constexpr GLsizei FACE_SIZE = sizeof(Face);

enum class PrimitiveMeshType : uint32_t
{
    Cube,
    Plane,
    Sphere,
    Cylinder,
    Torus,
    Count
};

struct PrimitiveMeshInfo
{
    PrimitiveMeshType type;
    const char* name;
};

inline constexpr std::array PrimitiveMeshInfos =
{
    PrimitiveMeshInfo {.type = PrimitiveMeshType::Cube, .name = "Cube" },
    PrimitiveMeshInfo {.type = PrimitiveMeshType::Plane, .name = "Plane" },
    PrimitiveMeshInfo {.type = PrimitiveMeshType::Sphere, .name = "Sphere" },
    PrimitiveMeshInfo {.type = PrimitiveMeshType::Cylinder, .name = "Cylinder" },
    PrimitiveMeshInfo {.type = PrimitiveMeshType::Torus, .name = "Torus" }
};

using ShaderStageMap = std::unordered_map<GLenum, std::string>;

inline const char* get_shader_stage_name_file_extension(GLenum stage)
{
    switch (stage)
    {
    case GL_VERTEX_SHADER:          return ".vert";
    case GL_FRAGMENT_SHADER:        return ".frag";
    case GL_COMPUTE_SHADER:         return ".comp";
    case GL_GEOMETRY_SHADER:        return ".geom";
    case GL_TESS_CONTROL_SHADER:    return ".tesc";
    case GL_TESS_EVALUATION_SHADER: return ".tese";
    default:                        return ".glsl";
    }
}

/*
 * Additional indirection layer for referencing material, mesh and other asset data on different scene objects.
 */
using MaterialSlot = std::shared_ptr<MaterialRef>;
using MeshSlot = std::shared_ptr<MeshRef>;

// Helper to create a slot preloaded with an asseted slot
inline MaterialSlot make_material_slot(const MaterialRef& mat)
{
    return std::make_shared<MaterialRef>(mat);
}

inline MaterialSlot make_material_slot()
{
    return std::make_shared<MaterialRef>(nullptr);
}

inline MeshSlot make_mesh_slot(const MeshRef& mesh)
{
    return std::make_shared<MeshRef>(mesh);
}

inline MeshSlot make_mesh_slot()
{
    return std::make_shared<MeshRef>(nullptr);
}

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

struct InstanceGPUData
{
    glm::vec3 position;       // 12 bytes (x, y, z)
    float     scale;          //  4 bytes (uniform scale)
    glm::vec2 rotation_half;  //  4 bytes (packed Quaternion)
    uint32_t  color_packed;   //  4 bytes (RGBA8 packed into a single uint)
};
static constexpr GLsizei INSTANCE_GPU_DATA_SIZE = sizeof(InstanceGPUData);
static_assert(sizeof(InstanceGPUData) == 28, "InstanceGPUData must stay tightly packed for GPU upload");
// Total: 24 to 28 bytes depending on alignment/packing technique


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
