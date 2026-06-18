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

#include <iostream>

#include "file_loader.h"

#include "Core/logger.h"

using namespace std;

vector<string> FileLoader::get_directories(const filesystem::path& path)
{
	vector<string> directories;

	try
	{
		if (filesystem::exists(path) && filesystem::is_directory(path))
		{
			for (const auto& entry : filesystem::directory_iterator(path))
			{
				if (filesystem::is_directory(entry.path()))
				{
					directories.push_back(entry.path().generic_string());
				}
			}
		}
	}
	catch (const filesystem::filesystem_error& e)
	{
		B3D_LOG_ERROR("File not successfully read: %s", e.what());
	}

	return directories;
}

vector<string> FileLoader::get_files_by_type(const filesystem::path& path, FileType type)
{
	vector<string> files;

	try
	{
		if (filesystem::exists(path) && filesystem::is_directory(path))
		{
			for (const auto& entry : filesystem::recursive_directory_iterator(path))
			{
				if (filesystem::is_regular_file(entry.path()) && entry.path().extension() == enum_to_string(type))
				{
					auto filename = entry.path().filename().generic_string();
					files.push_back(filename);
				}
			}
		}
	}
	catch (const filesystem::filesystem_error& e)
	{
		B3D_LOG_ERROR("File not successfully read: %s", e.what());
	}

	return files;
}

vector<pair<string, string>> FileLoader::get_files_by_type_with_path(const filesystem::path& path, FileType type)
{
	vector<pair<string, string>> files_list;

	try
	{
		if (filesystem::exists(path) && filesystem::is_directory(path))
		{
			for (const auto& entry : filesystem::recursive_directory_iterator(path))
			{
				if (filesystem::is_regular_file(entry.path()) &&
					entry.path().extension() == enum_to_string(type))
				{
					const auto& file_path = entry.path();

					string filepath = file_path.generic_string();
					string filename = file_path.filename().generic_string();

					files_list.emplace_back(filename, filepath);
				}
			}
		}
	}
	catch (const filesystem::filesystem_error& e)
	{
		B3D_LOG_ERROR("File not successfully read: %s", e.what());
	}

	return files_list;
}

vector<pair<string, string>> FileLoader::get_files_by_types_with_path(const filesystem::path& path, const vector<FileType>& types)
{
	vector<pair<string, string>> files_list;

	try
	{
		if (filesystem::exists(path) && filesystem::is_directory(path))
		{
			for (const auto& entry : filesystem::recursive_directory_iterator(path))
			{
				if (filesystem::is_regular_file(entry.path()))
				{
					const auto& file_path = entry.path();
					auto file_ext = file_path.extension();

					// Check if extension matches any requested type
					if (ranges::any_of(types, [&](const FileType& type)
					{
						return file_ext == enum_to_string(type);
					}))
					{
						string filepath = file_path.generic_string();
						string filename = file_path.filename().generic_string();

						// ⚠️ Fix: extract name without extension correctly
						string assetname = filename.substr(0, filename.find_last_of('.'));

						files_list.emplace_back(assetname, filepath);
					}
				}
			}
		}
	}
	catch (const filesystem::filesystem_error& e)
	{
		B3D_LOG_ERROR("File not successfully read: %s", e.what());
	}

	return files_list;
}

string FileLoader::get_filename_from_path(const filesystem::path& path)
{
	return path.filename().generic_string();
}

string FileLoader::get_filename_from_path(const string& path)
{
	return filesystem::absolute(path).filename().generic_string();
}

string FileLoader::get_name_from_filename(const string& filename, const bool to_lower_case)
{
	if (filename.empty())
	{
		B3D_LOG_WARNING("Could not get name from file. No filename was given");
		return filename;
	}
	string name = filename.substr(0, filename.find_last_of('.'));
	if (to_lower_case)
	{
		transform(name.begin(), name.end(), name.begin(), ::tolower);
		name[0] = toupper(name[0]);
	}

	return name;
}

string FileLoader::get_name_from_path(const string& path, const bool to_lower_case)
{
	const auto file_name = get_filename_from_path(path);
	return get_name_from_filename(file_name, to_lower_case);
}

string FileLoader::enum_to_string(FileType type)
{
	switch (type)
	{
		case png: return ".png";
		case jpg: return ".jpg";
		case vs: return ".vert";
		case vert: return ".vert";
		case fs: return ".fs";
		case frag: return ".frag";
		case comp: return ".comp";
		case obj: return ".obj";
		case mtl: return ".mtl";
		case fbx: return ".fbx";
		case json: return ".json";
		default: return ".obj";
	}
}

vector<char*> FileLoader::get_vector_items_to_array(const list<pair<string, string>> list_items, bool is_returning_path) // <file_name, file_path>
{
	vector<string> vector_items;
	for (const auto& item : list_items)
	{
		if (is_returning_path)
		{
			vector_items.push_back(item.second);
		}
		else
		{
			vector_items.push_back(item.first);
		}
	}

	vector<char*> c_str_items;
	c_str_items.reserve(vector_items.size());

	for (const auto& item : vector_items)
	{
		auto c_str = new char[item.size() + 1]; // +1 for null terminator
		strcpy_s(c_str, item.size() + 1, item.c_str());
		c_str_items.push_back(c_str);
	}

	return c_str_items;
}

string FileLoader::find_first_containing_string(const vector<string>& string_vector, const string& substring)
{
	for (const string& string_entry : string_vector)
	{
		if (string_entry.find(substring) != string::npos)
		{
			return string_entry;
		}
	}

	return "None.jpg";
}
