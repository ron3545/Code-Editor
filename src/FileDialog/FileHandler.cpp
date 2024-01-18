#include "FileHandler.h"
#include <fstream>
#include <cstdio>
#include "lwlog.h"

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
    if(fs::exists(path / file_name))
        return false;
    
    const std::string file = std::string(path.u8string() + file_name);
    std::ofstream new_file(file);
    new_file.close();

    return true;
}

/**
 * true => success
 * false => failed | already exist 
*/
bool FileHandler::CreateNewFolder(DirectoryNode& ParentNode, const std::filesystem::path& path, const char* folder_name)
{
    const std::filesystem::path new_path(path / folder_name);
    if(!fs::exists(new_path) && fs::create_directories(new_path))
        return true;
    return false;
}

bool FileHandler::DeleteSelectedFile(DirectoryNode& ParentNode, const std::filesystem::path& path)
{
    return fs::remove(path);
}

bool FileHandler::DeleteSelectedFolder(DirectoryNode& ParentNode, const std::filesystem::path& path)
{
    if(fs::remove_all(path)){
        RemoveNode(ParentNode, path.u8string());
        return true;
    }
    return false;
}

void FileHandler::CopyFile_Folder(DirectoryNode& ParentNode, const std::string& path)
{ 
    paste_mode = FileHandler_PasteMode::FileHandlerMode_Copy;
    ImGui::SetClipboardText(path.c_str());
}

void FileHandler::CutFile_Folder(DirectoryNode& ParentNode, const std::string& path)
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

void FileHandler::PasteFile(DirectoryNode& ParentNode, const std::filesystem::path& target_path)
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

    //To Do: Fix copy and copy_file
    switch(paste_mode)
    {
        case FileHandler_PasteMode::FileHandlerMode_Copy: { 
            try
            {
                const bool IsDirectory = std::filesystem::is_directory(target);
                const bool success = fs::copy_file(  src_file, target, 
                                        (overwrite_file)?  
                                        fs::copy_options::overwrite_existing : fs::copy_options::skip_existing);
                
                AddNode(ParentNode, target_path.u8string(), src_file.filename().u8string(), IsDirectory);
            }
            catch(const std::exception& e)
            {
                std::filesystem::path current_path(std::filesystem::current_path() / "ArmSimPro_Log.txt");
                auto log_file = std::make_shared<lwlog::file_logger>("FILE", current_path.u8string());
                log_file->error(e.what());
            }
        }break;
            
        case FileHandler_PasteMode::FileHandlerMode_Cut: {
            try
            {
                // Cut File
                fs::rename(src_file.u8string().c_str(), target.u8string().c_str()); 
                RemoveNode(ParentNode, src_file.u8string());
                AddNode(ParentNode, target_path.u8string(), src_file.filename().u8string(), std::filesystem::is_directory(target));
            }
            catch(const std::exception& e)
            {
                std::filesystem::path current_path(std::filesystem::current_path() / "ArmSimPro_Log.txt");
                auto log_file = std::make_shared<lwlog::file_logger>("FILE", current_path.u8string());
                log_file->error(e.what());
            }
        }break;
    }
}

void FileHandler::Rename(std::string& selected_path, const std::string& new_name)
{ 
    if(selected_path.empty())
        return;

    fs::path old_path(selected_path);
    auto parent_path = old_path.parent_path(); 
    auto new_path = parent_path / new_name;

    fs::rename(selected_path.c_str(), new_path.u8string().c_str());
    selected_path = new_path.u8string();
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

void FileHandler::RemoveNode(DirectoryNode& ParentNode, const std::string& path_to_remove)
{
    Search_RemoveNode(ParentNode, path_to_remove);
}