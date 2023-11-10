#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../ImageHandler/ImageHandler.h"

#include <functional>
#include <vector>
#include <unordered_map>

namespace ArmSimPro
{
    class StatusBar
    {
        typedef struct
        {
            std::string ToolID;
            ImageData image;
            std::function<void()> ptr_to_func;

            std::string Text;
            bool this_is_text;
        }ToolData;

    private:
        const char* _IDname;
        const float _thickness;

        const RGBA _bg_col;

        ImVec2 _tool_size;
        std::vector<ToolData> Tools;
        std::unordered_map<const char*, float> Spacer;
    public:

        StatusBar() : _IDname(nullptr), _thickness(NULL) {}
        StatusBar(const char* IDname, float thickness, const RGBA& bg_col) : _IDname(IDname), _thickness(thickness), _bg_col(bg_col) {}

        void BeginStatusBar();
        void EndStatusBar();
        inline float GetHeight() const { return _thickness; } 
    };
}