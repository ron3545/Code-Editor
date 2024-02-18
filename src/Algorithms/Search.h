#pragma once
#include <filesystem>
#include <vector>
#include <map>
#include <mutex>
#include <set>
#include <algorithm>

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
    };

    struct Handler_SearchKeyOnFile
    {
        //TODO: modify this
        unsigned int m_occurrences;
        std::filesystem::path m_path;
        std::vector<Handler_KeyLocation> m_KeyLocation;
        std::string m_key;

        Handler_SearchKeyOnFile(unsigned int occurrences, const std::string& key,
                                const std::filesystem::path& path,
                                std::vector<Handler_KeyLocation> KeyLocation)
        :  m_occurrences(occurrences), m_key(key), m_path(path), m_KeyLocation(KeyLocation)
        {}

        Handler_SearchKeyOnFile() {}
    };

    std::set<std::filesystem::path> Search_String_On_Files(const std::filesystem::path& project_path, const std::string& key); 
    Handler_SearchKeyOnFile Search_Needle_On_Haystack(const std::filesystem::path& path, const std::string& key);

    std::vector<std::filesystem::path> GetFileList(const std::filesystem::path &project_path);
private:    
    typedef std::vector<char> Line;
    typedef std::vector<Line> Lines;
    typedef std::vector<Handler_KeyLocation> KeyLocations;

    void GetLinesFromString(Lines* lines, char chr);
    void CheckKey_On_File(std::set<std::filesystem::path>* dest, const std::filesystem::path& path, const std::string& key);
    void SearchOnLine(const Line&, const std::string& key, unsigned int line_number, unsigned int* occurances, KeyLocations* lines);
};