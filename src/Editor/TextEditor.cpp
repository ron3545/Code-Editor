#include "TextEditor.h"

namespace  ArmSimPro
{
    void CodeEditor::SetCodeEditor(float right_margin, float top_margin,  float bottom_margin)
    {
        static ImGuiDockNodeFlags dockspace = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x + right_margin, viewport->Pos.y));
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        if(dockspace & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
            ImGui::Begin("DockSpace", nullptr, window_flags);
            ImGui::PopStyleVar();
            ImGui::PopStyleVar(2);
            {
                // DockSpace
                ImGuiIO& io = ImGui::GetIO();
                if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
                {
                    ImGuiID dockspace_id = ImGui::GetID("DockSpace");
                    const ImVec2 dockspace_size = ImGui::GetContentRegionAvail();
                    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace | ImGuiDockNodeFlags_NoWindowMenuButton);

                    static auto first_time = true;
                    if (first_time)
                    {
                        first_time = false;

                        ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
                        ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
                        ImGui::DockBuilderSetNodeSize(dockspace_id, dockspace_size);

                        // split the dockspace into 2 nodes -- DockBuilderSplitNode takes in the following args in the following order
                        //   window ID to split, direction, fraction (between 0 and 1), the final two setting let's us choose which id we want (which ever one we DON'T set as NULL, will be returned by the function)
                        //                                                              out_id_at_dir is the id of the node in the direction we specified earlier, out_id_at_opposite_dir is in the opposite direction
                        ImGuiID dock_id_down = ImGui::DockBuilderSplitNode(dockspace_id, ImGuiDir_Right, 0.3f, nullptr, &dockspace_id);
                        ImGuiID dock_id_left = dockspace_id;

                        // we now dock our windows into the docking node we made above
                        //ImGui::DockBuilderDockWindow("CMD viewport", dock_id_down);
                        //ImGui::DockBuilderDockWindow("Controll Panel", dock_id_left);

                        ImGui::DockBuilderFinish(dockspace_id);
                    }
                }
            }
            ImGui::End();
    }
};