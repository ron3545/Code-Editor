#pragma once 
#include <windows.h>
#include <string>

#include <sstream>
#include <locale>
#include <codecvt>
#include "../filesystem.hpp"

std::string ExecuteCommand(const std::string& command, const std::string& current_path);