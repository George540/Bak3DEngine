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

#pragma once

#include <assimp/material.h>

#include "asset.h"

/*
 * Class that stores texture data in memory loaded from an image asset on disk.
 * Used to bind textures in the GPU when called, integrates with assimp structure as well.
 * @todo: Consider bindless rendering for textures
 */
class Texture2D : public Asset
{
public:
    Texture2D() = default;
    Texture2D(const std::string& path, const std::string& file_name, bool is_editor = false);
    Texture2D(const Texture2D&) = delete;
    ~Texture2D() override;

    void bind(int unit_slot) const; // binds the texture as the current active GL_TEXTURE_2D texture object
    static void unbind(int unit_slot = 0);
    static void unbind_all(int slot_count);

    GLuint get_texture_id() const { return m_texture_id; }
    int get_width() const { return m_width; }
    int get_height() const { return m_height; }
    int get_nb_color_channels() const { return m_nb_color_channels; }
    bool is_editor_texture() const { return m_is_editor; }

    Texture2D& operator=(const Texture2D&) = delete;
    bool operator==(const Texture2D& other) const;
    bool operator!=(const Texture2D& other) const;
private:
    GLuint m_texture_id;
    int m_width, m_height; // texture image dimensions: width and height of loaded image in pixels
    int m_nb_color_channels; // the number of color components in the loaded image
    bool m_is_editor;
};