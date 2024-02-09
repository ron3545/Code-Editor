#include "ToolBar.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <future>

using namespace std::chrono_literals;

namespace ArmSimPro
{   
    inline bool ToolBar::ToolTip::isToolActive(const std::string& current) const
    {
        if(this->button_name == current)
            return this->tool.isActive;
        return false;
    }

    inline bool ToolBar::ToolTip::isToolActive(const std::string& current)
    {
        if(this->button_name == current)
            return this->tool.isActive;
        return false;
    }

    ToolBar::ToolBar(const char* label, const RGBA& bg_col, float toolbar_thickness, ImGuiAxis toolbar_axis)
        : _label(label),  _bg_col(bg_col), _highlighter_col(RGBA(0, 120, 212, 255)), _isPrimarySideBarVisible(false), _highlighter_thickness(3), _toolbar_thickness(toolbar_thickness), 
        _toolbar_axis(toolbar_axis), _spacing(24.6f), _primary_sidebar_width(340)

    {
        _tool_size = ImVec2(toolbar_thickness , toolbar_thickness);
        viewport = ImGui::GetMainViewport();
    }

    void ToolBar::AppendTool(const char* name, ImageData image, std::function<void()> ptr_to_func, bool NoHighlight, bool ActiveOnRun)
    {
        ToolTip tool;
        tool.button_name = name;
        tool.tool.image = image;
        tool.tool.isActive = ActiveOnRun;
        tool.tool.ptr_to_func = ptr_to_func;

        Tools.push_back(tool);

        if(NoHighlight)
            ToolsNoHighlight.push_back(name);
    }

    bool ToolBar::RunToolBar(ToolTip& Tool, const ImVec2& requested_size)
    {
        bool has_highlighter = true;
        if(std::find(ToolsNoHighlight.begin(), ToolsNoHighlight.end(), Tool.button_name) != ToolsNoHighlight.end())
            has_highlighter = false;

        float space = 5;
        auto it = SpaceBeforeButton.find(Tool.button_name);
        if(it != SpaceBeforeButton.end())
            SetSpace(it->second);
        
        if(_toolbar_axis == ImGuiAxis_X)
            AlignForHeight(requested_size.y);
        else
            VeritcalSpacing(Tool, _spacing);
    
        ButtonTool button = ButtonTool(Tool.button_name.c_str(), &Tool.tool.isActive, Tool.tool.image, _tool_size, _bg_col.GetCol(), _toolbar_axis, has_highlighter);
        if(has_highlighter)
            button.SetHighlighter(_highlighter_col.GetCol(), _highlighter_thickness);
        bool pressed = button.SetButton();

        if(Tool.tool.ptr_to_func && pressed)
        {   
            auto future = std::async(std::launch::async, [&](){
                std::lock_guard<std::mutex> lock(ToolItem_Mutex);
                Tool.tool.ptr_to_func(); 
            });
            future.wait();
        }

        if(_toolbar_axis == ImGuiAxis_X)
            ImGui::SameLine();

        ImGui::SetItemTooltip(Tool.button_name.c_str());
        
        if(pressed)
            ChangeButtonStateExcept(Tool.button_name);
        
        return pressed;
    }

    void ToolBar::SetToolBar(float top_margin, float bottom_margin)
    {   
        _toolbart_height = viewport->WorkSize.y - ((top_margin + bottom_margin));
        _primary_sidebar_posY= viewport->Pos.y + ((top_margin * 2) + 7);

        ImVec2 requested_size = (_toolbar_axis == ImGuiAxis_X)? ImVec2(viewport->WorkSize.x, (_toolbar_thickness + 20)) : ImVec2(_toolbar_thickness + 20, _toolbart_height);
        ImVec2 pos = ImVec2(viewport->WorkPos.x, (_toolbar_axis == ImGuiAxis_X)? viewport->Pos.y + top_margin - 11 : _primary_sidebar_posY);
        
        ImGui::SetNextWindowSize(requested_size);
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5.0f, 5.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, _bg_col.GetCol());
        ImGui::Begin(_label.c_str(), NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoDecoration);
        
        static std::string current_button;
        {   
            ImGui::Spacing();
            for(auto& Tool : Tools)
            {
                if(RunToolBar(Tool, requested_size) || Tool.tool.isActive)
                    current_button = Tool.button_name;
                ShowPrimarySideBar(Tool);
            }
        }
        ImGui::PopStyleColor();
        ImGui::End();
        ImGui::PopStyleVar(2);

        if(_toolbar_axis == ImGuiAxis_Y)
            _total_width = UpdateWidth(current_button);

        if(_isPrimarySideBarVisible){
            if(std::all_of(Tools.begin(), Tools.end(), [](const ToolTip& tool){return !tool.tool.isActive;}))
                _isPrimarySideBarVisible = false;
        }
    }

    float ToolBar::UpdateWidth(const std::string& current_button)
    {
        for(auto& Tool : Tools)
        {
            if(Tool.isToolActive(current_button))
                _isPrimarySideBarVisible = true;
        }
        
        if(_isPrimarySideBarVisible)
            return _primary_sidebar_width + _toolbar_thickness;
        else
            return _toolbar_thickness;
    }

    void ToolBar::ShowPrimarySideBar(ToolTip& Tool)
    {   
        if(!_isPrimarySideBarVisible || _toolbar_axis == ImGuiAxis_X || !Tool.tool.isActive)
            return;

        ImGui::SetNextWindowSize(ImVec2(_primary_sidebar_width, _toolbart_height));
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x + (_toolbar_thickness + 20), _primary_sidebar_posY));
        ImGui::SetNextWindowViewport(viewport->ID);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, _bg_col.GetCol());
        ImGui::Begin(std::string(Tool.button_name + " Primary SideBar").c_str(), NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking);
        {   
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImVec2 windowSize = ImGui::GetWindowSize();
            const float splitter_thickness = 6;

            ImGui::PushStyleColor(ImGuiCol_ChildBg, _bg_col.GetCol());
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            
            ImGui::BeginChild(Tool.button_name.c_str(), ImVec2(windowSize.x - splitter_thickness, windowSize.y - splitter_thickness), false,   ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove);
            if(Tool.tool.ptr_to_func)
            {   
                auto future = std::async(std::launch::async, [&](){
                    std::lock_guard<std::mutex> lock(ToolItem_Mutex);
                    Tool.tool.ptr_to_func(); 
                });
                future.wait();
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();

            ImVec2 splitter_size(splitter_thickness, windowPos[ImGuiAxis_Y] + windowSize[ImGuiAxis_Y]);
            ImVec2 splitter_pos(windowPos.x + (_primary_sidebar_width - splitter_thickness), 0);

            ImGui::Splitter(std::string(_label + " splitter").c_str(), ImGui::GetColorU32(_bg_col.GetCol()),
                            ImGui::GetColorU32(_highlighter_col.GetCol()), splitter_size, splitter_pos, 
                            &_primary_sidebar_width, ImGuiAxis_Y);
            
            ImGuiMouseCursor cursor = ImGuiMouseCursor_Arrow;
            if(ImGui::IsItemActive() || ImGui::IsItemHovered())
                cursor = ImGuiMouseCursor_ResizeEW;
            ImGui::SetMouseCursor(cursor);
            //size limit
            if(_primary_sidebar_width < 197.33f)
                _primary_sidebar_width = 197.33f;
            else if(_primary_sidebar_width > 579.33f)
                _primary_sidebar_width = 579.33f;
        }
        ImGui::PopStyleColor();
        ImGui::End();
        ImGui::PopStyleVar(2);
    }

    void ToolBar::VeritcalSpacing(const ToolTip& tool, float spacing)
    {
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return;

        if(&tool == &Tools.front())
            ImGui::ItemSize(ImVec2(0, 0));
        else
            ImGui::ItemSize(ImVec2(0, spacing));
    }

    void ToolBar::SetPaddingBefore(const char* buttonID, float space)
    {
        SpaceBeforeButton.insert(std::make_pair(buttonID, space));
    }

    void ToolBar::SetPrimarySideBarWidth(float val)
    {
        assert(_toolbar_axis == ImGuiAxis_Y); // if axis is not ImGuiAxis_Y, the code will terminate immediately
        _primary_sidebar_width = val;
    }

    void ToolBar::SetSpace(float space)
    {   
        if(_toolbar_axis == ImGuiAxis_X) 
            ImGui::SetCursorPosX(space);
        else
            ImGui::Dummy(ImVec2(0, space));
        
    }
    void ToolBar::AlignForHeight(float height, float alignment)
    {
        ImGuiStyle& style = ImGui::GetStyle();
        float avail = ImGui::GetContentRegionAvail().y;
        float off = (avail - height) * alignment;
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + off);
        ImGui::SameLine();
    }

    void ToolBar::CenterTool(const char* label, float alignment)
    {
        ImGuiStyle& style = ImGui::GetStyle();

        float size = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
        float avail = ImGui::GetContentRegionAvail().x;

        float off = (avail - size) * alignment;
        if (off > 0.0f)
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
    }

    void ToolBar::ChangeButtonStateExcept(const std::string& current_buttonID)
    {
        auto it = std::find(ToolsNoHighlight.begin(), ToolsNoHighlight.end(), current_buttonID);
        if(it != ToolsNoHighlight.end())
            return;

        for(auto& Tool : Tools)
        {
            if(Tool.button_name != current_buttonID)
                Tool.tool.isActive = false;
        }
    }

    
//================================================================================================================================
    ToolBar::ButtonTool::ButtonTool(const char* IDname, bool* activation_key, const ImageData& image, const ImVec2& size, const ImVec4& bg_col, ImGuiAxis toolbar_axis, bool HasHighligter)
        : _IDname(IDname), _activation_key(activation_key), _image(image), _size(size), _toolbar_axis(toolbar_axis), _bg_col(bg_col), _highlighter_col(ImVec4(0,0,0,0)), _thickness(0.0f), _HasHighligter(HasHighligter)
    { }

    void ToolBar::ButtonTool::SetHighlighter(const ImVec4& highlighter_col, float thickness)
    {
        _highlighter_col = highlighter_col;
        _thickness = thickness;
    }

    bool ToolBar::ButtonTool::SetButton()
    {
        if(_HasHighligter)
            return ButtonWithHighlighter();
        return NormalButton();
    }

    bool ToolBar::ButtonTool::ToggleButtonBehavior(const ImRect& interactiion_bounding_box, ImGuiID _id)
    {
        ImGui::ItemSize(interactiion_bounding_box);
        if(!ImGui::ItemAdd(interactiion_bounding_box, _id))
            return false;
        
        bool pressed = ImGui::ButtonBehavior(interactiion_bounding_box, _id, &_hovered, &_held, ImGuiButtonFlags_PressedOnClick);
        if(pressed)
        {
            *_activation_key = !(*_activation_key);
           ImGui::MarkItemEdited(_id);
        }
        return pressed;
    }

    bool ToolBar::ButtonTool::ButtonWithHighlighter()
    {
        if(_thickness == 0.0f && (_highlighter_col.w == 0 && _highlighter_col.x == 0 && _highlighter_col.y == 0 && _highlighter_col.z == 0))
            return false;

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;
        
        ImGuiID _id = window->GetID(_IDname);
        const ImVec2 padding = g.Style.FramePadding;
        ImRect bb = ImRect(window->DC.CursorPos, window->DC.CursorPos + _size + padding * 2.3f);

        bool pressed = ToggleButtonBehavior(bb, _id);
        ImGui::RenderNavHighlight(bb, _id);
        ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(_bg_col), true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, g.Style.FrameRounding));
        
        if(*_activation_key){
            if(_toolbar_axis == ImGuiAxis_Y)
                window->DrawList->AddLine(ImVec2(bb.Min.x - 4, bb.Min.y - 4), ImVec2(bb.Min.x - 4, bb.Max.y + 4), ImGui::GetColorU32(_highlighter_col), _thickness);
            else 
                window->DrawList->AddLine(ImVec2(bb.Min.x - 15, bb.Max.y - 15), ImVec2(bb.Max.x - 15, bb.Max.y + 15), ImGui::GetColorU32(_highlighter_col), _thickness);
        }
        window->DrawList->AddImage((pressed || _hovered || *_activation_key)? (void*)_image.ON_textureID : (void*)_image.OFF_textureID, bb.Min + padding, bb.Max - padding, ImVec2(0, 0), ImVec2(1, 1));
        return pressed;
    }

    bool ToolBar::ButtonTool::NormalButton()
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;
        ImGuiID _id = window->GetID(_IDname);
        const ImVec2 padding = g.Style.FramePadding;
        ImRect bb = ImRect(window->DC.CursorPos, window->DC.CursorPos + _size + padding * 2.0f);

        bool pressed = ToggleButtonBehavior(bb, _id);
        ImGui::RenderNavHighlight(bb, _id);
        ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(_bg_col), ImClamp((float)ImMin(padding.x, padding.y), 0.0f, g.Style.FrameRounding));
        
        window->DrawList->AddImage((pressed || _hovered || _held)? (void*)_image.ON_textureID : (void*)_image.OFF_textureID, bb.Min + padding, bb.Max - padding);
        return pressed;
    }
}