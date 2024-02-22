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
#include <future>
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

#include <filesystem>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"

#include <nlohmann/json.hpp>

#include "../ToolBar/ToolBar.h"
#include "../ImageHandler/ImageHandler.h"
#include "../StatusBar/StatusBar.h"
#include "../Editor/CmdPanel.h"
#include "../Editor/TextEditor.h"

#include "../IconFontHeaders/IconsCodicons.h"
#include "../IconFontHeaders/IconsMaterialDesignIcons.h"

#include "../FileDialog/FileHandler.h"
#include "../FileDialog/FileDialog.h"

#include "../Algorithms/Search.h"
#include "Utility.hpp"

#include <unordered_set>
#include <memory>

namespace fs = std::filesystem;

class CodeEditor
{
private:
    const char* m_Consolas_Font;
    const char* m_DroidSansMono_Font;
    const char* m_Menlo_Regular_Font;
    const char* m_MONACO_Font;

    bool auto_save;
    bool UseDefault_Location;
    bool ShouldCloseEditor;

    fs::path SelectedProjectPath; 
    fs::path NewProjectDir; 

    std::string selected_window_path, prev_selected_window_path; // for editing
    std::string current_editor, found_selected_editor;
    std::string Project_Name; 
    
    typedef std::vector<ArmSimPro::TextEditorState> TextEditors;
    TextEditors Opened_TextEditors;  //Storage for all the instances of text editors that has been opened

    const RGBA bg_col = RGBA(24, 24, 24, 255);
    const RGBA highlighter_col = RGBA(0, 120, 212, 255);
    const RGBA child_col = RGBA(31,31,31,255);
    const ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f); 

    std::map<std::filesystem::path, Search::Handler_SearchKeyOnFile> SearchResults;

    ImageData Compile_image;
    ImageData Verify_image;
    ImageData Folder_image;
    ImageData Debug_image;
    ImageData Robot_image;
    ImageData Search_image;
    SingleImageData ErroSymbol; 

    ImFont* DefaultFont;    
    ImFont* CodeEditorFont;
    ImFont* FileTreeFont;
    ImFont* StatusBarFont;
    ImFont* TextFont;   
    ImFont* IMDIFont;
    ImFont* ICFont;

    DirectoryNode project_root_node;

//======================================CLASS DECLARATION================================================================================
    std::unique_ptr< ArmSimPro::ToolBar > vertical_tool_bar;
    std::unique_ptr< ArmSimPro::ToolBar > horizontal_tool_bar;
    std::unique_ptr< ArmSimPro::StatusBar > status_bar;
    std::unique_ptr< ArmSimPro::CmdPanel > cmd_panel;

//====================================CLASS MUTEXES=======================================================================================
    std::mutex LoadEditor_mutex;
    std::mutex opened_editor;
    std::mutex RecentFile_Mutex;
    std::mutex editor_mutex;
    std::mutex icons_lock;
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

public:
    CodeEditor(const char* Consolas_Font, 
               const char* DroidSansMono_Font,
               const char* Menlo_Regular_Font,
               const char* MONACO_Font);

    ~CodeEditor();


    void InitializeEditor();
    void RunEditor();
    void SaveUserData();

    nlohmann::json LoadUserData();
    ImVec4 GetClearColor() const { return clear_color; }
    
    bool ShouldEditorClose() const { return ShouldCloseEditor; }

private:
//=======================================Directory Tree===================================================================================================
    DirStatus CreateProjectDirectory(const fs::path& path, const char* ProjectName, fs::path* out);
    DirStatus CreatesDefaultProjectDirectory(const fs::path& NewProjectPath, const char* ProjectName, fs::path* output_path);
    void RecursivelyDisplayDirectoryNode(DirectoryNode& parentNode);
    void ImplementDirectoryNode();

//=====================================Rendering Editors=====================================================================================================
    void EditorWithoutDockSpace(float main_menubar_height); 
    void DisplayContents(TextEditors::iterator it);
    void RenderTextEditors();
    std::tuple<bool, std::string> RenderTextEditorEx(   TextEditors::iterator it, size_t i, 
                                                        ImGuiTabItemFlags flag = ImGuiTabItemFlags_None, bool autosave = false);
    
//=========================================================================================================================================================
    float SetupMenuTab();

    void SearchOnCodeEditor();
    void DisplayTree(const std::set<std::filesystem::path>& SearchedFiles, const std::string& key);
    void ShowLinesFromSearchedResult(const std::filesystem::path& path);
    void ShowSearchResultTree(const std::filesystem::path& files, const std::string& key);
    void GetAllLinesFromSearchedResult(const std::filesystem::path& path, const std::string& key);

    void OpenFileDialog(fs::path& path, const char* key);
    void ProjectWizard();

    void LoadEditor(const std::string& file);
    void GetRecentlyOpenedProjects();
    void WelcomPage();

//===================================================Used for renaming and adding files/folders============================================================
    void RenderArrow(ImGuiDir dir);
    void NodeInputText(std::string& FileName, bool* state, float offsetX, std::function<void(const std::string&)> ptr_to_func, bool IsDirectory = false);
    void NodeInputText(bool* state, float offsetX, std::function<void(const std::string&)> ptr_to_func, bool IsDirectory = false);
    void Show_Find_Replace_Panel(ImGuiWindowFlags window_flags, float main_menubar_height);
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


