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

// =====================================================================================
// Texture2D class was made by George Mavroeidis as a utility class for texture loading.
// It contains a set of data that are loaded in the constructor of the class and stored
// within the declared object. It can be used by all objects that load texture files
// =====================================================================================

#include <glad.h>
#include <iostream>

#include "stb_image.h"
#include "texture.h"

#include <cassert>
#include <unordered_map>

#include "Core/logger.h"

using namespace std;

Texture2D::Texture2D(const string& path, const string& file_name, const bool is_editor)
    : Asset(path, file_name)
    , m_is_editor(is_editor)
{
    // Bypass gen texture ID with unique ID
    glGenTextures(1, &m_texture_id);

    if (const auto data = stbi_load(m_path.c_str(), &m_width, &m_height, &m_nb_color_channels, 0))
    {
        GLenum format;
        switch (m_nb_color_channels)
        {
            case 1: format = GL_RED;  break;
            case 3: format = GL_RGB;  break;
            case 4: format = GL_RGBA; break;
            default: B3D_LOG_ERROR("Invalid texture format for texture %s with ID %d", file_name, m_texture_id); break;
        }

        glBindTexture(GL_TEXTURE_2D, m_texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, m_width, m_height, 0, format, GL_UNSIGNED_BYTE, data);  // NOLINT(bugprone-narrowing-conversions)
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);

        stbi_image_free(data);

        B3D_LOG_INFO("Texture %s loaded with texture ID: %d", m_object_name.c_str(), m_texture_id);
    }
    else
    {
        B3D_LOG_ERROR("Texture failed to load at path: %s", path.c_str());
        stbi_image_free(data);
    }
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &m_texture_id);
}

void Texture2D::bind(const int unit_slot) const
{
    assert(m_texture_id && "Binding an invalid texture");
    glActiveTexture(GL_TEXTURE0 + unit_slot);
    glBindTexture(GL_TEXTURE_2D, m_texture_id);
}

void Texture2D::unbind(const int unit_slot)
{
    glActiveTexture(GL_TEXTURE0 + unit_slot);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture2D::unbind_all(const int slot_count)
{
    for (int i = 0; i < slot_count; i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

bool Texture2D::operator==(const Texture2D& other) const
{
    return m_object_id == other.m_object_id
        && m_file_name == other.m_file_name;
}

bool Texture2D::operator!=(const Texture2D& other) const
{
    return m_object_id != other.m_object_id
        || m_file_name != other.m_file_name;
}