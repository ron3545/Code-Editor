#include "CmdPanel.h"
#include <windows.h>
#include <string>

#include <ctime>
#include <sstream>
#include <locale>
#include <codecvt>
#include "../filesystem.hpp"

#define COMMAND_EXEC_FAILED "Failed to execute command"

ArmSimPro::CmdPanel::CmdPanel(const char* IDname, float status_bar_thickness, const RGBA& bg_col, const RGBA& highlighter_col) 
    : _IDname(IDname), _status_bar_thickness(status_bar_thickness), _height(120),  _bg_col(bg_col), _highlighter_col(highlighter_col), programming_language(PL_CPP)
{
    viewport = ImGui::GetMainViewport();
    viewportp = (ImGuiViewportP*)(void*)(viewport);
}

void ArmSimPro::CmdPanel::SetPanel(const std::filesystem::path current_path, float top_margin, float right_margin, std::string* output_display)
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
            ImGuiTabItemFlags flag = ImGuiTabItemFlags_NoCloseWithMiddleMouseButton | ImGuiTabItemFlags_NoReorder;

            if(ImGui::BeginTabItem("\tOUTPUT\t", nullptr, flag))
            {   
                if(ImGui::BeginChild("Output", ImVec2(0,0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar))
                {   
                    for(const auto message : Output_messages)
                        ImGui::Text(message.c_str());

                    if(!command.empty() )
                    {
                        Output_messages.clear();
                        Output_messages.push_back("\n[Running] " + command);
                        
                        switch(programming_language)
                        {
                        case PL_CPP:
                            RunCPPProgram(command, current_path.u8string());
                            break;
                        case PL_PYTHON:
                            RunPythonProgram(command, current_path.u8string());
                            break;
                        }

                        command.clear();
                    }
                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }

            //For executing user defuned commands such as git
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

void ArmSimPro::CmdPanel::BuildRunCode(const std::string &cmd, ProgrammingLanguage pl)
{
    command = cmd; 
    programming_language = pl;
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
            ExecutedTerminalCMDs.push_back(path_identifier);
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

std::string ArmSimPro::CmdPanel::ExecuteCommand(const std::string &command, const std::string &current_path)
{
    std::string result;
#ifdef _WIN32
    //Windows specific code
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    HANDLE hRead, hWrite;
    if (CreatePipe(&hRead, &hWrite, &sa, 0)) 
    {
        GetStartupInfo(&si);
        si.hStdError = hWrite;
        si.hStdOutput = hWrite;
        si.dwFlags |= STARTF_USESTDHANDLES;

        // Convert narrow string to wide string
        std::wstring wCommand(command.begin(), command.end());
        std::wstring wPath(current_path.begin(), current_path.end());

        if (CreateProcess(nullptr, 
                          const_cast<LPWSTR>(wCommand.c_str()), 
                          nullptr, 
                          nullptr, 
                          TRUE,
                          CREATE_NO_WINDOW, 
                          nullptr, 
                          const_cast<LPWSTR>(wPath.c_str()), 
                          &si, 
                          &pi)) 
        {
            CloseHandle(hWrite);
            WaitForSingleObject(pi.hProcess, INFINITE);

            DWORD bytesRead;
            char buffer[128];
            while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) != 0) {
                if (bytesRead == 0)
                    break;

                buffer[bytesRead] = '\0';
                result += buffer;
            }

            CloseHandle(hRead);
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        } 
        else 
            result = COMMAND_EXEC_FAILED;
    }
#else
    std::filesystem::current_path(current_path); //change the current working directory
    // Unix-like system code
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }

        pclose(pipe);
    } else {
        result = "Failed to execute command";
    }
#endif
    return result;
}

void ArmSimPro::CmdPanel::RunPythonProgram(const std::string command, const std::string &current_path)
{
    std::stringstream ss;
    const auto start = std::chrono::high_resolution_clock::now();

    std::string message = ExecuteCommand(command, current_path);
    const bool has_no_error_message = message.find("error") == std::string::npos || message.find("syntaxerror") == std::string::npos;
    
    const auto stop = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    ss << message + "\n";
    ss << "[Done] exited with code=";
    ss << (has_no_error_message && !message.empty())? "0" : "1";
    ss << " in ";
    ss << duration.count();
    ss << " microseconds";

    Output_messages.push_back(ss.str());
}

void ArmSimPro::CmdPanel::RunCPPProgram(const std::string command, const std::string& current_path)
{
    std::stringstream ss;
    const auto start = std::chrono::high_resolution_clock::now();

    std::string first_command, second_command;
    size_t pos = command.find("&&");
    if(pos != std::string::npos)
    {
        first_command = command.substr(0, pos);
        first_command.erase(0, first_command.find_first_not_of(" "));
        first_command.erase(first_command.find_last_not_of(" ") + 1);
        
        second_command += command.substr(pos + 2);
        second_command += ".exe";
        second_command.erase(0, second_command.find_first_not_of(" "));
        second_command.erase(second_command.find_last_not_of(" ") + 1);

    }
    
    const std::string compile_result = ExecuteCommand(first_command, current_path);
    ss << "\n" + compile_result;
    const bool has_no_error_message = compile_result.find("error") == std::string::npos || compile_result.find("return 1") == std::string::npos;

    if(has_no_error_message)
    {
        std::string execution_result = ExecuteCommand(second_command);

        //modify the string; Replace every "?" with an apostrophe
        for(size_t pos = execution_result.find("?");  pos != std::string::npos; pos = execution_result.find("?", pos + 1))
            execution_result.replace(pos, 1, "\"");

        ss << "[Executing] " + second_command + "\n";
        ss << execution_result;
    }

    const auto stop = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    
    ss << "\n[Done] exited with code=";
    ss << (has_no_error_message && !compile_result.empty())? "0" : "1";
    ss << " in ";
    ss << duration.count();
    ss << " microseconds";
    Output_messages.push_back(ss.str());
}

std::string ArmSimPro::CmdPanel::ExecuteCommand(const std::string &exe_file_path)
{
    std::string result;
#ifdef _WIN32
    //Windows specific code
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    HANDLE hRead, hWrite;
    if (CreatePipe(&hRead, &hWrite, &sa, 0)) 
    {
        GetStartupInfo(&si);
        si.hStdError = hWrite;
        si.hStdOutput = hWrite;
        si.dwFlags |= STARTF_USESTDHANDLES;
        
        std::wstring stemp = std::wstring(exe_file_path.begin(), exe_file_path.end());
        LPCWSTR sw = stemp.c_str();
        if (CreateProcess(sw,
                          nullptr, 
                          nullptr, 
                          nullptr, 
                          TRUE,
                          NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, 
                          nullptr, 
                          nullptr, 
                          &si, 
                          &pi)) 
        {
            CloseHandle(hWrite);
            WaitForSingleObject(pi.hProcess, INFINITE);

            DWORD bytesRead;
            char buffer[255];
            while (ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, nullptr) != 0) {
                if (bytesRead == 0)
                    break;

                buffer[bytesRead] = '\0';
                result += buffer;
            }

            CloseHandle(hRead);
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
        } 
        else 
            result = COMMAND_EXEC_FAILED;
    }
#else
    std::filesystem::current_path(current_path); //change the current working directory
    // Unix-like system code
    FILE* pipe = popen(command.c_str(), "r");
    if (pipe) {
        char buffer[128];
        while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            result += buffer;
        }

        pclose(pipe);
    } else {
        result = "Failed to execute command";
    }
#endif
    return result;
}
