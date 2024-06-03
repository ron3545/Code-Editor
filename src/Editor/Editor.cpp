#include "Editor.hpp"
#include "../CodeEditor/AppLog.hpp"
#include "../TerminalEmulator/Hexe/Terminal/ImGuiTerminal.h"

namespace ArmSimPro
{
    //==========================================================Editor Definition===============================================================================
    void Spacer(float space)
    {
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(space, 0));
        ImGui::SameLine();
    }

    void Editor::Show_Find_Replace_Panel(std::string* to_find, std::string* to_replace, unsigned int* panel_height)
    {
        std::vector<std::string> text_lines = this->GetTextLines(); 
        if(text_lines.empty() || to_find == nullptr || to_replace ==nullptr)
            return;

        const bool is_window_active = ImGui::IsWindowFocused();
        static bool isPressed = false;

        ImGui::Dummy(ImVec2(0, 6));
        ImGui::PushFont(TextFont);
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,255,255,255));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(49,49,49,255));
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(16, 16, 16,255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(24, 24, 24,255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(24, 24, 24,255));

            if(ImGui::ArrowButton("##drop_down", (isPressed)? ImGuiDir_Down : ImGuiDir_Right))
                isPressed = !isPressed;
            
            ImGui::SameLine();

            static bool not_found = false;
            ImGui::PushItemWidth(260);

            ImGui::InputTextWithHint("##Search", "Search Word on Files", to_find, ImGuiInputTextFlags_AutoSelectAll);

            static std::string prev_search_string;
            if( (!to_find->empty() && found_keys.empty()) || *to_find != prev_search_string || IsTextChanged())
            {   
                prev_search_string = *to_find;

                found_keys.clear();
                found_keys = Search::GetInstance().Search_Needle_On_Haystack(text_lines, *to_find);
                not_found = found_keys.empty();
            }
            
            if(to_find->empty())
                found_keys.clear();

            ImGui::PopItemWidth();
            ImGui::PopStyleColor(5);

            Spacer(15);

            ImGui::PushFont(DefaultFont);
                if(not_found) 
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
                    ImGui::Text("No results");
                    ImGui::PopStyleColor();
                }
                else if(to_find->empty())
                {
                    found_keys.clear();
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));
                    ImGui::Text("No results");
                    ImGui::PopStyleColor(); 
                }
                else
                {
                    //Count how many the found keys are
                    static std::string prev_find;
                    if(!this->found_keys.empty() && !to_find->empty() && (*to_find != prev_find || IsTextChanged()) )
                    {
                        prev_find = *to_find;
                        total_index_found_keys = 0;  //Make sure that key start from 0 before adding value
                        for(const auto& keys : found_keys)
                            total_index_found_keys += (unsigned int)keys.m_offset.size();
                    }

                    ImGui::Text("%d of %d", current_index_found_keys, total_index_found_keys);
                }
            ImGui::PopFont();
            
            //Get the coordinates
            {
                static std::string prev_find; 
                if(!this->found_keys.empty() && (prev_find != *to_find || IsTextChanged()))
                {
                    prev_find = *to_find;
                    this->coordinates.clear();

                    for(const auto& keys : this->found_keys) 
                        for(const auto& index : keys.m_offset)
                            this->coordinates.push_back(Coordinates(keys.line_number, index));
                }
            }

            /** TO DO:
             *  Add an enter key after determining which search nav to use.
            */
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 75);
            if((ImGui::ArrowButton("##move up", ImGuiDir_Up) || (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)) && nav_mode == SearchNavMode::SearchNavMode_UP)) && (!this->coordinates.empty() || this->coordinates.size() > 1))
            {   
                curr_coord = this->coordinates.at((coordinate_index > 0)? --coordinate_index : coordinate_index = static_cast<unsigned int>(this->coordinates.size()) - 1);
                current_index_found_keys = (coordinate_index != 0)? coordinate_index : 1; //Update Display

                nav_mode = SearchNavMode::SearchNavMode_UP;
            }

            Spacer(7);
            if((ImGui::ArrowButton("##move down", ImGuiDir_Down) || (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)) && nav_mode == SearchNavMode::SearchNaveMode_Down)) && (!this->coordinates.empty() || this->coordinates.size() > 1))
            {   
                curr_coord = this->coordinates.at((coordinate_index < total_index_found_keys)? coordinate_index++ :  coordinate_index = 0);
                current_index_found_keys = (coordinate_index <= total_index_found_keys && coordinate_index != 0)? coordinate_index : 1; //Update Display

                nav_mode = SearchNavMode::SearchNaveMode_Down;
            }

            if(isPressed)
            {
                *panel_height = 70;
                const int indent = 24;
                ImGui::Indent(indent);
                ImGui::Dummy(ImVec2(0, 6));

                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,255,255,255));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(49,49,49,255));

                ImGui::PushItemWidth(260);
                ImGui::InputTextWithHint("##Replace", "Replace Word on Files", to_replace, ImGuiInputTextFlags_AutoSelectAll);
                
                ImGui::PopItemWidth();

                ImGui::PopStyleColor(2);
                ImGui::Unindent(indent);
            }
            else
                *panel_height = 40;
        ImGui::PopFont();
    }
    
    void Editor::Show_Search_Panel(std::string &to_find, std::string &to_replace, const ImVec2 &offset, const ImVec2 &pos_ofset, bool show_panel, const ImVec4 &bg_col, ImVec2& cursorScreenPos, ImVec2 mCharAdvance, float mTextStart)
    {
        const ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                                              ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoFocusOnAppearing; 

        static unsigned int panel_height = 40;
        const float PanelWidth = 466;
        ImVec2 size,panel_pos;

        ImGuiViewport* viewport = ImGui::GetMainViewport();
        
        ImGuiViewportP* viewportp = (ImGuiViewportP*)(void*)(viewport);
        ImRect available_rect = viewportp->GetBuildWorkRect();

        panel_pos = available_rect.Min;
        panel_pos[ImGuiAxis_Y] = available_rect.Min[ImGuiAxis_Y] + (40 * 2) - 3;
        panel_pos[ImGuiAxis_X] = available_rect.Max[ImGuiAxis_X] - (PanelWidth + 30);

        size = available_rect.GetSize();
        size[ImGuiAxis_Y] = (float)panel_height;

        if (show_panel) 
        {
            ImGui::SetNextWindowSize(ImVec2(PanelWidth, (float)panel_height));
            ImGui::SetNextWindowPos(panel_pos, ImGuiCond_Always);
            ImGui::PushStyleColor(ImGuiCol_WindowBg, bg_col);
            ImGui::Begin("Search and replace", NULL, window_flags);
                Show_Find_Replace_Panel(&to_find, &to_replace, &panel_height);
            ImGui::End();
            ImGui::PopStyleColor();

            const int to_find_size = (int)to_find.size(); 

            static Coordinates prev_coord;
            if(curr_coord != prev_coord) //prevent looping this part which prevents padding when needed; This makes sure positioning happens only once every coordinate
            {
                prev_coord = curr_coord;
                switch(nav_mode)
                {
                    case SearchNavMode::SearchNaveMode_Down:
                    {
                        const Coordinates current_cursor_coordinate = GetActualCursorCoordinates();
                        const unsigned int line_amount = curr_coord.mLine - current_cursor_coordinate.mLine;

                        const int colmn = curr_coord.mColumn + to_find_size;
                        const int Colmn_amount = colmn - current_cursor_coordinate.mColumn;

                        if(!to_replace.empty())
                        {
                            const auto line = mLines[curr_coord.mLine];
                            // To Do: 
                            //Change the characters at this line at a given coordinate.
                            
                        }

                        MoveDown(line_amount);
                        MoveRight(Colmn_amount);
                        break;
                    }

                    case SearchNavMode::SearchNavMode_UP:
                    {
                        const Coordinates current_cursor_coordinate = GetActualCursorCoordinates();
                        const unsigned int line_amount = current_cursor_coordinate.mLine - curr_coord.mLine;

                        const int colmn = curr_coord.mColumn + to_find_size;
                        const int Colmn_amount = current_cursor_coordinate.mColumn - curr_coord.mColumn;

                        MoveUp(line_amount);
                        MoveLeft(Colmn_amount);

                        break;
                    }
                }
            }
            
        }
    }
    void Editor::DrawHighlightsFromSearch(int to_find_size, ImVec2& cursorScreenPos, ImVec2 mCharAdvance, float mTextStart)
    {
        if(this->coordinates.empty())
            return;

        for(const auto& coordinate : this->coordinates)
        {
            //Set selection
            auto selection_start = SanitizeCoordinates(coordinate);
            auto selection_end   = SanitizeCoordinates(Coordinates(coordinate.mLine, coordinate.mColumn + to_find_size));

            if(selection_start > selection_end)
                std::swap(selection_start, selection_end);

            ImVec2 LineStartScreenPos = ImVec2(cursorScreenPos.x, cursorScreenPos.y + coordinate.mLine * mCharAdvance.y);
            ImVec2 textScreenPos = ImVec2(LineStartScreenPos.x + mTextStart, LineStartScreenPos.y);

            Coordinates lineStartCoord(coordinate.mLine, 0);                                    //Starting position of the line
            Coordinates lineEndCoord(coordinate.mLine, GetLineMaxColumn(coordinate.mLine));     //End position of the line

            //Represent the starting and ending position of the sel
            float sstart = -1.0f;  //distance of the selction end from the start of the line
            float ssend = -1.0f;   // distance od the selcetion end from the start of the line

            assert(selection_start <= selection_end);
            if(selection_start <= lineEndCoord)
                sstart = selection_start > lineStartCoord ? TextDistanceToLineStart(selection_start) : 0.0f;
            
            if(selection_end > lineStartCoord)
                ssend = TextDistanceToLineStart(selection_end < lineEndCoord? selection_end : lineEndCoord);
            
            if(selection_end.mLine > coordinate.mLine)
                ssend += mCharAdvance.x;
            
            if (sstart != -1 && ssend != -1 && sstart < ssend)
            {
                ImVec2 vstart(LineStartScreenPos.x + mTextStart + sstart, LineStartScreenPos.y);
                ImVec2 vend(LineStartScreenPos.x + mTextStart + ssend, LineStartScreenPos.y + mCharAdvance.y);

                ImVec4 color = ImVec4(0.8f, 0.9f, 0.17f, 0.35f); //yellow
                if(curr_coord == coordinate)
                    color = ImVec4(0.901f, 0.286f, 0.043f, 0.35f); //red 

                ImGui::GetWindowDrawList()->AddRectFilled(vstart, vend, ImGui::ColorConvertFloat4ToU32(color));
            }
        }
    }

    void Editor::ShowAppLog(bool* p_open, const Search::KeyInstances_Position &positions)
    {
        
        AppLog log;

        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Example: Log", p_open);
        {
            for(const auto position : positions)
                log.AddLog("Line Number: %d ; Offset size: %d \n", position.line_number, position.m_offset.size());
        }
        ImGui::End();

        // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
        log.Draw("Example: Log", p_open);
    }

    void Editor::ShowAppLog(bool *p_open, const std::vector<Coordinates> &positions)
    {
        AppLog log;

        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Example: Log", p_open);
        {
            for(const auto position : positions)
                log.AddLog("Line Number: %d ; column: %d \n", position.mLine, position.mColumn);
        }
        ImGui::End();

        // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
        log.Draw("Example: Log", p_open);
    }

    void Editor::ShowAppLog(bool *p_open, int lineNo, const ImVec2& start, const ImVec2& end)
    {
        AppLog log;
    
        // For the demo: add a debug button _BEFORE_ the normal log window contents
        // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
        // Most of the contents of the window will be added by the log.Draw() call.
        ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Example: Log", p_open);
            log.AddLog("%d-> start[x]: %f ; end[x]: %f\n    start[y]: %f ; end[y]: %f\n\n\n", lineNo, start.x, end.x, start.y, end.y);
        
        ImGui::End();

        // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
        log.Draw("Example: Log", p_open);
    }
};