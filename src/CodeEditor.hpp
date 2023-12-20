#pragma once

#include <windows.h>
#include <string>
#include <shlobj.h>
#include <sstream>
#include <d3d11.h>
#include <tchar.h>
#include <dwmapi.h>
#include <filesystem>
#include <cstring>
#include <map>
#include <unordered_map>
#include <string_view>

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

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"

#include "ToolBar/ToolBar.h"
#include "ImageHandler/ImageHandler.h"
#include "StatusBar/StatusBar.h"
#include "Editor/CmdPanel.h"
#include "Editor/TextEditor.h"
#include "FileDialog/FileDialog.h"

#include "IconFontHeaders/IconsCodicons.h"
#include "IconFontHeaders/IconsMaterialDesignIcons.h"

const char* WELCOME_PAGE = "\tWelcome\t";
constexpr wchar_t* SOFTWARE_NAME = L"ArmSim Pro";
const char* LOGO = "";

//=======================================================================================================================================
static const char* Consolas_Font        = "../../../Utils/Fonts/Consolas.ttf";
static const char* DroidSansMono_Font   = "../../../Utils/Fonts/DroidSansMono.ttf";
static const char* Menlo_Regular_Font   = "../../../Utils/Fonts/Menlo-Regular.ttf";
static const char* MONACO_Font          = "../../../Utils/Fonts/MONACO.TTF";  
//=======================================================Variables==========================================================================
static bool auto_save = false;

static std::filesystem::path SelectedProjectPath; 
static std::filesystem::path NewProjectDir; 

static std::string selected_window_path, prev_selected_window_path; // for editing
static std::string current_editor;

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
//==========================================================================================================================================
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
//==================================================TREE VIEW OF DIRECTORY===================================================================
struct DirectoryNode
{
	std::string FullPath;
	std::string FileName;
	std::vector<DirectoryNode> Children;
	bool IsDirectory;
    bool Selected;
};

static DirectoryNode project_root_node;

//std::string Selected_File_Path;          
//std::string Selcted_File_Name;
//std::string Prev_Selected_File_Name;

static void RecursivelyAddDirectoryNodes(DirectoryNode& parentNode, std::filesystem::directory_iterator& directoryIterator);
static DirectoryNode CreateDirectryNodeTreeFromPath(const std::filesystem::path& rootPath);
static void ImplementDirectoryNode();
static void SearchOnCodeEditor();

//void EditorDockSpace(float main_menubar_height); //Under Development
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
const char* DirCreateLog[] = {"None","Project already exist.", "Project Created.", "Failed To create project.", "Project name not specified."};

static DirStatus CreateProjectDirectory(const std::filesystem::path& path, const char* ProjectName, std::filesystem::path* out)
{
    *out = path / ProjectName;
    if(std::filesystem::exists(*out))
        return DirStatus_AlreadyExist;
    
    //Create Directory
    if(std::filesystem::create_directory(*out))
        return DirStatus_Created;
    return DirStatus_FailedToCreate;
}

static DirStatus CreatesDefaultProjectDirectory(const std::filesystem::path& NewProjectPath, const char* ProjectName, std::filesystem::path* output_path)
{
    if(!std::filesystem::exists(NewProjectPath))
    {   
        //Creates "ArmSimPro Projects" folder and then create a named project directory of the user
        if(std::filesystem::create_directory(NewProjectPath))
            return CreateProjectDirectory(NewProjectPath, ProjectName, output_path);
        return DirStatus_FailedToCreate;
    }
    return CreateProjectDirectory(NewProjectPath, ProjectName, output_path);
}

std::string GetFileNameFromPath(const std::string& filePath) {
    // Find the position of the last directory separator
    size_t lastSeparatorPos = filePath.find_last_of("\\/");

    // Check if a separator is found
    if (lastSeparatorPos != std::string::npos) {
        // Extract the substring starting from the position after the separator
        return filePath.substr(lastSeparatorPos + 1);
    }

    // If no separator is found, return the original path
    return filePath;
}

void OpenFileDialog(std::filesystem::path& path, const char* key)
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
            NewProjectDir = std::filesystem::path(getenv("USERPROFILE")) / "Documents" / "ArmSimPro Projects";

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
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
        ImGui::SetItemTooltip(definition);
        ImGui::PopStyleColor();
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


namespace  ArmSimPro
{
    std::unordered_map<std::string, std::vector<std::string>> LoadUserDataFrom_ini()
    {
        std::filesystem::path current_exe_path = std::filesystem::current_path();
        std::string file_path(current_exe_path.u8string() + "\\ArmSim.ini");

        std::ifstream file(file_path);
        if (!file.is_open())
            return std::unordered_map<std::string, std::vector<std::string>>();

        std::unordered_map<std::string, std::vector<std::string>> user_data;
        
        std::string line;
        while(std::getline(file, line))
        {
            if(line.empty())
                continue;
            
            std::string root_project;
            std::vector<std::string> opened_editors;
            int total_editors = 0;

            // Extract Root project
            std::smatch root_project_match;
            if(std::regex_search(line, root_project_match, std::regex("Root Project -> (.+)")));
                root_project = root_project_match[1];
            
            // Extracting Total Editors
            std::smatch total_editors_match;
            if (std::regex_search(line, total_editors_match, std::regex("Total Editors -> (\\d+)")))
                total_editors = std::stoi(total_editors_match[1]);
            
            // Extracting Opened Editors
            std::smatch opened_editors_match;
            if (std::regex_search(line, opened_editors_match, std::regex("Opened_Editors -> \\[ (.+) \\]")))
            {
                std::string opened_editors_str = opened_editors_match[1];
                std::istringstream iss(opened_editors_str);
                std::string path;
                while (std::getline(iss, path, ',')) 
                    opened_editors.push_back(path);
            
            }
            user_data.insert(std::make_pair(root_project, opened_editors));
        }
        return user_data;
    }

    void SaveUserDataTo_ini()
    {
        std::vector<std::string> Opened_editor_file_paths;
        const std::string project_directory = SelectedProjectPath.u8string();

        if(project_directory.empty())
            return;

        for(const auto& editor : Opened_TextEditors)
            if(editor.Open)
                Opened_editor_file_paths.push_back(editor.editor.GetPath());
        
        std::filesystem::path current_exe_path = std::filesystem::current_path();
        std::string file(current_exe_path.u8string() + "\\ArmSim.ini");
        std::ofstream ini_file(file, std::ios::app);
            ini_file << "Root Project -> " << project_directory << std::endl << "Total Editors -> " << Opened_editor_file_paths.size() << std::endl << "Opened_Editors -> [ ";
            
            const size_t size = Opened_editor_file_paths.size();
            
            for(size_t i = 0; i < Opened_editor_file_paths.size(); ++i){
                ini_file << Opened_editor_file_paths[i];

                if(i < Opened_editor_file_paths.size() - 1)
                    ini_file << " , ";
            }
            ini_file << " ]\n\n\n";
        ini_file.close();
    }
};

void SetupPreprocIdentifiers(ArmSimPro::TextEditor::LanguageDefinition& programming_lang, const char* value)
{
    ArmSimPro::TextEditor::Identifier id;
    id.mDeclaration = value;
    programming_lang.mPreprocIdentifiers.insert(std::make_pair(std::string(value), id));
}

void SetupIdentifiers(ArmSimPro::TextEditor::LanguageDefinition& programming_lang, const char* value, const char* idecls)
{
    ArmSimPro::TextEditor::Identifier id;
    id.mDeclaration = std::string(idecls);
    programming_lang.mIdentifiers.insert(std::make_pair(std::string(value), id));
}

static std::mutex LoadEditor_mutex;
void LoadEditor(const std::string& file)
{
    ArmSimPro::TextEditor editor(file,bg_col.GetCol());
    auto programming_lang = ArmSimPro::TextEditor::LanguageDefinition::CPlusPlus();

    for (int i = 0; i < sizeof(ppnames) / sizeof(ppnames[0]); ++i)
        SetupPreprocIdentifiers(programming_lang, ppvalues[i]);
    for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
        SetupIdentifiers(programming_lang, identifiers[i], idecls[i]);

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

    ArmSimPro::TextEditor::Palette palette = editor.GetPalette();
    palette[(int)ArmSimPro::TextEditor::PaletteIndex::Background] = ImGui::ColorConvertFloat4ToU32(child_col.GetCol());
    palette[(int)ArmSimPro::TextEditor::PaletteIndex::Number] = ImGui::ColorConvertFloat4ToU32(RGBA(189, 219, 173, 255).GetCol());
    editor.SetPalette(palette);

    Opened_TextEditors.push_back(ArmSimPro::TextEditorState(editor));
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

        if(ButtonWithIcon("New Project...", ICON_CI_GIT_PULL_REQUEST, "Clone a remote repository to a local folder..."))
        {}

        ImGui::NextColumn();
        {
            ImGui::SetCursorPosY(80);
            ImGui::PushFont(FileTreeFont);
                ImGui::Text("Recent");
            ImGui::PopFont();

            auto user_data = ArmSimPro::LoadUserDataFrom_ini();
            
            if(!user_data.empty())
            {   
                ImGui::SetCursorPosY(120);
                ImGui::PushFont(TextFont);
                for(const auto& data : user_data)
                {
                    ImGui::Dummy(ImVec2(10, 0)); ImGui::SameLine();
                    if(!data.first.empty())
                    {      
                        std::string file_name;
                        size_t lastSeparatorPos = data.first.find_last_of("\\/");
                        if (lastSeparatorPos != std::string::npos)
                            file_name = data.first.substr(lastSeparatorPos + 1);
                        
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(39, 136, 255, 255));
                        ImGui::TextWrapped(file_name.c_str());
                        if(ImGui::IsItemClicked())
                        {
                            SelectedProjectPath = data.first;
                            for(const auto& file : data.second)
                            {
                                
                            }
                        }
                        ImGui::PopStyleColor();

                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 255, 255));
                        ImGui::SetItemTooltip(data.first.c_str());
                        ImGui::PopStyleColor();
                    }
                    ImGui::Dummy(ImVec2(0, 4));
                }
                ImGui::PopFont();
            }
        }
    ImGui::Columns();
}
