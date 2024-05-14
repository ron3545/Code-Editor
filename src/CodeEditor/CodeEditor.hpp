#pragma once

#include <windows.h>
#include <string>
#include <shlobj.h>
#include <sstream>
#include <d3d11.h>
#include <dwmapi.h>
#include <map>
#include <unordered_map>

#include <mutex>
#include <regex>
#include <sstream>
#include <fstream>
#include <streambuf>

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include <set>
#include <algorithm>
#include <functional>
#include <iterator>
#include <functional>

#include "../filesystem.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"

#include <nlohmann/json.hpp>

#include "../ToolBar/ToolBar.h"
#include "../StatusBar/StatusBar.h"
#include "../Editor/CmdPanel.h"
#include "../Editor/TextEditor.h"

#include "../IconFontHeaders/IconsCodicons.h"
#include "../IconFontHeaders/IconsMaterialDesignIcons.h"

#include "../FileDialog/FileHandler.h"
#include "../FileDialog/FileDialog.h"

#include "../Algorithms/Search.h"
#include "Utility.hpp"
#include "AppLog.hpp"

#include <unordered_set>
#include <array>
#include <memory>

namespace fs = std::filesystem;

class CodeEditor
{
private:
    const char* m_Consolas_Font;
    const char* m_DroidSansMono_Font;
    const char* m_Menlo_Regular_Font;
    const char* m_MONACO_Font;

    float explorer_panel_width = 320;
    float search_replace_panel_offset;

    bool auto_save;
    bool UseDefault_Location;
    bool ShouldCloseEditor;

    fs::path prev_system_path;
    fs::path SelectedProjectPath; 
    fs::path NewProjectDir; 

    std::string selected_window_path, prev_selected_window_path; // for editing
    std::string current_editor, found_selected_editor;
    std::string Project_Name, dialog_name;

    std::string to_find, to_replace;  bool use_search_panel = false;//for searching
    
    typedef std::vector<ArmSimPro::TextEditorState> TextEditors;
    TextEditors Opened_TextEditors;  //Storage for all the instances of text editors that has been opened

    const RGBA bg_col = RGBA(16, 16, 16, 255);
    const RGBA highlighter_col = RGBA(0, 120, 212, 255);
    const ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f); 

    std::map<std::filesystem::path, Search::Handler_SearchKeyOnFile> SearchResults;

    ImFont* DefaultFont;    
    ImFont* CodeEditorFont;
    ImFont* FileTreeFont;
    ImFont* StatusBarFont;
    ImFont* TextFont;   
    ImFont* IMDIFont;
    ImFont* ICFont;

    DirectoryNode project_root_node;
//======================================CLASS DECLARATION================================================================================
    //std::unique_ptr< ArmSimPro::ToolBar > vertical_tool_bar;
    std::unique_ptr< ArmSimPro::ToolBar > horizontal_tool_bar;
    std::unique_ptr< ArmSimPro::StatusBar > status_bar;
    std::unique_ptr< ArmSimPro::CmdPanel > cmd_panel;

//====================================CLASS MUTEXES=======================================================================================
    std::mutex LoadEditor_mutex;
    std::mutex opened_editor;
    std::mutex RecentFile_Mutex;
    std::mutex editor_mutex;
    std::mutex CodeEditor_mutex;
//===============================================================================================================================================

    enum DirStatus
    {
        DirStatus_None,
        DirStatus_AlreadyExist,
        DirStatus_Created,
        DirStatus_FailedToCreate,
        DirStatus_NameNotSpecified
    };

    enum ProgrammingType
    {
        PT_CPP,
        PT_PYTHON
    };

    int programming_type = PT_CPP; // type of programming language used for the project

    //For Finding the entry point file
    const std::string python_entry_point = "__main__";
    const std::string cpp_entry_point = "int main()";

    std::string build_folder_path, _lib_folder_path;

public:
    enum class TwoStateIconsIndex
    {
        Upload,
        Run,
        Folder,
        Debug,
        RobotArm,
        Search,
        Simulate,
        Max
    };

    enum class SingleStateIconsIndex
    {
        Error_Marker,
        Max
    };

    typedef std::array<TwoStateImageData, (unsigned)TwoStateIconsIndex::Max> TwoStateIconPallete;
    typedef std::array<SingleStateImageData, (unsigned)SingleStateIconsIndex::Max> SingleStateIconPallete;

    CodeEditor(const char* Consolas_Font, 
               const char* DroidSansMono_Font,
               const char* Menlo_Regular_Font,
               const char* MONACO_Font,
               const std::string& lib_folder_path);

    ~CodeEditor();


    void InitializeEditor(const TwoStateIconPallete& two_states_icon);
    void RunEditor();
    void SaveUserData();

    nlohmann::json LoadUserData();
    ImVec4 GetClearColor() const { return clear_color; }
    
    bool ShouldEditorClose() const { return ShouldCloseEditor; }

private:
    
    std::string Recursively_FindEntryPointFile_FromDirectory(const DirectoryNode& parentNode);
    void VerifyCode();
    void Recursively_List_All_CPP_Files(const DirectoryNode& parentNode, std::vector<std::string>& cpp_list);
    void FindBuildFolder();
//=======================================Directory Tree===================================================================================================
    DirStatus CreateProjectDirectory(const fs::path& path, const char* ProjectName, fs::path* out);
    DirStatus CreatesDefaultProjectDirectory(const fs::path& NewProjectPath, const char* ProjectName, fs::path* output_path);
    void RecursivelyDisplayDirectoryNode(DirectoryNode& parentNode);
    void ImplementDirectoryNode();
    void ShowFileExplorer(float top_margin, float bottom_margin);

//=====================================Rendering Editors=====================================================================================================
    void EditorWithoutDockSpace(float main_menubar_height); 
    void DisplayContents(TextEditors::iterator it);
    void RenderTextEditors();
    void LoadEditor(const std::string& file);
    std::tuple<bool, std::string> RenderTextEditorEx(   TextEditors::iterator it, size_t i, 
                                                        ImGuiTabItemFlags flag = ImGuiTabItemFlags_None, bool autosave = false);
    
//=========================================================================================================================================================
    float SetupMenuTab();

    void OpenFileDialog(fs::path& path, const char* key);
    void ProjectWizard();
    void ShowProjectWizard(const char* label);

    void GetRecentlyOpenedProjects();
    void WelcomPage();
    
//===================================================Used for renaming and adding files/folders============================================================
    void RenderArrow(ImGuiDir dir);
    void NodeInputText(std::string& FileName, bool* state, float offsetX, std::function<void(const std::string&)> ptr_to_func, bool IsDirectory = false);
    void NodeInputText(bool* state, float offsetX, std::function<void(const std::string&)> ptr_to_func, bool IsDirectory = false);
    void Show_Find_Replace_Panel();
//=========================================================================================================================================================

    template<class T> void SafeDelete(T*& pVal);
    template<class T> void SafeDeleteArray(T*& pVal);

    bool ShouldShowWelcomePage();
    bool ButtonWithIconEx(const char* label, const char* icon = nullptr, const char* definition = nullptr);
    bool ButtonWithIcon(const char* label, const char* icon, const char* definition);
    bool IsRootKeyExist(const std::string& root, const std::string& path);

    std::wstring Path_To_Wstring(const std::filesystem::path& path);
    int GetTextEditorIndex(const std::string txt_editor_path);

    void FilesHintWithKeys(const std::filesystem::path& path, const Search::Handler_KeyLocation& line);
};


