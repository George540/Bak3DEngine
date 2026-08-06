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

#include <fstream>
#include <iostream>

#include "file_loader.h"

#include "Core/logger.h"

using namespace std;

namespace
{
	unordered_map<string, string> file_content_storage;
}

void FileLoader::initialize()
{
	create_file(B3D_LOG_FILE.data());
	load_file_content(B3D_LOG_FILE.data());
}

void FileLoader::shutdown()
{
	
}

void FileLoader::create_file(const string& filename)
{
	ofstream file(filename, ios::trunc);
	if (!file)
	{
		B3D_LOG_WARNING("Could not open file '%s'", filename.c_str());
		return;
	}
	file.close();
	B3D_LOG_INFO("Created new file named '%s'", filename.c_str());
}

string FileLoader::load_file_content(const string& filename)
{
	ifstream file(filename);
	if (!file)
	{
		B3D_LOG_WARNING("Could not open file '%s'", filename.c_str());
		return "";
	}

	string file_content;
	string text_line;
	while (getline(file, text_line))
	{
		write_line_to_memory(filename, text_line);
	}
	file.close();
	B3D_LOG_INFO("Successfully loaded file %s and its content into memory", filename.c_str());

	if (!file_content_storage.contains(filename))
	{
		file_content_storage[filename] = file_content;
	}
	return file_content;
}

void FileLoader::write_line_to_memory(const string& filename, const string& text)
{
	if (!file_content_storage.contains(filename))
	{
		B3D_LOG_WARNING("Could not save file %s", filename.c_str());
		return;
	}
	file_content_storage[filename].append(text + "\n");
}

void FileLoader::save_file_to_memory(const string& filename)
{
	ofstream file(filename, ios::trunc);
	if (!file)
	{
		B3D_LOG_WARNING("Could not save file %s", filename.c_str());
		return;
	}
	file << file_content_storage.contains(filename) ? file_content_storage[filename] : "";
	file.close();
	B3D_LOG_INFO("Successfully saved file %s", filename.c_str());
}

void FileLoader::export_file_to_disk(const string& filename, const string& export_path)
{
	ofstream file(export_path + filename, ios::trunc);
	if (!file)
	{
		B3D_LOG_WARNING("Could not export file %s at path", filename.c_str(), export_path.c_str());
		return;
	}
	file << file_content_storage[filename];
	file.close();
	B3D_LOG_INFO("Successfully exported file %s to disk at %s", filename.c_str(), export_path.c_str());
}

void FileLoader::write_save_export_to_log_file_immediate(const std::string& text)
{
	// Early exit for when trying to write to the log file while it's being loaded
	if (!file_content_storage.contains(B3D_LOG_FILE.data()))
	{
		return;
	}
	ofstream file(B3D_LOG_FILE.data(), ios::app);
	if (!file)
	{
		B3D_LOG_WARNING("Could not write to log file %s", B3D_LOG_FILE.data());
	}
	file << text;
	file.close();
}

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

string FileLoader::enum_to_string(const FileType type)
{
	switch (type)
	{
		case FileType::png: return ".png";
		case FileType::jpg: return ".jpg";
		case FileType::vs: return ".vert";
		case FileType::vert: return ".vert";
		case FileType::fs: return ".fs";
		case FileType::frag: return ".frag";
		case FileType::comp: return ".comp";
		case FileType::obj: return ".obj";
		case FileType::mtl: return ".mtl";
		case FileType::fbx: return ".fbx";
		case FileType::json: return ".json";
		case FileType::txt: return ".txt";
		case FileType::log: return ".log";
		default: return ".txt";
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
