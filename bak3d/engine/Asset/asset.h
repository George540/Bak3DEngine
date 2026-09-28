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

#include <string>

#include "Core/bak3d_object.h"

/*
 * Abstract class for setting up assets loaded from files such as shaders, textures, models, etc.
 * It contains general data about file naming and identification.
 */
class Asset : public Bak3DObject
{
public:
    Asset() = default;
    Asset(const std::string& path, const std::string& asset_name) : Bak3DObject(asset_name), m_path(path)
    {
        m_directory = m_path.substr(0, m_path.find_last_of('/'));
        m_file_name = path.substr(m_path.find_last_of('/') + 1);
        m_object_name = asset_name;
    }
    
    virtual ~Asset() = 0;

    std::string get_path() const { return m_path; }
    std::string get_directory() const { return m_directory; }
    std::string get_file_name() const { return m_file_name; }
    GLuint get_thumbnail_id() const { return m_thumbnail_id; }

    void set_path(const std::string& path) { m_path = path; }
    void set_directory(const std::string& directory) { m_directory = directory; }
    void set_file_name(const std::string& file_name) { m_file_name = file_name; }
    void set_thumbnail_id(const GLuint thumbnail_id) { m_thumbnail_id = thumbnail_id; }
protected:
    std::string m_path;
    std::string m_directory;
    std::string m_file_name;

    GLuint m_thumbnail_id = 0;
};

inline Asset::~Asset() = default;

/*
 * Ref structure for Asset objects. Used mainly for indirection and holding references to all types of assets.
 * Helps with swapping inner pointer without any holders needing to know when it changes.
 */
template<typename T>
struct AssetRef
{
    T* asset = nullptr;

    explicit AssetRef(T* a) : asset(a) {}

    T* operator->() const { return asset; }
    T& operator*()  const { return *asset; }

    bool is_valid() const { return asset != nullptr; }

    // Swaps the inner pointer. All shared_ptr holders see the new asset immediately.
    void swap(T* new_asset)
    {
        delete asset;
        asset = new_asset;
    }
};

/*
 * Additional indirection layer for shared ref and ref counted pointer style assets.
 * Mainly helps for cutting directly through the reference access without needing to get reference for every call.
 */
template<typename T>
class SharedAssetRef
{
public:
    SharedAssetRef() = default;
    SharedAssetRef(std::nullptr_t) {}

    explicit SharedAssetRef(AssetRef<T>* ref) : m_ref(ref) {}
    SharedAssetRef(std::shared_ptr<AssetRef<T>> ref) : m_ref(std::move(ref)) {}
    SharedAssetRef(const SharedAssetRef& other) : m_ref(other.m_ref) {} // Copy
    SharedAssetRef(SharedAssetRef&& other) noexcept : m_ref(std::move(other.m_ref)) {} // Move

    // Copy
    SharedAssetRef& operator=(const SharedAssetRef& other)
    {
        if (this != &other)
        {
            m_ref = other.m_ref;
        }
        return *this;
    }

    // Move
    SharedAssetRef& operator=(SharedAssetRef&& other) noexcept
    {
        if (this != &other)
        {
            m_ref = std::move(other.m_ref);
        }
        return *this;
    }

    SharedAssetRef& operator=(std::nullptr_t)
    {
        m_ref = nullptr;
        return *this;
    }

    // Cuts through both levels — ref->use() works directly
    T* operator->() const { return m_ref->asset; }
    T& operator*()  const { return *m_ref->asset; }
    explicit operator bool() const { return m_ref && m_ref->is_valid(); }

    bool is_valid() const { return m_ref && m_ref->is_valid(); }
    std::shared_ptr<AssetRef<T>> ref() const { return m_ref; } // Expose the inner ref

private:
    std::shared_ptr<AssetRef<T>> m_ref;
};

/*
 * Easy use for asset ref making.
 */
template<typename T>
SharedAssetRef<T> make_asset_ref(T* asset)
{
    return std::make_shared<AssetRef<T>>(asset);
}