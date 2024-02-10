#pragma once
#include <filesystem>
#include <vector>
#include <map>
#include <mutex>


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

    struct FileHandler_SearchKeyOnFile
    {
        size_t m_lineNumber, m_offset;
        int m_occurrences;
        std::string m_Line;

        FileHandler_SearchKeyOnFile(size_t lineNumber,
                                    size_t offset,
                                    int occurrences,
                                    std::string Line)
        : m_lineNumber(lineNumber), m_offset(offset), m_occurrences(occurrences), m_Line(Line)
        {}

        FileHandler_SearchKeyOnFile() {}
    };

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

    typedef std::vector<FileHandler::FileHandler_SearchKeyOnFile> SearchedKeys;
    std::map<std::filesystem::path, std::vector<FileHandler_SearchKeyOnFile>> Search_String_On_Files(const std::filesystem::path& project_path, const std::string& key); 
    std::tuple<std::filesystem::path, SearchedKeys> Search_Needle_On_Haystack(const std::filesystem::path& path, const std::string& key);

    std::vector<std::filesystem::path> GetFileList(const std::filesystem::path &project_path);
private:
    FileHandler_PasteMode paste_mode; 
    ImFont* text_font;
    std::mutex mutex_class, search_mutex;

    void AddNode(DirectoryNode& ParentNode, const std::string& target_path, const std::string& to_add, bool IsDirectory);
    bool Search_AddNode(DirectoryNode& ParentNode, const std::string& target_path, const DirectoryNode& to_add); 
    bool Search_RemoveNode(DirectoryNode& ParentNode, const std::string& target_path);
};
