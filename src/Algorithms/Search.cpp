#include "search.h"

#include <fstream>
#include <sstream>
#include <cstdio>
#include <future>
#include <cstdio>
#include <algorithm>
#include <functional>
#include <iomanip>

#include <mio/mmap.hpp>
#include "../CodeEditor/AppLog.hpp"

void Search::Search_String_On_Files(const std::filesystem::path &project_path, const std::string &key, std::set<std::filesystem::path>* dest)
{
    std::vector<std::filesystem::path> Files;
    auto directories = CreateDirectryNodeTreeFromPath(project_path);
    GetFileList(directories, &Files);

    //Launch tasks on seperate threads.
    std::vector<std::future<void>> futures;
    for(const auto& file : Files)
        futures.push_back(std::async(std::launch::async, &Search::CheckKey_On_File, this, dest, file, key));
}

Search::Handler_SearchKeyOnFile Search::Search_Needle_On_Haystack(const std::filesystem::path &path, const std::string &key)
{
    KeyLocations keys_loc;
    size_t line_number = 0;
    size_t occurances = 0;

    std::ifstream file(path);
    
    Lines lines;

    if (file.is_open()) {
        std::string line;
        std::vector<std::future<void>> m_futures;
        while (std::getline(file, line)) 
            m_futures.push_back(std::async(std::launch::async, &Search::SearchOnLine, this, line, key, line_number, &occurances, &keys_loc));
        
        file.close();
    }
    return Handler_SearchKeyOnFile (key, path, keys_loc );
}

void Search::GetFileList(DirectoryNode& parentNode, std::vector<std::filesystem::path>* Files)
{
    const std::string supported_filetype[] = {".h", ".hpp", ".cpp", ".c", ".py", ".txt"}; 

    if(parentNode.IsDirectory)
        for(DirectoryNode& child : parentNode.Children)
            GetFileList(child, Files);
    else    
    {
        const std::filesystem::path temp(parentNode.FileName);
        auto it = std::find(std::begin(supported_filetype), std::end(supported_filetype), temp.extension().u8string().c_str());
        if(it != std::end(supported_filetype))
            Files->push_back(parentNode.FullPath);
    }
}

void Search::CheckKey_On_File(std::set<std::filesystem::path> *dest, const std::filesystem::path &path, std::string_view key)
{
    std::ifstream file(path);
    
    std::string file_contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    auto searcher = std::boyer_moore_horspool_searcher(key.begin(), key.end());
    auto it = std::search(file_contents.begin(), file_contents.end(), searcher);
    const bool is_key_found = it != file_contents.end();

    if(is_key_found)
    {   
        static std::mutex key_on_file_mutex;
        std::lock_guard<std::mutex> lock(key_on_file_mutex);

        dest->insert(path);
    }
    file.close();
}

// Helper Function to remove leading spaces or tabs from a string
std::string removeLeadingWhitespace(const std::string& str) {
    size_t pos = 0;
    // Find the position of the first character that is not a space or tab
    while (pos < str.length() && (str[pos] == ' ' || str[pos] == '\t')) {
        pos++;
    }
    // If there are spaces or tabs at the beginning of the string, remove them
    if (pos > 0) {
        return str.substr(pos);
    }
    // If no spaces or tabs at the beginning, return the original string
    return str;
}

void Search::SearchOnLine(const std::string_view& line, std::string_view key, size_t line_number, size_t *occurances, KeyLocations *lines)
{
    const auto it = std::search(line.begin(), line.end(), std::boyer_moore_searcher(key.begin(), key.end()));
    if ( it != line.end() )
    {
        static std::mutex convert_chr_str_mutex;
        std::lock_guard<std::mutex> lock(convert_chr_str_mutex);

        auto offset = it - line.begin();
        lines->push_back(Handler_KeyLocation(std::string(line), static_cast<unsigned int>(offset), static_cast<unsigned int>(line_number)));
    }
}
