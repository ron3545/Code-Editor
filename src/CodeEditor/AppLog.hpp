#pragma once
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

#include "../filesystem.hpp"
#include <set>

struct AppLog
{
    ImGuiTextBuffer     Buf;
    ImGuiTextFilter     Filter;
    ImVector<int>       LineOffsets; // Index to lines offset. We maintain this with AddLog() calls.
    bool                AutoScroll;  // Keep scrolling if already at the bottom.

    AppLog()
    {
        AutoScroll = true;
        Clear();
    }

    void    Clear()
    {
        Buf.clear();
        LineOffsets.clear();
        LineOffsets.push_back(0);
    }

    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        int old_size = Buf.size();
        va_list args;
        va_start(args, fmt);
        Buf.appendfv(fmt, args);
        va_end(args);
        for (int new_size = Buf.size(); old_size < new_size; old_size++)
            if (Buf[old_size] == '\n')
                LineOffsets.push_back(old_size + 1);
    }

    void    Draw(const char* title, bool* p_open = NULL)
    {
        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::EndPopup();
        }

        // Main window
        if (ImGui::Button("Options"))
            ImGui::OpenPopup("Options");
        ImGui::SameLine();
        bool clear = ImGui::Button("Clear");
        ImGui::SameLine();
        bool copy = ImGui::Button("Copy");
        ImGui::SameLine();
        Filter.Draw("Filter", -100.0f);

        ImGui::Separator();

        if (ImGui::BeginChild("scrolling", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (clear)
                Clear();
            if (copy)
                ImGui::LogToClipboard();

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
            const char* buf = Buf.begin();
            const char* buf_end = Buf.end();
            if (Filter.IsActive())
            {
                // In this example we don't use the clipper when Filter is enabled.
                // This is because we don't have random access to the result of our filter.
                // A real application processing logs with ten of thousands of entries may want to store the result of
                // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
                for (int line_no = 0; line_no < LineOffsets.Size; line_no++)
                {
                    const char* line_start = buf + LineOffsets[line_no];
                    const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                    if (Filter.PassFilter(line_start, line_end))
                        ImGui::TextUnformatted(line_start, line_end);
                }
            }
            else
            {
                ImGuiListClipper clipper;
                clipper.Begin(LineOffsets.Size);
                while (clipper.Step())
                {
                    for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
                    {
                        const char* line_start = buf + LineOffsets[line_no];
                        const char* line_end = (line_no + 1 < LineOffsets.Size) ? (buf + LineOffsets[line_no + 1] - 1) : buf_end;
                        ImGui::TextUnformatted(line_start, line_end);
                    }
                }
                clipper.End();
            }
            ImGui::PopStyleVar();

            // Keep up at the bottom of the scroll region if we were already at the bottom at the beginning of the frame.
            // Using a scrollbar or mouse-wheel will take away from the bottom edge.
            if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                ImGui::SetScrollHereY(1.0f);
        }
        ImGui::EndChild();
        ImGui::End();
    }
};

static void ShowAppLog(bool* p_open, const std::set<std::filesystem::path>& data)
{
    static AppLog log;

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Example: Log", p_open);
    {
        static int counter = 0;
        const char* categories[3] = { "info", "warn", "error" };
        for (const auto line : data)
        {
            log.AddLog("file: '%s'\n", line.filename().u8string().c_str());
            counter++;
        }
    }
    ImGui::End();

    // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
    log.Draw("Example: Log", p_open);
}

static void ShowAppLog(bool* p_open, const std::vector<std::string>& data)
{
    static AppLog log;

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Example: Log", p_open);
    {
        for (const auto line : data)
            log.AddLog("file: '%s'\n", line.c_str());
        
    }
    ImGui::End();

    // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
    log.Draw("Example: Log", p_open);
}

static void ShowAppLog(bool* p_open, const std::string& data)
{
    static AppLog log;

    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Example: Log", p_open);
    {
        log.AddLog("file: '%s'\n", data.c_str());
        
    }
    ImGui::End();

    // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
    log.Draw("Example: Log", p_open);
}

static void ShowAppLog(bool* p_open, size_t data)
{
    static AppLog log;

    // For the demo: add a debug button _BEFORE_ the normal log window contents
    // We take advantage of a rarely used feature: multiple calls to Begin()/End() are appending to the _same_ window.
    // Most of the contents of the window will be added by the log.Draw() call.
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Example: Log", p_open);
        log.AddLog("offset: '%d'\n", data);
    
    ImGui::End();

    // Actually call in the regular Log helper (which will Begin() into the same window as we just did)
    log.Draw("Example: Log", p_open);
}
