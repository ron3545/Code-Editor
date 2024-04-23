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

        Handler_KeyLocation() : m_Line(0), m_offset(0), m_lineNumber(0) {}
        Handler_KeyLocation(const std::string& Line, unsigned int offset, unsigned int lineNumber)
            : m_Line(Line), m_offset(offset), m_lineNumber(lineNumber)
        {}
        
        Handler_KeyLocation& operator = (const Handler_KeyLocation& other)
        {
            this->m_Line = other.m_Line;
            this->m_lineNumber = other.m_lineNumber;
            this->m_offset = other.m_offset;
        }

        inline bool operator < (const Handler_KeyLocation& other)
        {
            return this->m_lineNumber < other.m_lineNumber && this->m_offset < other.m_offset;
        }
    };

    typedef std::string String;
    typedef std::vector<std::tuple<unsigned int, std::string>> Lines_number;
    typedef std::vector<Handler_KeyLocation> KeyLocations;

    struct KeyFound_Containter
    {
        typedef std::vector<int> Offset;

        Offset m_offset;
        unsigned int line_number;

        KeyFound_Containter(const Offset& offset, unsigned int line) 
            : m_offset(offset), line_number(line) {}
    };

    typedef std::vector<KeyFound_Containter> KeyInstances_Position;

    struct Handler_SearchKeyOnFile
    {
        //TODO: modify this
        std::filesystem::path m_path;
        KeyLocations m_KeyLocation;
        String m_key;

        Handler_SearchKeyOnFile(const String& key,
                                const std::filesystem::path& path,
                                KeyLocations KeyLocation)
        : m_key(key), m_path(path), m_KeyLocation(KeyLocation)
        {}

        Handler_SearchKeyOnFile() {}
    };

    void Search_String_On_Files(const std::filesystem::path& project_path, const String& key, std::set<std::filesystem::path>* dest); 

    enum class StringMatchingAlgoType
    {
        StringMatchingAlgoType_KMP,
        StringMatchingAlgoType_RabinKarp,
        StringMatchingAlgoType_Max
    };

    Handler_SearchKeyOnFile Search_Needle_On_Haystack_File(const std::filesystem::path& path, const String& key);
    KeyInstances_Position Search_Needle_On_Haystack(const std::vector<std::string>& text_lines, const String& key);

    void GetFileList(DirectoryNode& parentNode, std::vector<std::filesystem::path>* Files);
    
    KeyInstances_Position SearchText(const std::vector<std::string>& text, const std::string& pattern);

    void CheckKey_On_File(std::set<std::filesystem::path>* dest, const std::filesystem::path& path, std::string_view key);
    void SearchOnLine(const std::string_view& line_view, std::string_view key, size_t line_number, size_t* occurances, KeyLocations* lines);

    KeyFound_Containter::Offset RabinKarp(const std::string& Text, const std::string& Pattern);    
};


