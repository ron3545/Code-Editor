#include "CmdPanel.h"
#include <windows.h>
#include <string>
#include <atlstr.h>
#include <ctime>
#include <sstream>
#include <locale>
#include <codecvt>

#include "../filesystem.hpp"
#include "Async_Wrapper.hpp"

#define COMMAND_EXEC_FAILED "Failed to execute command"
#define BUFFER_SIZE 128

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
            
            if(!command.empty())
                flag = ImGuiTabItemFlags_SetSelected;

            if(ImGui::BeginTabItem("\tOUTPUT\t", nullptr, flag))
            {   
                if(ImGui::BeginChild("Output", ImVec2(0,0), false, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar))
                {   
                    for(const auto message : Output_messages)
                        ImGui::Text(message.c_str());

                    if(!command.empty() )
                    {
                        Output_messages.clear();
                        Output_messages.push_back("[Running] " + command);

                        switch(programming_language)
                        {
                        case PL_CPP:
                            {
                                void_async(&ArmSimPro::CmdPanel::RunCPPProgram, this, command, current_path.u8string()); 

                                //RunCPPProgram(command, current_path.u8string());  //Slow version
                                //std::async(std::launc::async, &ArmSimPro::CmdPanel::RunCPPProgram, this, command, current_path.u8string());
                                break;
                            }
                        case PL_PYTHON:
                            {
                                void_async(&ArmSimPro::CmdPanel::RunPythonProgram, this, command, current_path.u8string());

                                //RunPythonProgram(command, current_path.u8string()); //Slow version
                                //std::async(std::launc::async, &ArmSimPro::CmdPanel::RunPythonProgram, this, command, current_path.u8string());
                                break;
                            }
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
                    void_async(&ArmSimPro::CmdPanel::TerminalControl, this, current_path.u8string());
                    //TerminalControl(current_path.u8string());
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

//https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
std::string ArmSimPro::CmdPanel::ExecuteCommand(const std::string &command,  bool should_run_file)
{
#ifdef _WIN32
    std::string strResult;
    HANDLE hPipeRead, hPipeWrite;

    SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES)};
    saAttr.bInheritHandle = TRUE; // Pipe handles are inherited by child process.
    saAttr.lpSecurityDescriptor = NULL;

     // Create a pipe to get results from child's stdout.
    if (!CreatePipe(&hPipeRead, &hPipeWrite, &saAttr, 0))
        return strResult;
    
    STARTUPINFOW si = {sizeof(STARTUPINFOW)};
    si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    si.hStdOutput  = hPipeWrite;
    si.hStdError   = hPipeWrite;
    si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing.
                              // Requires STARTF_USESHOWWINDOW in dwFlags.

    PROCESS_INFORMATION pi = { 0 };

    BOOL fSuccess = FALSE;
    if(should_run_file) //executes an exe file
    {
        std::wstring stemp = std::wstring(command.begin(), command.end());
        LPCWSTR sw = stemp.c_str();
        
        fSuccess = CreateProcessW(sw,NULL, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
    }
    else //executes a command
    {
        std::wstring wCommand(command.begin(), command.end());
        fSuccess = CreateProcessW(NULL, const_cast<LPWSTR>(wCommand.c_str()), NULL, NULL, TRUE, 0, NULL,NULL, &si, &pi);//const_cast<LPWSTR>(wPath.c_str()), &si, &pi);
    }
   
    if (!fSuccess)
    {
        CloseHandle(hPipeWrite);
        CloseHandle(hPipeRead);
        return strResult;
    }

    bool bProcessEnded = false;
    while (!bProcessEnded)
    {
        // Give some timeslice (50 ms), so we won't waste 100% CPU.
        bProcessEnded = WaitForSingleObject( pi.hProcess, 50) == WAIT_OBJECT_0;

        while(true)
        {
            char buf[1024];
            DWORD dwRead = 0;
            DWORD dwAvail = 0;

            if (!::PeekNamedPipe(hPipeRead, NULL, 0, NULL, &dwAvail, NULL))
                break;

            if (!dwAvail) // No data available, return
                break;

            if (!::ReadFile(hPipeRead, buf, min(sizeof(buf) - 1, dwAvail), &dwRead, NULL) || !dwRead)
                // Error, the child process might ended
                break;

            buf[dwRead] = 0;
            std::string cstemp = buf;
            strResult += cstemp;
        }
    }

    CloseHandle(hPipeWrite);
    CloseHandle(hPipeRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return strResult;
#else
    std::filesystem::current_path(current_path); //change the current working directory
    // Unix-like system code
    std::string result;
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
    return result;
#endif
}


void ArmSimPro::CmdPanel::TerminalControl(const std::string& path)
{
    std::lock_guard<std::mutex> lock(run_terminal_commands);
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
                result = ExecuteCommand(cmd);

            ExecutedTerminalCMDs.push_back(path_identifier + " " + cmd + "\n" + result);
        }
    }

    ImGui::PopItemWidth();
    ImGui::PopStyleColor(2);
}


void ArmSimPro::CmdPanel::RunPythonProgram(const std::string command, const std::string &current_path)
{
    std::stringstream ss;
    const auto start = std::chrono::high_resolution_clock::now();

    std::string message = ExecuteCommand(command);
    const bool has_no_error_message = message.find("error") == std::string::npos || message.find("syntaxerror") == std::string::npos;
    
    const auto stop = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);

    ss << message + "\n";
    ss << "[Done] exited with code=";
    ss << (has_no_error_message && !message.empty())? "0" : "1";
    ss << " in ";
    ss << duration.count();
    ss << " microseconds";

    std::lock_guard<std::mutex> lock(run_python_program);
    Output_messages.push_back(ss.str());
}


void ArmSimPro::CmdPanel::RunCPPProgram(const std::string command, const std::string& current_path)
{
    std::stringstream ss;
    const auto start = std::chrono::high_resolution_clock::now();

    bool has_error_message = false; std::string compile_result;
    
    size_t pos = command.find("&&");
    if(pos != std::string::npos)
    {
        std::string first_command, second_command;
        first_command = command.substr(0, pos);
        first_command.erase(0, first_command.find_first_not_of(" "));
        first_command.erase(first_command.find_last_not_of(" ") + 1);
        
        second_command += command.substr(pos + 2);
        second_command.erase(0, second_command.find_first_not_of(" "));
        second_command.erase(second_command.find_last_not_of(" ") + 1);

        compile_result = ExecuteCommand(first_command); 
        ss << "\n" + compile_result;
        has_error_message = compile_result.find("error") != std::string::npos;

        if(!has_error_message)
        {
            std::string execution_result = ExecuteCommand(second_command, !has_error_message);
            ss << "[Executing] " + second_command + "\n";
            ss << execution_result;
        }
    }

    const auto stop = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    
    ss << "\n[Done] exited with code=";
    ss << (!has_error_message && !compile_result.empty())? "0" : "1";
    ss << " in ";
    ss << duration.count();
    ss << " microseconds";

    std::lock_guard<std::mutex> lock(run_cpp_program_mutex);
    Output_messages.push_back(ss.str());
}


