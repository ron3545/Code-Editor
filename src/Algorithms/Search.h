#pragma once
#include "../filesystem.hpp"
#include <vector>
#include <map>
#include <mutex>
#include <set>
#include <algorithm>
#include <string_view>

#include "../FileDialog/FileHandler.h"


class Search
{
private:
    
    Search() {}
public:
    Search(Search const&) = delete;
    void operator=(Search const&) = delete;

    static Search& GetInstance()
    {
        static Search instance; // Guaranteed to be destroyed. Only initiated once
        return instance;
    }

    struct Handler_KeyLocation
    {
        std::string m_Line;
        unsigned int m_offset, m_lineNumber;
        Handler_KeyLocation(const std::string& Line, unsigned int offset, unsigned int lineNumber)
            : m_Line(Line), m_offset(offset), m_lineNumber(lineNumber)
        {}

        inline bool operator < (const Handler_KeyLocation& other)
        {
            return this->m_lineNumber < other.m_lineNumber && this->m_offset < other.m_offset;
        }
    };

    typedef std::vector<std::string> Lines;
    typedef std::vector<Handler_KeyLocation> KeyLocations;

    struct Handler_SearchKeyOnFile
    {
        //TODO: modify this
        std::filesystem::path m_path;
        KeyLocations m_KeyLocation;
        std::string m_key;

        Handler_SearchKeyOnFile(const std::string& key,
                                const std::filesystem::path& path,
                                KeyLocations KeyLocation)
        : m_key(key), m_path(path), m_KeyLocation(KeyLocation)
        {}

        Handler_SearchKeyOnFile() {}
    };

    void Search_String_On_Files(const std::filesystem::path& project_path, const std::string& key, std::set<std::filesystem::path>* dest); 
    Handler_SearchKeyOnFile Search_Needle_On_Haystack(const std::filesystem::path& path, const std::string& key);

    void GetFileList(DirectoryNode& parentNode, std::vector<std::filesystem::path>* Files);
    
private:    
    void CheckKey_On_File(std::set<std::filesystem::path>* dest, const std::filesystem::path& path, std::string_view key);
    void SearchOnLine(const std::string_view& line_view, std::string_view key, size_t line_number, size_t* occurances, KeyLocations* lines);
};


