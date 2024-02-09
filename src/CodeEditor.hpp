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
namespace fs = std::filesystem;

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"
#include <nlohmann/json.hpp>

#include "ToolBar/ToolBar.h"
#include "ImageHandler/ImageHandler.h"
#include "StatusBar/StatusBar.h"
#include "Editor/CmdPanel.h"
#include "Editor/TextEditor.h"

#include "IconFontHeaders/IconsCodicons.h"
#include "IconFontHeaders/IconsMaterialDesignIcons.h"

#include "FileDialog/FileHandler.h"
#include "FileDialog/FileDialog.h"
#include "Utility.hpp"

//#include "Graphics3D/Graphics3D.h"

const char* WELCOME_PAGE = "\tWelcome\t";
constexpr wchar_t* SOFTWARE_NAME = L"ArmSim Pro";
const char* LOGO = "";

//=======================================================================================================================================
static const char* Consolas_Font        = "../../../Utils/Fonts/Consolas.ttf";
static const char* DroidSansMono_Font   = "../../../Utils/Fonts/DroidSansMono.ttf";
static const char* Menlo_Regular_Font   = "../../../Utils/Fonts/Menlo-Regular.ttf";
static const char* MONACO_Font          = "../../../Utils/Fonts/MONACO.TTF";  
//=======================================================Variables==========================================================================
static bool auto_save = true;

static fs::path SelectedProjectPath; 
static fs::path NewProjectDir; 

static std::string selected_window_path, prev_selected_window_path; // for editing
static std::string current_editor, found_selected_editor;

typedef std::vector<ArmSimPro::TextEditorState> TextEditors;
static TextEditors Opened_TextEditors;  //Storage for all the instances of text editors that has been opened

static std::string Project_Name; 
static bool UseDefault_Location = true;

const RGBA bg_col = RGBA(24, 24, 24, 255);
const RGBA highlighter_col = RGBA(0, 120, 212, 255);
const RGBA child_col(31,31,31,255);

static ImageData Compile_image;
static ImageData Verify_image;

static ImageData Folder_image;
static ImageData Debug_image;
static ImageData Robot_image;
static ImageData Search_image;
static SingleImageData ErroSymbol; 

static ImFont* DefaultFont;     
static ImFont* CodeEditorFont;
static ImFont* FileTreeFont;
static ImFont* StatusBarFont;
static ImFont* TextFont;   
static ImFont* IMDIFont;
static ImFont* ICFont;
//===========================================DIRECTORY TREE=================================================================================
static DirectoryNode project_root_node;
static void ImplementDirectoryNode();

//==========================================================================================================================================
static void SearchOnCodeEditor();

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
void EditorWithoutDockSpace(float main_menubar_height); 

const char* ppnames[] = { "NULL", "PM_REMOVE",
    "ZeroMemory", "DXGI_SWAP_EFFECT_DISCARD", "D3D_FEATURE_LEVEL", "D3D_DRIVER_TYPE_HARDWARE", "WINAPI","D3D11_SDK_VERSION", "assert" };

const char* ppvalues[] = { 
    "#define NULL ((void*)0)", 
    "#define PM_REMOVE (0x0001)",
    "Microsoft's own memory zapper function\n(which is a macro actually)\nvoid ZeroMemory(\n\t[in] PVOID  Destination,\n\t[in] SIZE_T Length\n); ", 
    "enum DXGI_SWAP_EFFECT::DXGI_SWAP_EFFECT_DISCARD = 0", 
    "enum D3D_FEATURE_LEVEL", 
    "enum D3D_DRIVER_TYPE::D3D_DRIVER_TYPE_HARDWARE  = ( D3D_DRIVER_TYPE_UNKNOWN + 1 )",
    "#define WINAPI __stdcall",
    "#define D3D11_SDK_VERSION (7)",
    " #define assert(expression) (void)(                                                  \n"
    "    (!!(expression)) ||                                                              \n"
    "    (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \n"
    " )"
};

// set your own identifiers
const char* identifiers[] = {
    "HWND", "HRESULT", "LPRESULT","D3D11_RENDER_TARGET_VIEW_DESC", "DXGI_SWAP_CHAIN_DESC","MSG","LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
    "ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
    "ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
    "IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "TextEditor" };
const char* idecls[] = 
{
    "typedef HWND_* HWND", "typedef long HRESULT", "typedef long* LPRESULT", "struct D3D11_RENDER_TARGET_VIEW_DESC", "struct DXGI_SWAP_CHAIN_DESC",
    "typedef tagMSG MSG\n * Message structure","typedef LONG_PTR LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
    "ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
    "ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
    "IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "class TextEditor" };

//===================================================HELPER FUNCTIONS=========================================================================================================
std::string Path_To_String(const std::filesystem::path& path)
{
    return path.u8string();
}

std::wstring Path_To_Wstring(const std::filesystem::path& path)
{
    auto str = path.u8string();
    return std::wstring(str.begin(), str.end());
}

int GetTextEditorIndex(const std::string txt_editor_path)
{
    int index = 0;
    auto iterator = std::find(Opened_TextEditors.cbegin(), Opened_TextEditors.cend(), txt_editor_path);
    if(iterator != Opened_TextEditors.cend())
        index = static_cast<int>(std::distance(Opened_TextEditors.cbegin(), iterator)) + 1;
    else
        index = -1;
    return index;
}

enum DirStatus
{
    DirStatus_None,
    DirStatus_AlreadyExist,
    DirStatus_Created,
    DirStatus_FailedToCreate,
    DirStatus_NameNotSpecified
};

static DirStatus CreateProjectDirectory(const fs::path& path, const char* ProjectName, fs::path* out)
{
    *out = path / ProjectName;
    if(fs::exists(*out))
        return DirStatus_AlreadyExist;
    
    //Create Directory
    if(fs::create_directory(*out))
        return DirStatus_Created;
    return DirStatus_FailedToCreate;
}

static DirStatus CreatesDefaultProjectDirectory(const fs::path& NewProjectPath, const char* ProjectName, fs::path* output_path)
{
    if(!fs::exists(NewProjectPath))
    {   
        //Creates "ArmSimPro Projects" folder and then create a named project directory of the user
        if(fs::create_directory(NewProjectPath))
            return CreateProjectDirectory(NewProjectPath, ProjectName, output_path);
        return DirStatus_FailedToCreate;
    }
    return CreateProjectDirectory(NewProjectPath, ProjectName, output_path);
}

void OpenFileDialog(fs::path& path, const char* key)
{   
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, bg_col.GetCol());
    ImGui::PushFont(TextFont);
    if (ArmSimPro::FileDialog::Instance().IsDone(key)) {
        if (ArmSimPro::FileDialog::Instance().HasResult()) 
            path = ArmSimPro::FileDialog::Instance().GetResult();
        
        ArmSimPro::FileDialog::Instance().Close();
    }
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    std::string_view folder_name = path.filename().u8string();
    std::string_view full_path = path.u8string();
    if(folder_name.empty() && !full_path.empty()){
        ImGui::OpenPopup("Warning Screen");
        path.clear();
    }

    ImGui::SetNextWindowSize(ImVec2(300, 130));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(50.0f, 5.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, bg_col.GetCol());
    ImGui::PushStyleColor(ImGuiCol_TitleBg, bg_col.GetCol());
    if(ImGui::BeginPopupModal("Warning Screen", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
    {  
        ImGui::PushFont(CodeEditorFont);
            ImGui::TextWrapped("Selected folder Invalid");
            ImGui::Dummy(ImVec2(0, 3));
            ImGui::Separator();
            ImGui::Dummy(ImVec2(0, 3));
            if(ImGui::Button("Ok", ImVec2(200, 27)) || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
                ImGui::CloseCurrentPopup();
        ImGui::PopFont();
        ImGui::EndPopup();
    }
    ImGui::PopStyleColor(2);
    ImGui::PopStyleVar();
    ImGui::PopFont();
}

static bool ShouldShowWelcomePage()
{
    return Opened_TextEditors.empty() && project_root_node.FileName.empty() && project_root_node.FullPath.empty();
}

template<class T> void SafeDelete(T*& pVal)
{
    delete pVal;
    pVal = NULL;
}

template<class T> void SafeDeleteArray(T*& pVal)
{
    delete[] pVal;
    pVal = NULL;
}

namespace ArmSimPro
{
    struct MenuItemData{
        const char *label, *shortcut;
        bool *selected, enable;
        std::function<void()> ToExec;

        MenuItemData() {}
        MenuItemData(const char* Label, const char* Shortcut, bool* Selected, bool isEnable, std::function<void()> ptr_to_func)
            : label(Label), shortcut(Shortcut), selected(Selected), enable(isEnable), ToExec(ptr_to_func)
        {}
    };

    void MenuItem(const MenuItemData& data, bool is_func_valid)
    { 
        if(ImGui::MenuItem(data.label, data.shortcut, data.selected, data.enable))
        {
            if(data.ToExec && is_func_valid)
                data.ToExec();
        }
    }
}

void ProjectWizard()
{
    const char* DirCreateLog[] = {"None","Project already exist.", "Project Created.", "Failed To create project.", "Project name not specified."};

    ImGui::PushFont(TextFont);
    ImGui::TextWrapped("This wizard allows you to create new PlatformIO project. In the last case, you need to uncheck \"Use default location\" and specify path to chosen directory");
        static DirStatus DirCreateStatus = DirStatus_None;

        if(DirCreateStatus == DirStatus_AlreadyExist || DirCreateStatus == DirStatus_FailedToCreate || DirCreateStatus == DirStatus_NameNotSpecified){
            ImGui::SetCursorPos(ImVec2(185, 110));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255,0,0,255));
            ImGui::Text(DirCreateLog[DirCreateStatus]);
            ImGui::PopStyleColor();
        }
        ImGui::SetCursorPos(ImVec2(60, 130));
        ImGui::Text("Project Name:"); ImGui::SameLine();
        ImGui::InputText("##Project Name", &Project_Name);

        ImGui::SetCursorPos(ImVec2(97, 180));
        ImGui::Text("Location:"); ImGui::SameLine();

        std::string Project_FullPath= NewProjectDir.u8string();
        ImGui::InputText("##Location", &Project_FullPath, ImGuiInputTextFlags_ReadOnly);

        if(ImGui::IsItemClicked() && !UseDefault_Location)
            ArmSimPro::FileDialog::Instance().Open("SelectProjectDirectory", "Select new project directory", "");

        OpenFileDialog(NewProjectDir, "SelectProjectDirectory");

        ImGui::Checkbox("Use default Location", &UseDefault_Location);
        if(UseDefault_Location)
            NewProjectDir = fs::path(getenv("USERPROFILE")) / "Documents" / "ArmSimPro Projects";

        ImGui::SetCursorPosY(240);
        ImGui::Separator();
        ImGui::Spacing();

        ImGui::Dummy(ImVec2(0, 10));
        ImGui::SetCursorPosX(500);
        float ok_cancel_width = 24 * 7;
        if(ImGui::Button("Cancel", ImVec2(ok_cancel_width / 2 - ImGui::GetStyle().ItemSpacing.x, 0.0f)) || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
            ImGui::CloseCurrentPopup();
         
        ImGui::SameLine();
        ImGui::Dummy(ImVec2(5, 0)); ImGui::SameLine();
        if(ImGui::Button("Finish", ImVec2(ok_cancel_width / 2 - ImGui::GetStyle().ItemSpacing.x, 0.0f)) || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
        {
            if(Project_Name.empty())
                DirCreateStatus = DirStatus_NameNotSpecified;

            if(UseDefault_Location && !Project_Name.empty())
            {   
                if((DirCreateStatus = CreatesDefaultProjectDirectory(NewProjectDir, Project_Name.c_str(), &SelectedProjectPath)) == DirStatus_Created)
                    ImGui::CloseCurrentPopup();
            }
            else if(!Project_Name.empty())
                if((DirCreateStatus = CreateProjectDirectory(NewProjectDir, Project_Name.c_str(), &SelectedProjectPath)) == DirStatus_Created)
                    ImGui::CloseCurrentPopup();
        }
    ImGui::PopFont();
}

bool ButtonWithIconEx(const char* label, const char* icon = nullptr, const char* definition = nullptr)
{   

    ImVec2 pos = ImGui::GetCursorPos();
    
    if(icon != nullptr)
    {
        ImGui::SetCursorPosX(64);
        ImGui::Text(icon);
    }
    pos.y -= 12;
    ImGui::SetCursorPos(ImVec2(95.64, pos.y));
    ImGui::PushFont(TextFont);
        bool clicked = ImGui::Button(label);
    ImGui::PopFont();

    if(definition != nullptr)
    {
        ImGui::PushFont(TextFont);
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
        ImGui::SetItemTooltip(definition);
        ImGui::PopStyleColor();
        ImGui::PopFont();
    }

    ImGui::Dummy(ImVec2(0, 45));
    return clicked;
}

bool ButtonWithIcon(const char* label, const char* icon, const char* definition)
{
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(39, 136, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Button,  IM_COL32(0, 0, 0, 0));
        bool clicked = ButtonWithIconEx(label, icon, definition);
    ImGui::PopStyleColor(4);
    return clicked;
}

//========================================Loading and Saving User Data Helper Functions==============================================
namespace  ArmSimPro
{
    nlohmann::json LoadUserData()
    {
        fs::path current_exe_path = fs::current_path();
        std::string file_path(current_exe_path.u8string() + "\\ArmSim.json");
        nlohmann::json data;
        
        std::ifstream config_file(file_path);
            data = nlohmann::json::parse(config_file);
        config_file.close();

        return data;
    }

    bool IsRootKeyExist(const std::string& root, const std::string& path)
    {
        std::ifstream config_file(path);
        auto json_data = nlohmann::json::parse(config_file);
        config_file.close();

        if(json_data.contains(RECENT_FILES) && json_data[RECENT_FILES].is_array())
        {
            for(const auto& element : json_data[RECENT_FILES])
                if(element[ROOT_PROJECT] == root)
                    return true;
            return false;
        }
        return false;
    }

    void SaveUserData()
    {
        fs::path current_exe_path = fs::current_path();
        std::string file_path(current_exe_path.u8string() + "\\ArmSim.json");
        
        if(SelectedProjectPath.empty() && Opened_TextEditors.empty())
            return;

        std::vector<std::string> FilePath_OpenedEditors;
        for(auto it = Opened_TextEditors.cbegin(); it != Opened_TextEditors.cend(); ++it)
            if(it->Open)
                FilePath_OpenedEditors.push_back(it->editor.GetPath());

        nlohmann::json json_data; 

        nlohmann::json new_data;
        new_data[OPENED_FILES] = FilePath_OpenedEditors;
        new_data[ROOT_PROJECT] = SelectedProjectPath.u8string();

        bool file_exist = fs::exists(fs::path(file_path));
        if(file_exist)
        {
            std::ifstream config_file(file_path);
            json_data = nlohmann::json::parse(config_file);
            config_file.close();
            
            if(IsRootKeyExist(SelectedProjectPath.u8string(), file_path))
            {
                for(auto& element : json_data[RECENT_FILES])
                {
                    if(element[ROOT_PROJECT] == SelectedProjectPath.u8string())
                    {
                        element[OPENED_FILES] = FilePath_OpenedEditors;
                        break;
                    }
                }
            }
            else
                json_data[RECENT_FILES].push_back(new_data);
        }
        else
            json_data[RECENT_FILES].push_back(new_data);

        if(json_data.empty())
            return;

        std::ofstream file(file_path, std::ios::trunc);
        file << std::setw(4) << json_data << "\n\n";
        file.close();
    }
};

//==========================================================================================================================================

static std::mutex LoadEditor_mutex;
void LoadEditor(const std::string& file)
{
    std::lock_guard<std::mutex> lock(LoadEditor_mutex);
    ArmSimPro::TextEditor editor(file,bg_col.GetCol());
    auto programming_lang = ArmSimPro::TextEditor::LanguageDefinition::CPlusPlus();

    for (int i = 0; i < sizeof(ppnames) / sizeof(ppnames[0]); ++i)
    {
        ArmSimPro::TextEditor::Identifier id;
        id.mDeclaration = ppvalues[i];
        programming_lang.mPreprocIdentifiers.insert(std::make_pair(std::string(ppvalues[i]), id));
    }  

    for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
    {
        ArmSimPro::TextEditor::Identifier id;
        id.mDeclaration = std::string(idecls[i]);
        programming_lang.mIdentifiers.insert(std::make_pair(std::string(identifiers[i]), id));
    } 

    editor.SetLanguageDefinition(programming_lang);

    std::ifstream t(file.c_str());
    if (t.good())
    {
        std::string str;
        t.seekg(0, std::ios::end);
            str.reserve(t.tellg());
        t.seekg(0, std::ios::beg);
        str.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

        editor.SetText(str);
    }

    // ArmSimPro::TextEditor::Palette palette = editor.GetPalette();
    // palette[(int)ArmSimPro::TextEditor::PaletteIndex::Background] = ImGui::ColorConvertFloat4ToU32(child_col.GetCol());
    // palette[(int)ArmSimPro::TextEditor::PaletteIndex::Number] = ImGui::ColorConvertFloat4ToU32(RGBA(189, 219, 173, 255).GetCol());
    // editor.SetPalette(palette);

    Opened_TextEditors.push_back(ArmSimPro::TextEditorState(editor));
}

void GetRecentlyOpenedProjects()
{
    static std::mutex RecentFile_Mutex;
    std::lock_guard<std::mutex> lock(RecentFile_Mutex);

    typedef std::string Root_Path;
    typedef std::set<std::string> Prev_Files;
    std::map<Root_Path, Prev_Files> Application_data;

    const nlohmann::json data = ArmSimPro::LoadUserData();
    if(data.contains(RECENT_FILES))
    {
        const auto recent_projects = data[RECENT_FILES];
        for(const auto& element : recent_projects)
        {
            const std::string project_path = element[ROOT_PROJECT];
            if(!std::filesystem::exists(project_path))
                continue;

            std::set<std::string> files;
            for(const auto& file : element[OPENED_FILES])
                    files.insert(file);
            
            Application_data[project_path] = files;
        }
    }

    std::string prev_root_file;
    ImGui::SetCursorPosY(120);
    for(const auto& data : Application_data)
    {
        ImGui::PushFont(TextFont);
        {
            if(data.first != prev_root_file && !data.first.empty())
            {
                prev_root_file = data.first;
                ImGui::Dummy(ImVec2(10, 0)); ImGui::SameLine();
                {   
                    std::string file_name;
                    size_t lastSeparatorPos = data.first.find_last_of("\\/");
                    if (lastSeparatorPos != std::string::npos)
                        file_name = data.first.substr(lastSeparatorPos + 1);
                    
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(39, 136, 255, 255));
                    ImGui::TextWrapped(file_name.c_str());
                    ImGui::PopStyleColor();
                    if(ImGui::IsItemClicked())
                    {
                        SelectedProjectPath = data.first;
                        
                        std::vector<std::future<void>> m_futures;
                        for(const auto& file : data.second)
                        {
                            if(fs::exists(file))
                                m_futures.push_back(std::async(std::launch::async, LoadEditor, file));
                        }

                        for(auto& future : m_futures)
                            future.wait();
                    }
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                    ImGui::SetItemTooltip(data.first.c_str());
                    ImGui::PopStyleColor();
                    ImGui::Dummy(ImVec2(0, 11));
                }
            }
        }
        ImGui::PopFont();
    }
}

void WelcomPage()
{  
    // Render Welcome Page
    float window_width = ImGui::GetWindowWidth();
    ImGui::Columns(2, "mycols", false);
        ImGui::SetCursorPos(ImVec2(60,80));
        ImGui::PushFont(FileTreeFont);
            ImGui::Text("Start");
        ImGui::PopFont();             

        ImGui::SetCursorPosY(140);
        if(ButtonWithIcon("New Project...", ICON_CI_ADD, "Create new Platform IO project"))
            ImGui::OpenPopup("Project Wizard");

        bool is_Open;
        ImGui::SetNextWindowSize(ImVec2(700, 300));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(50.0f, 10.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, bg_col.GetCol());
        ImGui::PushStyleColor(ImGuiCol_TitleBg, bg_col.GetCol());
        if(ImGui::BeginPopupModal("Project Wizard", &is_Open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
        {   
            ProjectWizard();
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        if(ButtonWithIcon("Open Project...", ICON_CI_FOLDER_OPENED, "Open a project to start working (Ctrl+O)"))
            ArmSimPro::FileDialog::Instance().Open("SelectProjectDir", "Select project directory", "");
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10.0f, 10.0f));
        OpenFileDialog(SelectedProjectPath, "SelectProjectDir");
        ImGui::PopStyleVar();

        if(ButtonWithIcon("Clone Project...", ICON_CI_GIT_PULL_REQUEST, "Clone a remote repository to a local folder..."))
        {
            // Not a priority for now
        }

        ImGui::NextColumn();
        {
            ImGui::SetCursorPosY(80);
            ImGui::PushFont(FileTreeFont);
                ImGui::Text("Recent");
            ImGui::PopFont();
            
            auto future = std::async(std::launch::async, GetRecentlyOpenedProjects);
            future.wait();
        }
    ImGui::Columns();
}

//===================================================Used for renaming and adding files/folders============================================================
void RenderArrow(ImGuiDir dir)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;
    
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const ImVec2 label_size = ImGui::CalcTextSize(" ", NULL, false);
    const ImVec2 padding = style.FramePadding;//ImVec2(style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));
    const float frame_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2), label_size.y + padding.y * 2);

    const float text_offset_x = g.FontSize + (padding.x);           // Collapser arrow width + Spacing
    const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);                    // Latch before ItemSize changes it
    const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);
    ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);

    ImGui::ItemSize(ImVec2(text_width, frame_height), padding.y);
    ImGui::RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y), ImGui::GetColorU32(ImVec4(1,1,1,1)), dir, 0.70f);
}

void NodeInputText(std::string& FileName, bool* state, float offsetX, std::function<void(const std::string&)> ptr_to_func, bool IsDirectory = false)
{
    if(!ptr_to_func)
        return;
    
    //Just for visual clarification only for the user to determin that he/she is creating a directory node
    if(IsDirectory)
        RenderArrow(ImGuiDir_Right);
    
    ImGui::SameLine();
    ImGui::Indent(offsetX);
    {
        std::string buffer;
        if(!FileName.empty())
            buffer = FileName;
    
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5,0));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_col.GetCol());
        ImGui::PushStyleColor(ImGuiCol_Border, RGBA(0, 120, 212, 255).GetCol());

        ImGui::PushItemWidth(ImGui::GetWindowSize().x - (offsetX + 60));
        if(ImGui::InputText("###input", &buffer,   ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
        {
            if(!buffer.empty())
                ptr_to_func(buffer);
            FileName = buffer;
            *state = false;
        }

        // Close the input text when there is any activities outside the input text box
        if (!ImGui::IsItemHovered() && !ImGui::IsItemActive() &&
            (ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))) 
            *state = false;

        ImGui::PopItemWidth();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    }
    ImGui::Unindent(offsetX);
}

void NodeInputText(bool* state, float offsetX, std::function<void(const std::string&)> ptr_to_func, bool IsDirectory = false)
{
   if(!ptr_to_func)
        return;
    
    //Just for visual clarification only for the user to determin that he/she is creating a directory node
    if(IsDirectory)
        RenderArrow(ImGuiDir_Right);
    
    ImGui::SameLine();
    ImGui::Indent(offsetX);
    {
        std::string buffer;
    
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5,0));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_col.GetCol());
        ImGui::PushStyleColor(ImGuiCol_Border, RGBA(0, 120, 212, 255).GetCol());

        ImGui::PushItemWidth(ImGui::GetWindowSize().x - (offsetX + 60));
        if(ImGui::InputText("###input", &buffer,   ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
        {  
            if(!buffer.empty())
                ptr_to_func(buffer);
            *state = false;
        }

        // Close the input text when there is any activities outside the input text box
        if (!ImGui::IsItemHovered() && !ImGui::IsItemActive() &&
            (ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))) 
            *state = false;

        ImGui::PopItemWidth();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(2);
    }
    ImGui::Unindent(offsetX);
}

void Show_Find_Replace_Panel(ImGuiWindowFlags window_flags, float main_menubar_height)
{   
    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);

    ImGuiIO& io = ImGui::GetIO();
    auto ctrl = io.KeyCtrl;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    static bool show_find_replace_panel = false;

    if(Opened_TextEditors.empty())
        return;

    static ArmSimPro::TextEditor * focused_editor = nullptr;
    if((selected_window_path != prev_selected_window_path)){
        prev_selected_window_path = selected_window_path;
        auto iterator = std::find(Opened_TextEditors.begin(), Opened_TextEditors.end(), selected_window_path); 
        if(iterator != Opened_TextEditors.cend())
            focused_editor = &(iterator->editor);
    }

    if(ctrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F)) && focused_editor != nullptr && focused_editor->IsEditorFocused())
        show_find_replace_panel = true;
    
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
        show_find_replace_panel = false;

    if(show_find_replace_panel)
    {   
        static unsigned int panel_height = 40;
        const float PanelWidth = 466;
        ImVec2 size,panel_pos;

        {
            ImGuiViewportP* viewportp = (ImGuiViewportP*)(void*)(viewport);
            ImRect available_rect = viewportp->GetBuildWorkRect();

            panel_pos = available_rect.Min;
            panel_pos[ImGuiAxis_Y] = available_rect.Max[ImGuiAxis_Y] - (panel_height + main_menubar_height);
            panel_pos[ImGuiAxis_X] = available_rect.Max[ImGuiAxis_X] - (PanelWidth + 30);

            size = available_rect.GetSize();
            size[ImGuiAxis_Y] = panel_height;
        }

        ImGui::SetNextWindowSize(ImVec2(PanelWidth, panel_height));
        ImGui::SetNextWindowPos(panel_pos, ImGuiCond_Always);

        ImGui::PushStyleColor(ImGuiCol_WindowBg, bg_col.GetCol());
        ImGui::Begin("Search and Replace", NULL, window_flags);
        {
            static bool isPressed = false;
            static std::string buffer;

            ImGui::PushFont(TextFont);
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,255,255,255));
                ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(49,49,49,255));
                ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(24, 24, 24,255));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(24, 24, 24,255));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(24, 24, 24,255));

                if(ImGui::ArrowButton("##drop_down", (isPressed)? ImGuiDir_Down : ImGuiDir_Right))
                    isPressed = !isPressed;
                
                ImGui::SameLine();

                ImGui::PushItemWidth(260);
                if(ImGui::InputTextWithHint("##Search", "Search Word on Files", &buffer, ImGuiInputTextFlags_EnterReturnsTrue) && !SelectedProjectPath.empty())
                {

                }
                ImGui::PopItemWidth();
                ImGui::PopStyleColor(5);

                ImGui::SameLine();
                ImGui::PushFont(DefaultFont);
                ImGui::Text("No results");
                ImGui::PopFont();

                ImGui::SameLine();
                if(ImGui::ArrowButton("##move uo", ImGuiDir_Up))
                {

                }

                ImGui::SameLine();
                if(ImGui::ArrowButton("##move down", ImGuiDir_Down))
                {

                }

                if(isPressed)
                {
                    panel_height = 70;
                    const int indent = 32;
                    ImGui::Indent(indent);
                    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,255,255,255));
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(49,49,49,255));
                    if(ImGui::InputTextWithHint("##Replace", "Replace Word on Files", &buffer, ImGuiInputTextFlags_EnterReturnsTrue) && !SelectedProjectPath.empty())
                    {
                        
                    }
                    ImGui::PopStyleColor(2);
                    ImGui::Unindent(indent);
                }
                else
                    panel_height = 40;

                
            ImGui::PopFont();
        }
        ImGui::End();
        ImGui::PopStyleColor();   
    }
}