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

    std::error_code error;
    mio::mmap_source ro_mmap;
    ro_mmap.map(path.u8string().c_str(), error);
    
    const std::string data(ro_mmap.begin(), ro_mmap.end());

    Lines lines;
    lines.emplace_back(Line());

    for(auto& chr : data)
        GetLinesFromString(&lines, chr);

    std::vector<std::future<void>> SearchOnLine_future;
    for(const auto& line : lines){
        SearchOnLine_future.push_back(std::async(std::launch::async, &Search::SearchOnLine, this, line, key, line_number, &occurances, &keys_loc));
        ++line_number;
    }

    Handler_SearchKeyOnFile result(occurances, key, path, keys_loc );
    return result;
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

void Search::GetLinesFromString(Lines *lines, char chr)
{
    static std::string line;

    switch(chr)
    {
        case '\r': break;
        case '\n': lines->emplace_back(Line()); line.clear(); break;
        default: lines->back().emplace_back(chr); break;
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

void Search::SearchOnLine(const Line &line, std::string_view key, size_t line_number, size_t *occurances, KeyLocations *lines)
{
    const std::string data(line.begin(), line.end());
    const std::string_view m_line = data;

    auto it = std::search(m_line.begin(), m_line.end(),
                            std::boyer_moore_horspool_searcher(key.begin(), key.end()));

    if (const auto it = std::search(m_line.begin(), m_line.end(),
            std::boyer_moore_searcher(key.begin(), key.end()));
        it != m_line.end()
    )
    {
        size_t offset = it - m_line.begin();

        static std::mutex convert_chr_str_mutex;
        std::lock_guard<std::mutex> lock(convert_chr_str_mutex);

        *occurances += 1;
        lines->push_back(Handler_KeyLocation(data, offset, line_number));
    }
}
