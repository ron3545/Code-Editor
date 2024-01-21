#pragma once
#include <filesystem>
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"

struct DirectoryNode
{
	std::string FullPath;
	std::string FileName;
	std::vector<DirectoryNode> Children;
	bool IsDirectory;
    bool Selected;
};

void RecursivelyAddDirectoryNodes(DirectoryNode& parentNode, std::filesystem::directory_iterator directoryIterator);
DirectoryNode CreateDirectryNodeTreeFromPath(const std::filesystem::path& rootPath);

class FileHandler
{
private:
    enum class FileHandler_PasteMode
    {
        FileHandlerMode_Copy,
        FileHandlerMode_Cut
    };

    FileHandler() {}
public:
    //delete this methods.
    FileHandler(FileHandler const&) = delete;
    void operator=(FileHandler const&) = delete;

    static FileHandler& GetInstance()
    {
        static FileHandler instance; // Guaranteed to be destroyed. Only initiated once
        return instance;
    }
    void SetFont(ImFont* font) { this->text_font = font; }

    bool CreateNewFile(DirectoryNode& ParentNode, const std::filesystem::path& path, const char* file_name);
    bool CreateNewFolder(DirectoryNode& ParentNode, const std::filesystem::path& path, const char* folder_name);

    bool DeleteSelectedFile(DirectoryNode& ParentNode, const std::filesystem::path& path);
    bool DeleteSelectedFolder(DirectoryNode& ParentNode, const std::filesystem::path& path);

    void Copy(DirectoryNode& ParentNode, const std::string& path);
    void Cut(DirectoryNode& ParentNode, const std::string& path);

    void Paste(DirectoryNode& ParentNode, const std::filesystem::path& target_path, bool IsDirectory);
    void Paste(DirectoryNode& ParentNode, const std::filesystem::path& src_file, const std::filesystem::path& target_file);

    //barowed from https://github.com/ocornut/imgui/issues/3730
    void Rename(std::string& selected_path, const std::string& new_name);
private:
    FileHandler_PasteMode paste_mode; 
    ImFont* text_font;

    void AddNode(DirectoryNode& ParentNode, const std::string& target_path, const std::string& to_add, bool IsDirectory);
    bool Search_AddNode(DirectoryNode& ParentNode, const std::string& target_path, const DirectoryNode& to_add); 
    bool Search_RemoveNode(DirectoryNode& ParentNode, const std::string& target_path);
};
