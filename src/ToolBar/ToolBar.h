#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../ImageHandler/ImageHandler.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <mutex>

namespace ArmSimPro
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

            const TwoStateImageData _image;
            const ImVec2 _size;
            
            const ImVec4 _bg_col;

            ImVec4 _highlighter_col;
            float  _thickness;

            const ImGuiAxis _toolbar_axis;
        
        public:
            ButtonTool( const char* IDname, bool* activation_key, const TwoStateImageData& image, const ImVec2& size, 
                        const ImVec4& bg_col, ImGuiAxis toolbar_axis, bool HasHighligter = false);
            
            bool SetButton();
            void SetHighlighter(const ImVec4& highlighter_col, float thickness);

            inline ImVec4 GetColor() const {return _bg_col;}
        private:
            bool ToggleButtonBehavior(const ImRect& interactiion_bounding_box, ImGuiID _id);

            bool ButtonWithHighlighter();
            bool NormalButton();
        };

        struct ToolTipData
        {
            TwoStateImageData image;
            std::function<void()> ptr_to_func;

            bool isActive;
        };
        
        struct ToolTip
        {
            std::string button_name;
            ToolTipData tool;

            inline bool isToolActive(const std::string& current) const;
            inline bool isToolActive(const std::string& current);          
        };

    private:
        std::mutex ToolItem_Mutex, ToolBar_mutex;
        const std::string _label;
        ImVec2 _tool_size;
        
        const RGBA _bg_col;
        const RGBA _highlighter_col;

        bool _isPrimarySideBarVisible;

        const float _highlighter_thickness;
        const float _toolbar_thickness;
        float _spacing;
        float _toolbart_height;

        float _primary_sidebar_width; //only available when axis is at Y position
        float _primary_sidebar_posY;
        float _total_width;
        
        const ImGuiAxis _toolbar_axis;
        ImGuiViewport* viewport;
        std::vector<ToolTip> Tools;
        std::unordered_map<std::string, float> SpaceBeforeButton;
        std::vector<std::string> ToolsNoHighlight;
        
    public:
        ToolBar() : _toolbar_axis(ImGuiAxis_None), _isPrimarySideBarVisible(false), _highlighter_thickness(0.0f), _toolbar_thickness(0.0f), _spacing(0.0f), _primary_sidebar_width(0.0f) {}
        ToolBar(const char* label, const RGBA& bg_col, float toolbar_thickness, ImGuiAxis toolbar_axis);
        ~ToolBar() {}

        void AppendTool(const char* name, TwoStateImageData image, std::function<void()> ptr_to_func, bool NoHighlight = false, bool ActiveOnRun = false);

        void SetToolBar(float top_margin = 0, float bottom_marigin = 8);
        void SetPaddingBefore(const char* buttonID, float space);
        void SetSpacing(float val) {_spacing = val;}
        void SetPrimarySideBarWidth(float val);

        inline ImVec2 GetToolSize() const           {return _tool_size;}
        inline RGBA GetbackgroundColor() const      {return _bg_col;}

        inline float GetThickness() const           {return _toolbar_thickness;} 
        inline float GetToolBarHeight() const       {return _toolbart_height;}
        inline float GetToolBarPosY() const         {return _primary_sidebar_posY;}
        inline float GetTotalWidth() const          {return _total_width;}

    private:
        void SetSpace(float space);
        void CenterTool(const char* label, float alignment = 0.5f);
        void AlignForHeight(float height, float alignment = 0.5);
        void ChangeButtonStateExcept(const std::string& current_buttonID);
        void VeritcalSpacing(const ToolTip& tool, float spacing);

        void ShowPrimarySideBar(ToolTip& Tool);
        bool RunToolBar(ToolTip& Tool, const ImVec2& requested_size);
        float UpdateWidth(const std::string& current_button);
    };
}