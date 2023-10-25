#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>

#include "../ImageHandler/ImageHandler.h"

namespace ImGui
{
    class ToolBar
    {   

        class ButtonTool
        {
        private:
            const char* _IDname;

            bool* _activation_key;
            bool _HasHighligter;
            bool _hovered, _held;

            const ImageData _image;
            const ImVec2 _size;
            
            const ImVec4 _bg_col;
            const ImVec4 _highlighter_col;
            const float  _thickness;
            const ImGuiAxis _toolbar_axis;

            ImRect bb;
            ImGuiID _id;
        public:
            ButtonTool( const char* IDname, bool* activation_key, const ImageData& image, const ImVec2& size, 
                        const ImVec4& bg_col, const ImVec4& highlighter_col, float thickness, bool HasHighligter, 
                        ImGuiAxis toolbar_axis = ImGuiAxis_None);
            bool SetButton();

            inline ImRect GetBoundingBox() const    {return bb;}
            inline ImGuiID GetID() const            {return _id;}
            inline ImVec4 GetColor() const          {return _bg_col;}
        private:
            bool ToggleButtonBehavior(const ImRect& interactiion_bounding_box);

            bool ButtonWithHighlighter();
            bool NormalButton();
        };

        struct ToolTipData
        {
            ImageData image;
            std::function<void()> ptr_to_func;

            bool isActive;
        };
        
        struct ToolTip
        {
            std::string button_name;
            ToolTipData tool;
        };

    private:
        const char* _label;
        const ImVec2 _tool_size;
        
        const RGBA _bg_col;
        const RGBA _highlighter_col;

        float _highlighter_thickness;
        float _toolbar_thickness;

        const ImGuiAxis _toolbar_axis;

        std::vector<ToolTip> Tools;
        std::unordered_map<std::string, float> SpaceBeforeButton;
        std::vector<ToolTip> ToolsNoHighlight;

    public:
        ToolBar() : _label(nullptr), _toolbar_axis(ImGuiAxis_None) {}
        ToolBar(const char* label, const ImVec2& tool_size, const RGBA& bg_col, const RGBA& highlighter_col, float highlighter_thickness, float toolbar_thickness = 50, ImGuiAxis toolbar_axis = ImGuiAxis_Y);
        ToolBar(const char* label, const ImVec2& tool_size, const RGBA& bg_col, float toolbar_thickness = 50, ImGuiAxis toolbar_axis = ImGuiAxis_X);
        
        void AppendTool(const char* name, ImageData image, std::function<void()> ptr_to_func, bool NoHighlight = false);
        void SetToolBar(float top_margin = 0, float bottom_marigin = 8);

        void SetPaddingBefore(const char* buttonID, float space);

        inline ImVec2 GetToolSize() const {return _tool_size;}
        inline float GetThickness() const {return _toolbar_thickness;}
        inline RGBA GetbackgroundColor() const {return _bg_col;}

    private:
        void SetSpace(float space);
        void ChangeButtonStateExcept(const std::string& current_buttonID);
        void ShowOutputPanel();
    };
}
