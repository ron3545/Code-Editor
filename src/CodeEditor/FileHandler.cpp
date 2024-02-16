#include "FileHandler.h"

#include <fstream>
#include <sstream>
#include <cstdio>
#include <future>
#include <cstdio>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <string_view>

#include <mio/mmap.hpp>

namespace fs = std::filesystem;

void RecursivelyAddDirectoryNodes(DirectoryNode& parentNode, std::filesystem::directory_iterator directoryIterator)
{   
	for (const std::filesystem::directory_entry& entry : directoryIterator)
	{
		DirectoryNode& childNode = parentNode.Children.emplace_back();
		childNode.FullPath = entry.path().u8string();
		childNode.FileName = entry.path().filename().u8string();
		if (childNode.IsDirectory = entry.is_directory(); childNode.IsDirectory)
			RecursivelyAddDirectoryNodes(childNode, std::filesystem::directory_iterator(entry));
	}

	auto moveDirectoriesToFront = [](const DirectoryNode& a, const DirectoryNode& b) { return (a.IsDirectory > b.IsDirectory); };
	std::sort(parentNode.Children.begin(), parentNode.Children.end(), moveDirectoriesToFront);
}

DirectoryNode CreateDirectryNodeTreeFromPath(const std::filesystem::path& rootPath)
{   
    static std::mutex dir_tree;
    std::lock_guard<std::mutex> lock_dir_tree(dir_tree);
    
    DirectoryNode rootNode;
	rootNode.FullPath = rootPath.u8string();
	rootNode.FileName = rootPath.filename().u8string();

	if (rootNode.IsDirectory = fs::is_directory(rootPath); rootNode.IsDirectory)
        RecursivelyAddDirectoryNodes(rootNode, fs::directory_iterator(rootPath));
	return rootNode;
}

//========================================CLASS IMPL===============================================================
/**
 * true => success
 * false => failed | already exist 
*/
bool FileHandler::CreateNewFile(DirectoryNode& ParentNode, const std::filesystem::path& path, const char* file_name)
{
    std::filesystem::path new_file_path = path / file_name;
    if(fs::exists(new_file_path))
        return false;
    
    const std::string file = new_file_path.u8string();
    std::ofstream new_file(file);
    new_file.close();

    AddNode(ParentNode, path.u8string(), file_name, std::filesystem::is_directory(new_file_path));
    return true;
}

/**
 * true => success
 * false => failed | already exist 
*/
bool FileHandler::CreateNewFolder(DirectoryNode& ParentNode, const std::filesystem::path& path, const char* folder_name)
{
    const std::filesystem::path new_path(path / folder_name);
    if(!fs::exists(new_path) && fs::create_directories(new_path)){
        AddNode(ParentNode, path.u8string(), folder_name, std::filesystem::is_directory(new_path));
        return true;
    }
    return false;
}

bool FileHandler::DeleteSelectedFile(DirectoryNode& ParentNode, const std::filesystem::path& path)
{
    if(fs::remove(path))
    {
        Search_RemoveNode(ParentNode, path.u8string());
        return true;
    }
    return false;
}

bool FileHandler::DeleteSelectedFolder(DirectoryNode& ParentNode, const std::filesystem::path& path)
{
    if(fs::remove_all(path)){
        Search_RemoveNode(ParentNode, path.u8string());
        return true;
    }
    return false;
}

void FileHandler::Copy(DirectoryNode& ParentNode, const std::string& path)
{ 
    paste_mode = FileHandler_PasteMode::FileHandlerMode_Copy;
    ImGui::SetClipboardText(path.c_str());
}

void FileHandler::Cut(DirectoryNode& ParentNode, const std::string& path)
{
    paste_mode = FileHandler_PasteMode::FileHandlerMode_Cut;
    ImGui::SetClipboardText(path.c_str());
}

std::string GetFileName(const std::string& path)
{
    size_t lastSeparatorPos = path.find_last_of("\\/");
    if (lastSeparatorPos != std::string::npos) 
        // Extract the substring starting from the position after the separator
        return path.substr(lastSeparatorPos + 1);
    else
        return path;
}

void FileHandler::Paste(DirectoryNode& ParentNode, const std::filesystem::path& target_path, bool IsDirectory)
{   
    bool overwrite_file = false;
    fs::path src_file = ImGui::GetClipboardText(); 

    if(src_file.empty() || target_path.empty())
        return;

    const fs::path target = target_path / src_file.filename();

    if(fs::exists(target))
        ImGui::OpenPopup("File Exist");

    ImGui::SetNextWindowSize(ImVec2(300, 130));
    if(ImGui::BeginPopup("File Exist", ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
    {
        if(text_font != nullptr)
            ImGui::PushFont(text_font);
        
        ImGui::TextWrapped("File already exist. Do you want to overwrite?");
        ImGui::Dummy(ImVec2(0, 3));
        ImGui::Separator();
        ImGui::Dummy(ImVec2(0, 3));
        if(ImGui::Button("Yes", ImVec2(200, 27)) || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
        {
            overwrite_file = true;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine(); ImGui::Dummy(ImVec2(5,0)); ImGui::SameLine();
        if(ImGui::Button("No", ImVec2(200, 27)) || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
        {
            overwrite_file = false;
            ImGui::CloseCurrentPopup();
        }

        if(text_font != nullptr)
            ImGui::PopFont();
        ImGui::EndPopup();
    }

/** To Do: 
     * Fix the FileHandlerMode_Copy -> Should be able to distinguish directory and a file before copying
     * Fix the AddNode -> Should be able to copy the whole DirectoryNode including its child nodes
    */
    switch(paste_mode)
    {
        case FileHandler_PasteMode::FileHandlerMode_Copy: { 
            try
            {                
                if(IsDirectory)
                    fs::copy(src_file, target);
                else
                    fs::copy_file(  src_file, target, (overwrite_file)?  
                                    fs::copy_options::overwrite_existing : fs::copy_options::skip_existing);
                
                DirectoryNode new_node = CreateDirectryNodeTreeFromPath(target);
                Search_AddNode(ParentNode, target_path.u8string(), new_node);
            }
            catch(const std::exception& e)
            {
                std::filesystem::path current_path(std::filesystem::current_path() / "ArmSimPro_Log.txt");
                
            }
        }break;
            
        case FileHandler_PasteMode::FileHandlerMode_Cut: {
            try
            {
                // Cut File
                fs::rename(src_file.u8string().c_str(), target.u8string().c_str()); 
                Search_RemoveNode(ParentNode, src_file.u8string());

                DirectoryNode new_node = CreateDirectryNodeTreeFromPath(target);
                Search_AddNode(ParentNode, target_path.u8string(), new_node);
            }
            catch(const std::exception& e)
            {
                std::filesystem::path current_path(std::filesystem::current_path() / "ArmSimPro_Log.txt");
                
            }
        }break;
    }
}

void FileHandler::Paste(DirectoryNode& ParentNode, const std::filesystem::path& src_file, const std::filesystem::path& target_file)
{
    try
    {
        const fs::path target = target_file / src_file.filename();
        fs::rename(src_file.u8string().c_str(), target.u8string().c_str()); 
        Search_RemoveNode(ParentNode, src_file.u8string());

        DirectoryNode new_node = CreateDirectryNodeTreeFromPath(target);
        Search_AddNode(ParentNode, target_file.u8string(), new_node);
    }
    catch(const std::exception& e)
    {
        std::filesystem::path current_path(std::filesystem::current_path() / "ArmSimPro_Log.txt");
        
    }
}

void FileHandler::Rename(std::string& selected_path, const std::string& new_name)
{ 
    if(selected_path.empty())
        return;

    try
    {
        fs::path old_path(selected_path);
        auto parent_path = old_path.parent_path(); 
        auto new_path = parent_path / new_name;

        fs::rename(selected_path.c_str(), new_path.u8string().c_str());
        selected_path = new_path.u8string();
    }
    catch(const std::exception& e)
    {
        std::filesystem::path current_path(std::filesystem::current_path() / "ArmSimPro_Log.txt");
        
    }
}

bool FileHandler::Search_RemoveNode(DirectoryNode& ParentNode, const std::string& target_path)
{
    if(target_path.empty())
        return false;
    
    auto it = std::remove_if(ParentNode.Children.begin(), ParentNode.Children.end(),
                             [&](const DirectoryNode& child) { return child.FullPath == target_path; });
    ParentNode.Children.erase(it, ParentNode.Children.end());

    if(ParentNode.FullPath == target_path)
        return true;
    
    for(auto& child : ParentNode.Children)
        if(Search_RemoveNode(child, target_path))
            return true;
    return false;
}


std::vector<std::filesystem::path> FileHandler::GetFileList(const std::filesystem::path &project_path)
{
    static std::mutex getFile_mutex;
    std::lock_guard<std::mutex> lock(getFile_mutex);

    std::vector<std::filesystem::path> Files;
    for(std::filesystem::recursive_directory_iterator end, dir(project_path); dir != end; ++dir)
    {
        if(dir->path().filename() == "build" || dir->path().filename() == ".git")
            continue;

        if(!dir->is_directory())
            Files.push_back(dir->path());
    }
    return Files;
}

std::set<std::filesystem::path>  FileHandler::Search_String_On_Files(const std::filesystem::path &project_path, const std::string &key)
{   
    if(key.empty())
        return std::set<std::filesystem::path> ();

    static std::string prev_path;
    static std::vector<std::filesystem::path> Files; 

    std::set<std::filesystem::path>  result;
    if(project_path.u8string() != prev_path) //execute once
    {
        prev_path = project_path.u8string();
        auto file_list_future = std::async(std::launch::async, &FileHandler::GetFileList, this, project_path);
        Files = file_list_future.get();
    }

    if(Files.empty())
        return std::set<std::filesystem::path> ();

    //Launch tasks on seperate threads.
    std::vector<std::future<void>> futures;
    for(const auto& file : Files)
        futures.push_back(std::async(std::launch::async, &FileHandler::AddFiles, this, &result, file, key));

    return result;
}

void FileHandler::AddFiles(std::set<std::filesystem::path>* dest, const std::filesystem::path& path, const std::string& key)
{
    std::error_code error;
    mio::mmap_source ro_mmap;
    ro_mmap.map(path.u8string().c_str(), error);

    std::string data(ro_mmap.begin(), ro_mmap.end());
    
    auto it = std::search(data.begin(), data.end(),
                          std::boyer_moore_searcher(key.begin(), key.end()));
    
    std::lock_guard<std::mutex> lock(key_on_file_mutex);
    if(it != data.end())
        dest->insert(path);
}

std::filesystem::path FileHandler::Search_Needle_On_Haystack(const std::filesystem::path& path, const std::string& key)
{   
    std::lock_guard<std::mutex> lock(search_mutex);

    SearchedKeys contains_key;

    std::error_code error;
    mio::mmap_source ro_mmap;
    ro_mmap.map(path.u8string().c_str(), error);

    // typedef std::vector<char> Line;
    // typedef std::vector<Line> Lines;
    
    // Lines lines;

    // lines.emplace_back(Line());
    // for(auto& chr : ro_mmap){
    //     switch(chr)
    //     {
    //         case '\r': break;
    //         case '\n':  lines.emplace_back(Line());
    //         default: lines.back().emplace_back(chr);
    //     }
    // }

    // return std::make_tuple(path, contains_key);
    return path;
}

bool FileHandler::Search_AddNode(DirectoryNode& ParentNode, const std::string& target_path, const DirectoryNode& to_add)
{
    if(to_add.FullPath.empty() || target_path.empty())
        return false;   

    if(ParentNode.FullPath == target_path)
    {
        ParentNode.Children.push_back(to_add);

        auto moveDirectoriesToFront = [](const DirectoryNode& a, const DirectoryNode& b) { return (a.IsDirectory > b.IsDirectory); };
	    std::sort(ParentNode.Children.begin(), ParentNode.Children.end(), moveDirectoriesToFront);
        return true;
    }

    for(auto& ChildNode : ParentNode.Children)
        if(Search_AddNode(ChildNode, target_path, to_add))
            return true; //Target was found and successfully added new node
    return false; //Target was not found
}

void FileHandler::AddNode(DirectoryNode& ParentNode, const std::string& target_path, const std::string& to_add, bool IsDirectory)
{   
    DirectoryNode new_node;
    new_node.IsDirectory = IsDirectory;
    new_node.FileName = to_add;
    new_node.FullPath = target_path + "\\" + to_add;
    Search_AddNode(ParentNode, target_path, new_node);
}



