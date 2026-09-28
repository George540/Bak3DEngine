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

#include <functional>
#include <unordered_map>

#include "asset.h"
#include "Core/global_settings.h"
#include "Core/logger.h"

/*
 * Generic class that stores asset type instances into a map.
 * Helps with safely adding, swapping and deleting entries.
 * If one fails, it keeps the older version.
 */
template<typename T>
class ResourceMap
{
public:
    void insert_or_swap(const std::string& name, T* new_asset,
                        const std::function<bool(T*)>& is_valid)
    {
        if (!is_valid(new_asset) && !GlobalSettings::get_global_setting_value<bool>(GlobalSettingOption::Resources_ForceFail))
        {
            B3D_LOG_WARNING("Asset '%s' failed validation, keeping old version.",
                          name.c_str());
            delete new_asset;
            return;
        }

        if (m_map.contains(name))
        {
            // Swap inner pointer — all SharedAssetRef holders see the new asset
            m_map[name].ref()->swap(new_asset);
            B3D_LOG_INFO("Hot-swapped asset: %s", name.c_str());
        }
        else
        {
            m_map[name] = make_asset_ref<T>(new_asset);
        }
    }

    SharedAssetRef<T> get(const std::string& name) const
    {
        const auto it = m_map.find(name);
        if (it == m_map.end())
        {
            B3D_LOG_ERROR("Asset not found: %s", name.c_str());
            return nullptr;
        }
        return it->second;
    }

    std::string find_name(const T* asset) const
    {
        for (const auto& [name, ref] : m_map)
        {
            if (ref.ref()->asset == asset)
            {
                return name;
            }
        }
        return {};
    }

    bool contains(const std::string& name) const
    {
        return m_map.contains(name);
    }

    void remove(const std::string& name)
    {
        const auto it = m_map.find(name);
        if (it != m_map.end())
        {
            delete it->second->asset;
            it->second->asset = nullptr;
            m_map.erase(it);
        }
    }

    void clear()
    {
        for (auto& [name, shared_ref] : m_map)
        {
            delete shared_ref.ref()->asset;
            shared_ref.ref()->asset = nullptr;
        }
        m_map.clear();
    }

    // Iterate over all assets (e.g. for reload_all)
    const std::unordered_map<std::string, SharedAssetRef<T>>& all() const
    {
        return m_map;
    }

    size_t size() const { return m_map.size(); }

    bool empty() const { return m_map.empty(); }

private:
    std::unordered_map<std::string, SharedAssetRef<T>> m_map;
};
