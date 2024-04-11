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
    
    std::vector<std::string> lines;

    if (file.is_open()) {
        std::string line;
        std::vector<std::future<void>> m_futures;
        while (std::getline(file, line)) 
            m_futures.push_back(std::async(std::launch::async, &Search::SearchOnLine, this, line, key, line_number, &occurances, &keys_loc));
        
        file.close();
    }
    return Handler_SearchKeyOnFile (key, path, keys_loc );
}

Search::KeyInstances_Position Search::Search_Needle_On_Haystack(const std::vector<std::string> &text_lines, const String &key)
{
    if(text_lines.empty() || key.empty())
        return KeyInstances_Position();

    return SearchText(text_lines, key);
}

// Function to implement Rabin-Karp algorithm
Search::KeyInstances_Position Search::SearchText(const std::vector<std::string>& text, const std::string& pattern) 
{
    KeyInstances_Position key_pos;
    int line_num = 0;
    
    for (const auto& str : text) 
    {
        const KeyFound_Containter::Offset offset = RabinKarp(str, pattern);

        if(!offset.empty())
            key_pos.push_back(KeyFound_Containter(offset, line_num));
        
        ++line_num;
    }
    return key_pos;
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

size_t Calculate_StringHash(const std::string& text, int prime_number, int modulus)
{
    size_t hash = 0;
    const size_t size = text.size();

    for(size_t i = 0; i < size; i++)
        hash += (text[i] * static_cast<size_t>(std::pow(prime_number, size - i))) % modulus; 

    return hash;
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

Search::KeyFound_Containter::Offset Search::RabinKarp(const std::string &Text, const std::string &Pattern)
{
    KeyFound_Containter::Offset offset;

     // Calculating the length of S and P 
    int Ns = Text.length();
    int Np = Pattern.length();
    
    if(Ns < Np)
        return KeyFound_Containter::Offset();

    // Initialize the value of prime number and mod for calculating hash values
    int prime = 31;
    int mod = 1e9 + 9;
        
    // Calculating the power raise to the taken prime
    std::vector<long long> p_pow(Ns);
    p_pow[0] = 1; 
    for (int i = 1; i < Ns; i++) 
        p_pow[i] = (p_pow[i-1] * prime) % mod;
    
   	
    std::vector<long long> h(Ns + 1, 0); 
    for (int i = 0; i < Ns; i++)
        h[i+1] = (h[i] + (Text[i] - 'a' + 1) * p_pow[i]) % mod;
     
    
    // Calculating the hash value of P 
    long long hash_P = 0; 
    for (int i = 0; i < Np; i++) 
        hash_P = (hash_P + (Pattern[i] - 'a' + 1) * p_pow[i]) % mod; 
    
  	
    /*
       Now slide the pattern by one character and check for the corresponding
       hash value to match with the hash value of the given pattern
    */
    for (int i = 0; i + Np - 1 < Ns; i++) 
    { 
        long long curr_hash = (h[i+Np] + mod - h[i]) % mod; 
        if (curr_hash == hash_P * p_pow[i] % mod)
            offset.push_back(i);
    }

    return offset;
}
