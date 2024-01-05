#include "FileHandler.h"
#include <fstream>
#include <cstdio>
#include "lwlog.h"

namespace fs = std::filesystem;

/**
 * true => success
 * false => failed | already exist 
*/
bool FileHandler::CreateNewFile(const std::filesystem::path& path, const char* file_name)
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
bool FileHandler::CreateNewFolder(const std::filesystem::path& path, const char* folder_name)
{
    if(!fs::exists(path / folder_name) && fs::create_directories(path / folder_name))
        return true;
    return false;
}

bool FileHandler::DeleteSelectedFile(const std::filesystem::path& path)
{
    return fs::remove(path);
}

bool FileHandler::DeleteSelectedFolder(const std::filesystem::path& path)
{
    return fs::remove_all(path);
}

void FileHandler::CopyFile_Folder(const std::string& path)
{ 
    paste_mode = FileHandler_PasteMode::FileHandlerMode_Copy;
    ImGui::SetClipboardText(path.c_str());
}

void FileHandler::CutFile_Folder(const std::string& path)
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

void FileHandler::PasteFile(const std::filesystem::path& target_path)
{   
    bool overwrite_file = false;
    fs::path src_file = ImGui::GetClipboardText();

    fs::path target = target_path / src_file.filename();
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

    switch(paste_mode)
    {
    case FileHandler_PasteMode::FileHandlerMode_Copy:
        {
            try
            {
                fs::copy_file(src_file, target, (overwrite_file)? fs::copy_options::overwrite_existing : fs::copy_options::skip_existing);
            }
            catch(const std::exception& e)
            {
                
            }
            
        }return;
    
    case FileHandler_PasteMode::FileHandlerMode_Cut:
        {
            try
            {
               fs::rename(src_file.u8string().c_str(), target.u8string().c_str()); 
            }
            catch(const std::exception& e)
            {
                
            }
            
        }return;
    }
}

void FileHandler::Rename(std::string& selected_path, const std::string& new_name)
{
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