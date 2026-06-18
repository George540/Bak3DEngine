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
#include <string>
#include <unordered_map>
#include <list>
#include <filesystem>

enum FileType 
{
	png,
	jpg,
	vs,
	vert,
	fs,
	frag,
	comp,
	obj,
	mtl,
	fbx,
	json
};

/*
 * Static utility class for loading assets from files such as shaders, models, textures,
 * and generally folders and directories in order to get content to the engine.
 */
class FileLoader
{
public:
	static std::unordered_map<FileType, std::string> enum_to_string_map;

	static std::vector<std::string> get_directories(const std::filesystem::path& path);
	static std::vector<std::string> get_files_by_type(const std::filesystem::path& path, FileType type);
	static std::vector<std::pair<std::string, std::string>> get_files_by_type_with_path(const std::filesystem::path& path, FileType type);
	static std::vector<std::pair<std::string, std::string>> get_files_by_types_with_path(const std::filesystem::path& path, const std::vector<FileType>& types);
	static std::string get_filename_from_path(const std::filesystem::path& path);
	static std::string get_filename_from_path(const std::string& path);
	static std::string get_name_from_filename(const std::string& filename, bool to_lower_case = false);
	static std::string get_name_from_path(const std::string& path, const bool to_lower_case = false);
	static std::string enum_to_string(FileType type);
	static std::vector<char*> get_vector_items_to_array(const std::list<std::pair<std::string, std::string>>, bool is_returning_path = true);
	static std::string find_first_containing_string(const std::vector<std::string>& string_vector, const std::string& substring);
};