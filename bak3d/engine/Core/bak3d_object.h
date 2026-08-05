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

#include <chrono>
#include <string>

#include <glad/glad.h>

/*
 * Base abstract object class for the entire engine.
 * Contains simple metadata to help identify an instance across the entire project.
 */
class Bak3DObject
{
public:
    Bak3DObject()
    {
        m_object_id = generate_unique_id();
    }
    Bak3DObject(const std::string& name) : Bak3DObject() { m_object_name = name; }
    virtual ~Bak3DObject() = 0;

    const std::string& get_object_name() const { return m_object_name; }
    void set_object_name(const std::string& name) { m_object_name = name; }

    GLuint get_object_id() const { return m_object_id; }
    void set_object_id(const GLuint id) { m_object_id = id; }

    GLuint get_object_size() const { return m_object_size; }
protected:
    std::string m_object_name;
    GLuint m_object_id = 0;
    GLuint m_object_size = 0;
private:
    static uint32_t generate_unique_id()
    {
        static std::atomic<uint32_t> s_counter{ 1 };
        uint32_t count = s_counter.fetch_add(1, std::memory_order_relaxed);

        // FNV-1a base mixed with counter via Knuth multiplicative hash.
        // Keeps IDs non-sequential and spread across the uint32 space.
        uint32_t hash = 2166136261u;
        hash ^= count;
        hash *= 16777619u;
        hash ^= (count >> 16);
        hash *= 2654435761u;
        return hash == 0 ? 1 : hash; // 0 reserved as null/invalid
    }
};

inline Bak3DObject::~Bak3DObject() {}