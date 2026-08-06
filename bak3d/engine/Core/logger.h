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
#include <cstdio>

#include "global_definitions.h"
#include "Asset/file_loader.h"

namespace log_helper
{
    constexpr const char* extract_filename(const char* path)
    {
        const char* last = path;
        for (const char* p = path; *p != '\0'; ++p)
        {
            if (*p == '/' || *p == '\\')
            {
                last = p + 1;
            }
        }
        return last;
    }
}

#define LOG_FILENAME (log_helper::extract_filename(__FILE__))

enum class LogLevel : uint8_t
{
    Log_Info = 1 << 0,
    Log_Warning = 1 << 1,
    Log_Error = 1 << 2,
    Log_Max = 1 << 3
};

inline const char* to_string(LogLevel e)
{
    switch (e)
    {
    case LogLevel::Log_Info: return "LOG";
    case LogLevel::Log_Warning: return "WARNING";
    case LogLevel::Log_Error: return "ERROR";
    case LogLevel::Log_Max: return "MAX";
    default: return "unknown";
    }
}

struct LogEntry
{
    std::string header_buffer;
    std::string log_buffer;
    LogLevel log_level;
};

/*
 * Simple Logger class for logging different levels of logging severity (Log, Warning, Error).
 */
class Logger
{
public:
    static void initialize();
    static void shutdown();
    template<typename... Args>
    static void print_log(const char* file, int line, LogLevel level, const char* fmt, Args... args)
    {
        const char* tag = "LOG";
        switch (level)
        {
            case LogLevel::Log_Info: tag = "LOG";  break;
            case LogLevel::Log_Warning: tag = "WARN"; break;
            case LogLevel::Log_Error: tag = "ERR";  break;
            case LogLevel::Log_Max: break;
        }

        char header_buffer[128];
        std::string elapsed_time_formatted = get_elapsed_time_formatted();
        int length = std::snprintf(header_buffer, sizeof(header_buffer), "[%s][%s] %s:%d - ", elapsed_time_formatted.c_str(), tag, file, line);

        char log_buffer[256];
        length = std::snprintf(log_buffer, sizeof(log_buffer), fmt, args...);

        LogEntry log_entry;
        log_entry.log_level = level;
        log_entry.header_buffer = header_buffer;
        log_entry.log_buffer = log_buffer;
        

        if (log_entries.size() >= MAX_LOG_ENTRIES)
        {
            log_entries.erase(log_entries.begin());
        }

        log_entries.push_back(log_entry);

        const std::string full_log = log_entry.header_buffer + log_entry.log_buffer + "\n";
        FileLoader::write_save_export_to_log_file_immediate(full_log);
    }

    static std::vector<LogEntry>& get_log_entries() { return log_entries; }
    static void clear_log_entries() { log_entries.clear(); }
private:
    static std::string get_elapsed_time_formatted();

    static std::chrono::steady_clock::time_point start_time;
    static std::vector<LogEntry> log_entries;
};

#define B3D_LOG_INFO(fmt, ...) \
    Logger::print_log(LOG_FILENAME, __LINE__, LogLevel::Log_Info, fmt, ##__VA_ARGS__)
 
#define B3D_LOG_WARNING(fmt, ...) \
    Logger::print_log(LOG_FILENAME, __LINE__, LogLevel::Log_Warning, fmt, ##__VA_ARGS__)
 
#define B3D_LOG_ERROR(fmt, ...) \
    Logger::print_log(LOG_FILENAME, __LINE__, LogLevel::Log_Error, fmt, ##__VA_ARGS__)
