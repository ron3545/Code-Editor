#include "CmdPanel.h"

ArmSimPro::CmdPanel::CmdPanel(const char* IDname, float status_bar_thickness, const RGBA& bg_col, const RGBA& highlighter_col) 
    : _IDname(IDname), _status_bar_thickness(status_bar_thickness), _height(120),  _bg_col(bg_col), _highlighter_col(highlighter_col)
{
    viewport = ImGui::GetMainViewport();
    viewportp = (ImGuiViewportP*)(void*)(viewport);
}

void ArmSimPro::CmdPanel::SetPanel(float top_margin, float right_margin)
{
    ImVec2 size, pos;
    //Set the status bar to the very bottom of the window
    ImRect available_rect = viewportp->GetBuildWorkRect();
    //ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
    {
        pos = available_rect.Min;
        pos[ImGuiAxis_Y] = (available_rect.Max[ImGuiAxis_Y] - _height) - _status_bar_thickness;
        pos[ImGuiAxis_X] += right_margin + 20;

        size = available_rect.GetSize();
        size[ImGuiAxis_Y] = _height;
        size[ImGuiAxis_X] += right_margin;
    }

    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5.0f, 5.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, _bg_col.GetCol());
    ImGui::Begin(_IDname.c_str(), NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus );
    { 
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        const float splitter_thickness = 6;
        ImVec2 splitter_size(windowSize.x + windowPos.x, splitter_thickness);
        ImVec2 splitter_pos(0, windowPos.y );

        ImGui::Splitter(std::string(_IDname + " splitter").c_str(), ImGui::GetColorU32(_bg_col.GetCol()),
                        ImGui::GetColorU32(_highlighter_col.GetCol()), splitter_size, splitter_pos, 
                        &_height, ImGuiAxis_X);
        
        ImGuiMouseCursor cursor = ImGuiMouseCursor_Arrow;
        if(ImGui::IsItemActive() || ImGui::IsItemHovered())
                cursor = ImGuiMouseCursor_ResizeNS;
        ImGui::SetMouseCursor(cursor);

        //size limit
        if(_height < 93)
            _height = 93;
        else if(_height > 404)
            _height = 404;
    }
    ImGui::PopStyleColor();
    ImGui::End();
    ImGui::PopStyleVar(3);
}