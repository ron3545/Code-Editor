#pragma once
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../ImageHandler/ImageHandler.h"

#include <string>

namespace ArmSimPro
{
    class CmdPanel
    {
    private:
        std::string _IDname;

        float _status_bar_thickness;
        float _current_height;       
        float _width;
        float _height;

        ImGuiViewport* viewport;
        ImGuiViewportP* viewportp;

        const RGBA _bg_col;
        
    public:
        CmdPanel() : viewportp(nullptr) {}
        CmdPanel(const char* IDname, float status_bar_thickness, const RGBA& bg_col);
        ~CmdPanel() {}

        void SetPanel(float top_margin, float right_margin);
        void SetHeight(float height) {_height = height;}
        void SetWidth(float width)   { width = _width;}

        inline float GetCurrentWidth() const  {return _width;}
        inline float GetCurretnHeight() const {return _height;}
    private:

    };
};