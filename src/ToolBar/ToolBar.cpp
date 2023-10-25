#include "ToolBar.h"
#include <algorithm>


namespace ImGui
{
    ToolBar::ToolBar(const char* label, const ImVec2& tool_size, const RGBA& bg_col, const RGBA& highlighter_col, float highlighter_thickness, float toolbar_thickness, ImGuiAxis toolbar_axis)
        : _label(label), _tool_size(tool_size), _bg_col(bg_col), _highlighter_col(highlighter_col), _highlighter_thickness(highlighter_thickness), _toolbar_thickness(toolbar_thickness), _toolbar_axis(toolbar_axis)
    {

    }

    ToolBar::ToolBar(const char* label, const ImVec2& tool_size, const RGBA& bg_col, float toolbar_thickness, ImGuiAxis toolbar_axis)
        : _label(label), _tool_size(tool_size), _bg_col(bg_col), _highlighter_col(RGBA(0,0,0,0)), _highlighter_thickness(NULL), _toolbar_thickness(toolbar_thickness), _toolbar_axis(toolbar_axis)

    {

    }

    void ToolBar::AppendTool(const char* name, ImageData image, std::function<void()> ptr_to_func, bool NoHighlight)
    {
        ToolTip tool;
        tool.button_name = name;
        tool.tool.image = image;
        tool.tool.isActive = false;
        tool.tool.ptr_to_func = ptr_to_func;

        if(NoHighlight)
            ToolsNoHighlight.push_back(tool);
        else
            Tools.push_back(tool);
    }

    void ToolBar::SetToolBar(float top_margin, float bottom_margin)
    {
        ImGuiViewport* viewport = GetMainViewport();
        ImVec2 requested_size = (_toolbar_axis == ImGuiAxis_X)? ImVec2(viewport->WorkSize.x, _toolbar_thickness) : ImVec2(viewport->WorkSize.y - bottom_margin, _toolbar_thickness + 10);

        SetNextWindowSize(requested_size);
        SetNextWindowPos(viewport->Pos + ImVec2(0, top_margin - bottom_margin));
        SetNextWindowViewport(viewport->ID);

        ImGuiWindowFlags window_flags =   ImGuiWindowFlags_MenuBar 
                                        | ImGuiWindowFlags_NoDocking
                                        | ImGuiWindowFlags_NoTitleBar 
                                        | ImGuiWindowFlags_NoCollapse
                                        | ImGuiWindowFlags_NoResize 
                                        | ImGuiWindowFlags_NoMove
                                        | ImGuiWindowFlags_NoBringToFrontOnFocus 
                                        | ImGuiWindowFlags_NoNavFocus;

        PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.5f);
        PushStyleColor(ImGuiCol_WindowBg, _bg_col.GetCol());
        ImGui::Begin(_label, NULL, window_flags);
        {
            for(auto& Tool : Tools)
            {
                auto it = SpaceBeforeButton.find(Tool.button_name);
                if(it != SpaceBeforeButton.end())
                    SetSpace(it->second);
                
                ButtonTool button = ButtonTool(Tool.button_name.c_str(), &Tool.tool.isActive, Tool.tool.image, _tool_size, _bg_col.GetCol(), 
                                    (_toolbar_axis == ImGuiAxis_Y)? _highlighter_col.GetCol() : ImVec4(0,0,0,0), (_toolbar_axis == ImGuiAxis_Y)? _highlighter_thickness : NULL,
                                    (_toolbar_axis == ImGuiAxis_Y)? true : false, _toolbar_axis);
                bool pressed = button.SetButton();
                if(pressed)
                    ChangeButtonStateExcept(Tool.button_name);
            }
        }
        PopStyleColor();
        ImGui::End();
        ImGui::PopStyleVar(3);
    }

    void ToolBar::SetPaddingBefore(const char* buttonID, float space)
    {
        SpaceBeforeButton.insert(std::make_pair(buttonID, space));
    }

    void ToolBar::SetSpace(float space)
    {   
        if(_toolbar_axis == ImGuiAxis_X)
            ImGui::Dummy(ImVec2(space, 0));
        else
            ImGui::Dummy(ImVec2(0, space));
    }

    void ToolBar::ChangeButtonStateExcept(const std::string& current_buttonID)
    {
        for(auto& Tool : Tools)
        {
            if(Tool.button_name.compare(current_buttonID) != 0)
                Tool.tool.isActive = false;
        }
    }

    void ToolBar::ShowOutputPanel()
    {

    }
//================================================================================================================================
    ToolBar::ButtonTool::ButtonTool(const char* IDname, bool* activation_key, const ImageData& image, const ImVec2& size, const ImVec4& bg_col, const ImVec4& highlighter_col, float thickness, bool HasHighligter, ImGuiAxis toolbar_axis)
        : _IDname(IDname), _activation_key(activation_key), _image(image), _size(size), _toolbar_axis(toolbar_axis), _bg_col(bg_col), _highlighter_col(highlighter_col), _thickness(thickness), _HasHighligter(HasHighligter)
    { }

    bool ToolBar::ButtonTool::SetButton()
    {
        if(_HasHighligter)
            return ButtonWithHighlighter();
        return NormalButton();
    }

    bool ToolBar::ButtonTool::ToggleButtonBehavior(const ImRect& interactiion_bounding_box)
    {
        ImGui::ItemSize(interactiion_bounding_box);
        if(!ImGui::ItemAdd(interactiion_bounding_box, _id))
            return false;
        
        bool pressed = ImGui::ButtonBehavior(interactiion_bounding_box, _id, &_hovered, &_held, ImGuiButtonFlags_PressedOnClick);
        if(pressed)
        {
            *_activation_key = !(*_activation_key);
            MarkItemEdited(_id);
        }
        return pressed;
    }

    bool ToolBar::ButtonTool::ButtonWithHighlighter()
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;
        
        _id = window->GetID(_IDname);
        const ImVec2 padding = g.Style.FramePadding;
        bb = ImRect(window->DC.CursorPos, window->DC.CursorPos + _size + padding * 2.0f);

        bool pressed = ToggleButtonBehavior(bb);
        RenderNavHighlight(bb, _id);
        RenderFrame(bb.Min, bb.Max, GetColorU32(_bg_col), ImClamp((float)ImMin(padding.x, padding.y), 0.0f, g.Style.FrameRounding));
        
        if(*_activation_key || pressed){
            if(_toolbar_axis == ImGuiAxis_Y)
                window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y), ImVec2(bb.Min.x, bb.Max.y), GetColorU32(_highlighter_col), _thickness);
            else 
                window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Max.y), ImVec2(bb.Max.x, bb.Max.y), GetColorU32(_highlighter_col), _thickness);
        }
        window->DrawList->AddImage((pressed || _hovered || *_activation_key)? (void*)_image.ON_textureID : (void*)_image.OFF_textureID, bb.Min + padding, bb.Max - padding);
        return pressed;
    }

    bool ToolBar::ButtonTool::NormalButton()
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;
        
        const ImVec2 padding = g.Style.FramePadding;
        bb = ImRect(window->DC.CursorPos, window->DC.CursorPos + _size + padding * 2.0f);

        bool pressed = ToggleButtonBehavior(bb);
        RenderNavHighlight(bb, _id);
        RenderFrame(bb.Min, bb.Max, GetColorU32(_bg_col), ImClamp((float)ImMin(padding.x, padding.y), 0.0f, g.Style.FrameRounding));
        
        window->DrawList->AddImage((pressed || _hovered || _held)? (void*)_image.ON_textureID : (void*)_image.OFF_textureID, bb.Min + padding, bb.Max - padding);
        return pressed;
    }
}