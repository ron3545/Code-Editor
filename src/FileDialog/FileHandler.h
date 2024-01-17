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

    bool CreateNewFile(const std::filesystem::path& path, const char* file_name);
    bool CreateNewFolder(const std::filesystem::path& path, const char* folder_name);

    bool DeleteSelectedFile(const std::filesystem::path& path);
    bool DeleteSelectedFolder(const std::filesystem::path& path);

    void CopyFile_Folder(const std::string& path);
    void CutFile_Folder(const std::string& path);
    void PasteFile(const std::filesystem::path& target_path);

    //https://github.com/ocornut/imgui/issues/3730
    void Rename(std::string& selected_path, const std::string& new_name);
private:
    FileHandler_PasteMode paste_mode; 
    ImFont* text_font;

    enum class SearchMode_
    {
        SearchMode_AddNode,
        SearchMode_DeleteNode
    };

    bool SearchNode(DirectoryNode& ParentNode, const std::string& path, SearchMode_ mode);
    void AddNode(DirectoryNode& ParentNode, const std::string& path_to_add);
    void RemoveNode(DirectoryNode& ParentNode, const std::string& path_to_remove);
};
