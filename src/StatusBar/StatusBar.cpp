#include "StatusBar.h"

void ArmSimPro::StatusBar::BeginStatusBar()
{
    _tool_size = ImVec2(_thickness, _thickness);
    ImGuiViewport* viewport = ImGui::GetMainViewport();

    ImVec2 size, pos;
    //Set the status bar to the very bottom of the window
    {
        ImGuiViewportP* viewportp = (ImGuiViewportP*)(void*)(viewport);
        ImRect available_rect = viewportp->GetBuildWorkRect();

        pos = available_rect.Min;
        pos[ImGuiAxis_Y] = available_rect.Max[ImGuiAxis_Y] - _thickness;

        size = available_rect.GetSize();
        size[ImGuiAxis_Y] = _thickness;
    }

    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5.0f, 5.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, _bg_col.GetCol());
    ImGui::Begin(_IDname, NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing);
}
void ArmSimPro::StatusBar::EndStatusBar()
{
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
    ImGui::End();
}
