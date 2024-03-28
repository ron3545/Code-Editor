#include "search.h"

#include <fstream>
#include <sstream>
#include <cstdio>
#include <future>
#include <cstdio>
#include <algorithm>
#include <functional>
#include <iomanip>
#include <array>
#include <cmath>
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

Search::Handler_SearchKeyOnFile Search::Search_Needle_On_Haystack_File(const std::filesystem::path &path, const String &key)
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

Search::KeyInstances_Position Search::Search_Needle_On_Haystack(const std::vector<std::string> &text_lines, const String &key, StringMatchingAlgoType string_matching_type)
{
    if(text_lines.empty())
        return KeyInstances_Position();

    KeyInstances_Position search_result;
    unsigned int line_num = 0; 
 
    //By specifying the enum value, it can automatically select which function to use based on the index of the enum class
    std::function<Search::KeyFound_Containter::Offset(const std::string &text, const std::string &pattern)> Searching_Algo_Types[(int)StringMatchingAlgoType::StringMatchingAlgoType_Max];

    Searching_Algo_Types[(int)StringMatchingAlgoType::StringMatchingAlgoType_KMP] = [this]
        (const std::string &text, const std::string &pattern){ return searchKMP(text, pattern); };

    Searching_Algo_Types[(int)StringMatchingAlgoType::StringMatchingAlgoType_RabinKarp] = [this]
        (const std::string &text, const std::string &pattern){ 
            const size_t length_text = text.length() + 1;
            const size_t length_pattern = pattern.length() + 1;

            char* char_array_text = new char[length_text]; 
            char* char_array_pattern = new char[length_pattern]; 

            strcpy_s(char_array_text, length_text, text.c_str()); 
            strcpy_s(char_array_pattern, length_pattern, pattern.c_str()); 

            return RobinKarp(char_array_text, char_array_pattern); 
        };

    std::vector<std::future<void>> result_futures;
    for(const auto& line : text_lines)
    {
        KeyFound_Containter::Offset result = Searching_Algo_Types[static_cast<unsigned int>(string_matching_type)] (line, key);
        if(!result.empty())
            search_result.push_back(KeyFound_Containter(line, result, line_num));
        line_num++;
    }

    return search_result;
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

void Search::preprocessKMP(const std::string &pattern, std::vector<int> &lps)
{
    int m = pattern.size();
    int len = 0; // Length of the previous longest prefix suffix

    lps[0] = 0; // lps[0] is always 0

    for (int i = 1; i < m; ++i) {
        while (len > 0 && pattern[i] != pattern[len])
            len = lps[len - 1];

        if (pattern[i] == pattern[len])
            ++len;

        lps[i] = len;
    }
}

Search::KeyFound_Containter::Offset Search::searchKMP(const std::string &text, const std::string &pattern)
{
    KeyFound_Containter::Offset offset;
    size_t n = static_cast<size_t>(text.size());       //Size of the main text
    size_t m = static_cast<size_t>(pattern.size());    //Size of the patern

    std::vector<int> lps(m, 0);
    preprocessKMP(pattern, lps);

    int i = 0; // Index for text[]
    int j = 0; // Index for pattern[]

    while (i < n) {
        if (pattern[j] == text[i]) {
            ++i;
            ++j;
        }

        if (j == m) {
            // Pattern found at position (i - j)
            offset.push_back(i-j);

            j = lps[j - 1];
        } else if (i < n && pattern[j] != text[i]) {
            if (j != 0)
                j = lps[j - 1];
            else
                ++i;
        }
    }

    return offset;
}

size_t Calculate_StringHash(const std::string& text, int prime_number, int modulus)
{
    size_t hash = 0;
    const size_t size = text.size();

    for(size_t i = 0; i < size; i++)
        hash += (text[i] * static_cast<size_t>(std::pow(prime_number, size - i))) % modulus; 

    return hash;
}

Search::KeyFound_Containter::Offset Search::RobinKarp(char pat[], char txt[])
{
    KeyFound_Containter::Offset key_found_indexes;
    
    const int M = static_cast<int>(strlen(pat));
    const int N = static_cast<int>(strlen(txt));
    int i, j;
    int p = 0; // hash value for pattern
    int t = 0; // hash value for txt
    int h = 1;

    int q = INT_MAX;
    int d = 256;

    // The value of h would be "pow(d, M-1)%q"
    for (i = 0; i < M - 1; i++)
        h = (h * d) % q;
 
    // Calculate the hash value of pattern and first
    // window of text
    for (i = 0; i < M; i++) {
        p = (d * p + pat[i]) % q;
        t = (d * t + txt[i]) % q;
    }
 
    // Slide the pattern over text one by one
    for (i = 0; i <= N - M; i++) {
 
        // Check the hash values of current window of text
        // and pattern. If the hash values match then only
        // check for characters one by one
        if (p == t) {
            /* Check for characters one by one */
            for (j = 0; j < M; j++) {
                if (txt[i + j] != pat[j]) {
                    break;
                }
            }
 
            // if p == t and pat[0...M-1] = txt[i, i+1,
            // ...i+M-1]
 
            if (j == M)
                key_found_indexes.push_back(i);
        }
 
        // Calculate hash value for next window of text:
        // Remove leading digit, add trailing digit
        if (i < N - M) {
            t = (d * (t - txt[i] * h) + txt[i + M]) % q;
 
            // We might get negative value of t, converting
            // it to positive
            if (t < 0)
                t = (t + q);
        }
    }

    return key_found_indexes;
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


