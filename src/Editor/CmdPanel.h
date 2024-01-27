#pragma once
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../ImageHandler/ImageHandler.h"
#include "TextEditor.h"

#include <filesystem>
#include <string>
#include <vector>

#include <mutex>
#include <future>
#include <regex>

namespace ArmSimPro
{
    class CmdPanel 
    {
    private:
        std::mutex cmd_exec_mutex;
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
        
        std::vector<std::string> ExecutedTerminalCMDs;
    public:
        CmdPanel() : viewportp(nullptr) {}
        CmdPanel(const char* IDname, float status_bar_thickness, const RGBA& bg_col, const RGBA& highlighter_col);
        ~CmdPanel() {}

        void SetPanel(const std::filesystem::path current_path, float top_margin, float right_margin);
        void SetHeight(float height) {_height = height;}
        
        inline float GetCurretnHeight() const {return _height;}
    private:    
        void TerminalControl(const std::string& current_path);
        std::string ExecuteCommand(const std::string& command);
    };
};
