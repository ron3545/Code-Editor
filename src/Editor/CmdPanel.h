/**
 * This class is responsible for compiling and executing C++ and Python program;
 * It's also responsible for interacting with the command prompt using the terminal
*/

#pragma once
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../ImageHandler/ImageHandler.h"
#include "TextEditor.h"

#include "../filesystem.hpp"
#include <string>
#include <vector>

#include <mutex>
#include <future>
#include <regex>

namespace ArmSimPro
{
    class CmdPanel 
    {
    public:
        enum ProgrammingLanguage
        {
            PL_PYTHON,
            PL_CPP
        };

        CmdPanel() : viewportp(nullptr) {}
        CmdPanel(const char* IDname, float status_bar_thickness, const RGBA& bg_col, const RGBA& highlighter_col);
        ~CmdPanel() {}

        void SetPanel(const std::filesystem::path current_path, float top_margin, float right_margin, std::string* output_display);
        void BuildRunCode(const std::string& cmd, ProgrammingLanguage pl);
        void SetHeight(float height) {_height = height;}
        
        inline float GetCurretnHeight() const {return _height;}
    private:    
        void TerminalControl(const std::string& current_path);
        void RunPythonProgram(const std::string command, const std::string& current_path);
        void RunCPPProgram(const std::string command, const std::string& current_path);

        std::string ExecuteCommand(const std::string& command, const std::string& current_path);
        std::string ExecuteCommand(const std::string& exe_file_path);
    private:
        std::string _IDname;

        float _status_bar_thickness;
        float _current_height;       
        float _width;
        float _height;

        ImGuiViewport* viewport;
        ImGuiViewportP* viewportp;

        const RGBA _bg_col;
        const RGBA _highlighter_col;
        ImVec2 size, pos;
        
        std::string command;
        ProgrammingLanguage programming_language;

        std::vector<std::string> ExecutedTerminalCMDs;
        std::vector<std::string> Output_messages;
    };
};
