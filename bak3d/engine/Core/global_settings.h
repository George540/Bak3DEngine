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

#include <map>
#include <variant>

#include "global_definitions.h"
#include "logger.h"

/*
 * 
 */
class GlobalSettings
{
    static std::map<GlobalSettingOption, GlobalSettingValueType> global_settings;
    static const char* to_string(GlobalSettingOption enum_setting);
public:
    static void initialize();
    static void shutdown();

    template<typename T>
    static T get_global_setting_value(GlobalSettingOption option)
    {
        auto it = global_settings.find(option);
        if (it == global_settings.end())
        {
            B3D_LOG_ERROR("Get - Unknown setting: %s.", to_string(option));
            return T{}; // default fallback for specific value type
        }

        if (const T* value = std::get_if<T>(&it->second))
        {
            return *value;
        }

        B3D_LOG_ERROR("Type mismatch for option %s.", to_string(option));
        return T{};
    }

    template<typename T>
    static void set_global_setting(GlobalSettingOption option, const GlobalSettingValueType& value)
    {
        const auto setting_it = global_settings.find(option);
        if (setting_it == global_settings.end())
        {
            B3D_LOG_ERROR("Unknown setting: %s.", to_string(option));
            return;
        }

        setting_it->second = value;
    }

    static void reset_to_defaults();
};
