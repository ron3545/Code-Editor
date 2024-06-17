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

namespace fs = std::filesystem;

void RecursivelyAddDirectoryNodes(DirectoryNode& parentNode, std::filesystem::directory_iterator directoryIterator)
{   
	for (const std::filesystem::directory_entry& entry : directoryIterator)
	{
		DirectoryNode& childNode = parentNode.Children.emplace_back();
		childNode.FullPath = entry.path().u8string();
		childNode.FileName = entry.path().filename().u8string();
        childNode.IsDirectory = std::filesystem::is_directory(entry.path()); 
		if (childNode.IsDirectory)
			RecursivelyAddDirectoryNodes(childNode, std::filesystem::directory_iterator(entry));
	}

	auto moveDirectoriesToFront = [](const DirectoryNode& a, const DirectoryNode& b) { return (a.IsDirectory > b.IsDirectory); };
	std::sort(parentNode.Children.begin(), parentNode.Children.end(), moveDirectoriesToFront);
}

DirectoryNode CreateDirectryNodeTreeFromPath(const std::filesystem::path& rootPath)
{   
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
std::string  FileHandler::CreateNewFile(DirectoryNode& ParentNode, const std::filesystem::path& path, const char* file_name)
{
    std::filesystem::path new_file_path = path / file_name;
    if(fs::exists(new_file_path))
        return std::string();
    
    const std::string file = new_file_path.u8string();
    std::ofstream new_file(file);
    new_file.close();

    AddNode(ParentNode, path.u8string(), file_name, std::filesystem::is_directory(new_file_path));
    return new_file_path.u8string();
}

/**
 * true => success
 * false => failed | already exist 
*/
std::string  FileHandler::CreateNewFolder(DirectoryNode& ParentNode, const std::filesystem::path& path, const char* folder_name)
{
    const std::filesystem::path new_path(path / folder_name);
    if(!fs::exists(new_path) && fs::create_directories(new_path)){
        AddNode(ParentNode, path.u8string(), folder_name, std::filesystem::is_directory(new_path));
        return new_path.u8string();
    }
    return new_path.u8string();
}

std::string GetFolderName(const std::string& path) {
    // Find the last occurrence of the path separator (either '/' or '\\')
    size_t lastSeparator = path.find_last_of("/\\");
    if (lastSeparator != std::string::npos) {
        // Extract the folder name
        return path.substr(lastSeparator + 1);
    }
    // If no separator is found, return the entire path
    return path;
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
    }
}

void FileHandler::CreateWholeProjectDirectory(const std::filesystem::path &project_dir, const std::filesystem::path& library_directory, Language language)
{
   /** Project Directory layout:
     *      Root Project Directory
     *          includes
     *              [for libraries]
     *          
     *           main file
    */

    std::filesystem::copy(library_directory, project_dir, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

    // if(!library_directory.empty() && std::filesystem::create_directory(project_dir/"includes"))
    //     std::filesystem::copy(library_directory, project_dir/"includes", std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);

    // {
    //     switch(language)
    //     {
    //     case Language_CPP:
    //         CreateMainCPPFile(project_dir);
    //         break;
            
    //     case Lanugae_Python:
    //         CreateMainPythonFile(project_dir);
    //         break; 
    //     }
    // }
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

void FileHandler::CreateMainCPPFile(const std::filesystem::path &path)
{
    std::ofstream outputFile(path / "main.cpp");
        const std::string file_contents= "#include \"includes/RMR.h\"\n\nint main()\n{\n}";
        outputFile << file_contents;
    outputFile.close();
}

void FileHandler::CreateMainPythonFile(const std::filesystem::path &path)
{   
    std::ofstream outputFile(path / "main.py");
        const std::string file_contents= "def main(): \n\tprint(\"Hello World!\")\n\nif __name__ == \"__main__\": \n\tmain()";
        outputFile << file_contents;
    outputFile.close();
}

bool FileHandler::Search_AddNode(DirectoryNode& ParentNode, const std::string& target_path, const DirectoryNode& to_add)
{
    if(to_add.FullPath.empty() || target_path.empty())
        return false;   

    if(ParentNode.FullPath == target_path)
    {
        ParentNode.Children.push_back(to_add);

        auto moveDirectoriesToFront = [](const DirectoryNode& a, const DirectoryNode& b) {
            if (a.IsDirectory && !b.IsDirectory)
                return true; // Move directories to the front
            else if (!a.IsDirectory && b.IsDirectory)
                return false; // Keep files after directories
            else
                return a.FullPath < b.FullPath; // Sort alphabetically
        };
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



