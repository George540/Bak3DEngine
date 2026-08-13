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

#include <vector>
#include <glad/glad.h>

#include "buffer.h"

/*
 * Framebuffer class that stores generated output textures with rendering buffer objects.
 * Default framebuffer for OpenGL has ID 0, which is rendering a texture in the full viewport window.
 */
class FrameBuffer : public Buffer
{
public:
    FrameBuffer(GLsizeiptr size,
        const void* data,
        const GLuint width,
        const GLuint height,
        const GLenum usage = GL_NONE,
        const bool use_depth_texture = false,
        const char* debug_name = nullptr);
    // Share externally-owned depth texture instead of creating one.
    // Subclasses are expected to override create_attachments(), get_draw_buffers()
    FrameBuffer(GLuint width, GLuint height, GLuint shared_depth_texture, const char* debug_name = nullptr);

    ~FrameBuffer() override;

    void bind() const override;
    void unbind() const override;

    void resize(GLuint new_width, GLuint new_height, GLuint new_shared_depth_texture = 0);
    void resolve_to(const FrameBuffer* fbo_target) const;
    void clear() const;

    void bind_color_attachment(GLuint index = 0) const;

    GLuint get_width() const { return m_width; }
    GLuint get_height() const { return m_height; }
    float get_aspect_ratio() const;
    GLuint get_color_texture(GLuint index = 0) const;
    GLuint get_buffer() const { return m_rbo; }
    GLuint get_depth_texture() const { return m_depth_texture; }
    bool is_using_depth_texture() const { return m_use_depth_texture && m_depth_texture > 0; }

protected:
    virtual std::vector<GLenum> get_draw_buffers() const;
    virtual std::string get_color_attachment_label(size_t index) const;
    virtual void create_attachments();

    GLuint create_texture_2d_attachment(
        GLenum attachment_slot,
        GLenum internal_format,
        GLenum format,
       GLenum type,
       GLenum min_filter = GL_LINEAR,
       GLenum mag_filter = GL_LINEAR,
       bool track_as_output = true);
    void attach_shared_depth_texture() const;

    void create_framebuffer();
    void destroy_framebuffer();
    void label_resources() const;

    GLuint m_width;
    GLuint m_height;
    std::vector<GLuint> m_color_textures;
    GLuint m_rbo;
    GLuint m_depth_texture = -1;
    bool m_use_depth_texture = false;
    bool m_owns_depth_texture = true;
    std::string m_debug_name;
};

/*
 * A FrameBuffer variant that stores sample sizes for Multisampled Anti-Aliasing
 */
class MultisampleFrameBuffer : public FrameBuffer
{
public:
    MultisampleFrameBuffer(
        GLuint width,
        GLuint height,
        GLsizei samples = 4,
        const char* debug_name = nullptr);

    void set_samples(const GLsizei new_samples);
    GLsizei get_samples() const { return m_samples; }
    GLsizei get_max_samples () const { return m_max_samples; }
protected:
    void create_attachments() override;

private:
    GLsizei m_samples;
    GLsizei m_max_samples;
};

class WBOITFrameBuffer : public FrameBuffer
{
public:
    WBOITFrameBuffer(const GLuint width, const GLuint height, const GLuint shared_depth_texture, const char* debug_name = nullptr);

    GLuint get_accum_texture() const { return get_color_texture(0); }
    GLuint get_revealage_texture() const { return get_color_texture(1); }
protected:
    void create_attachments() override;
    std::vector<GLenum> get_draw_buffers() const override;
    std::string get_color_attachment_label(size_t index) const override;
};
