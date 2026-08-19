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

#include <unordered_map>

#include "editor_panel.h"
#include "Asset/asset.h"
#include "Asset/resource_map.h"
#include "Asset/texture.h"

struct AssetTreeNode
{
    std::string name;
    std::string full_path;
    std::unordered_map<std::string, std::unique_ptr<AssetTreeNode>> sub_folders;
    std::vector<std::pair<std::string, Asset*>> assets; // <resource map key, non-owning asset pointer>
};

/*
 * Panel view for all loaded assets stored in the Resource Manager
 */
class AssetPanel : public EditorPanel
{
public:
    AssetPanel();

    void begin_frame() override;
    void update() override;
    void end_frame() override;
private:
    void draw_asset_toolbar();
    void draw_folder_tree(AssetTreeNode& node);
    void draw_asset_grid(AssetTreeNode& node);
    void draw_folder_tile(AssetTreeNode* folder);
    void draw_asset_tile(const std::string& name, Asset* asset);

    void build_asset_tree();
    template<typename T>
    void insert_to_tree(const ResourceMap<T>& map);

    AssetTreeNode m_root;
    AssetTreeNode* m_selected_folder = nullptr;
    size_t m_last_asset_count = SIZE_MAX; // force first rebuild
};
