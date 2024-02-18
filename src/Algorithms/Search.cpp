#include "search.h"

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

std::set<std::filesystem::path> Search::Search_String_On_Files(const std::filesystem::path &project_path, const std::string &key)
{
    if(key.empty())
        return std::set<std::filesystem::path>();

    static std::string prev_path;
    static std::vector<std::filesystem::path> Files; 

    std::set<std::filesystem::path>  result;
    if(project_path.u8string() != prev_path) //execute once
    {
        prev_path = project_path.u8string();
        Files = GetFileList(project_path);
    }

    if(Files.empty())
        return std::set<std::filesystem::path> ();

    //Launch tasks on seperate threads.
    std::vector<std::future<void>> futures;
    for(const auto& file : Files)
        futures.push_back(std::async(std::launch::async, &Search::CheckKey_On_File, this, &result, file, key));

    return result;
}

Search::Handler_SearchKeyOnFile Search::Search_Needle_On_Haystack(const std::filesystem::path &path, const std::string &key)
{
    KeyLocations keys_loc;
    unsigned int line_number = 0;
    unsigned int occurances = 0;

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

std::vector<std::filesystem::path> Search::GetFileList(const std::filesystem::path &project_path)
{
    std::vector<std::filesystem::path> Files;
    std::vector<std::future<void>> m_futures;

    const std::string supported_filetype[] = {".h", ".hpp", ".cpp", ".c", ".py", ".txt"}; 

    for(std::filesystem::recursive_directory_iterator end, dir(project_path); dir != end; ++dir)
    {
        if(dir->path().filename() == "build" || dir->path().filename() == ".git")
            continue;

        if(!dir->is_directory())
        {
            m_futures.push_back(std::async(std::launch::async, [&Files, &dir, &supported_filetype]()
            {
                auto it = std::find(std::begin(supported_filetype), std::end(supported_filetype), dir->path().extension().u8string());
                if(it != std::end(supported_filetype))
                {
                    static std::mutex getFile_mutex;
                    std::lock_guard<std::mutex> lock(getFile_mutex);

                    Files.push_back(dir->path());
                }
            }));
        }
    }
    return Files;
}

void Search::GetLinesFromString(Lines *lines, char chr)
{
    static std::string line;

    static std::mutex getlines_mutex;
    std::lock_guard<std::mutex> lock(getlines_mutex);
    switch(chr)
    {
        case '\r': break;
        case '\n': lines->emplace_back(Line()); line.clear(); break;
        default: lines->back().emplace_back(chr); break;
    }
}

void Search::CheckKey_On_File(std::set<std::filesystem::path> *dest, const std::filesystem::path &path, const std::string &key)
{
    std::error_code error;
    mio::mmap_source ro_mmap;
    ro_mmap.map(path.u8string().c_str(), error);

    std::string data(ro_mmap.begin(), ro_mmap.end());
    
    auto it = std::search(data.begin(), data.end(),
                          std::boyer_moore_searcher(key.begin(), key.end()));

    static std::mutex key_on_file_mutex;
    std::lock_guard<std::mutex> lock(key_on_file_mutex);
    if(it != data.end())
        dest->insert(path);
}

void Search::SearchOnLine(const Line &line, const std::string &key, unsigned int line_number, unsigned int *occurances, KeyLocations *lines)
{
    const std::string m_line(line.begin(), line.end());
    auto it = std::search(m_line.begin(), m_line.end(),
                            std::boyer_moore_searcher(key.begin(), key.end()));

    if(it != m_line.end())
    {
        *occurances += 1;
        size_t offset = it - m_line.begin();

        static std::mutex convert_chr_str_mutex;
        std::lock_guard<std::mutex> lock(convert_chr_str_mutex);
        lines->push_back(Handler_KeyLocation(m_line, offset, line_number));
    }
}
