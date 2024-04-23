#include "CmdPanel.h"
#include <windows.h>
#include <string>

#include <sstream>
#include <locale>
#include <codecvt>
#include "../filesystem.hpp"
#include "CommandHandler.hpp"

ArmSimPro::CmdPanel::CmdPanel(const char* IDname, float status_bar_thickness, const RGBA& bg_col, const RGBA& highlighter_col) 
    : _IDname(IDname), _status_bar_thickness(status_bar_thickness), _height(120),  _bg_col(bg_col), _highlighter_col(highlighter_col)
{
    viewport = ImGui::GetMainViewport();
    viewportp = (ImGuiViewportP*)(void*)(viewport);
}

void ArmSimPro::CmdPanel::SetPanel(const std::filesystem::path current_path, float top_margin, float right_margin)
{
    
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
    ImGui::Begin(_IDname.c_str(), NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse);
    { 
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        const float splitter_thickness = 6;
        ImVec2 splitter_size(windowSize.x + windowPos.x, splitter_thickness);
        ImVec2 splitter_pos(0, windowPos.y );

        ImGui::Splitter(std::string(_IDname + " splitter").c_str(), ImGui::GetColorU32(_bg_col.GetCol()),
                        ImGui::GetColorU32(_highlighter_col.GetCol()), splitter_size, splitter_pos, 
                        &_height, ImGuiAxis_X);

//===========================================================Tab Bar===============================================================================
        ImGui::PushStyleColor(ImGuiCol_Tab, _bg_col.GetCol());
        if(ImGui::BeginTabBar("##tabbar", ImGuiTabBarFlags_NoCloseWithMiddleMouseButton))
        {  
            if(ImGui::BeginTabItem("\tOUTPUT\t", nullptr, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton | ImGuiTabItemFlags_NoReorder))
            {
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("\tTERMINAL\t", nullptr, ImGuiTabItemFlags_NoCloseWithMiddleMouseButton | ImGuiTabItemFlags_NoReorder))
            {   
                if(ImGui::BeginChild("Terminal", ImVec2(0,0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar))
                {
                    TerminalControl(current_path.u8string());
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        ImGui::PopStyleColor();
//=================================================================================================================================================

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

void ArmSimPro::CmdPanel::TerminalControl(const std::string& path)
{
    std::string current_path = path;
    const std::string path_identifier = current_path + ">";

    if(!ExecutedTerminalCMDs.empty()){
        for(const auto& cmd : ExecutedTerminalCMDs){
            ImGui::Text("%s", cmd.c_str());
        }
    }

    std::filesystem::current_path(current_path);
    ImGui::Text("%s", path_identifier.c_str());
    ImGui::SameLine();

    std::string cmd;
    ImGui::PushStyleColor(ImGuiCol_FrameBg, _bg_col.GetCol());
    ImGui::PushStyleColor(ImGuiCol_Border, _bg_col.GetCol());

    ImGui::PushItemWidth(ImGui::GetWindowSize().x / 2 - (ImGui::CalcTextSize(path_identifier.c_str()).x - 30));
    
    // The use of input text is just for temporary use. Will update this later.
    if(ImGui::InputText("##terminal", &cmd, ImGuiInputTextFlags_EnterReturnsTrue))
    {
        if(cmd == "cls" || cmd == "clear")
        {
            ExecutedTerminalCMDs.clear();
            ExecutedTerminalCMDs.push_back(path_identifier + " " + cmd);
        }
        else
        {
            std::string result;
            if(cmd.find("cd") != std::string::npos)
            {
                if(cmd.length() < 4)
                    result = current_path;
                else
                { 
                    const std::string path_arg = cmd.substr(3); //removes the "cd" command
                    current_path = current_path + "\\" + path_arg;
                }
            }
            else
                result = ExecuteCommand(cmd, current_path);

            ExecutedTerminalCMDs.push_back(path_identifier + " " + cmd + "\n" + result);
        }
    }

    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);
}


