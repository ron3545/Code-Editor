#define _CRT_SECURE_NO_WARNINGS

#include "CodeEditor.hpp"
#include "../ImageHandler/ImageHandler.h"
#include <shellapi.h>
#include <string_view>

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

CodeEditor::CodeEditor( const char *Consolas_Font, 
                        const char *DroidSansMono_Font, 
                        const char *Menlo_Regular_Font, 
                        const char *MONACO_Font)
    :   m_Consolas_Font(Consolas_Font),
        m_DroidSansMono_Font(DroidSansMono_Font),
        m_Menlo_Regular_Font(Menlo_Regular_Font),
        m_MONACO_Font(MONACO_Font),
        auto_save(true),
        UseDefault_Location(true),
        ShouldCloseEditor(false)
{

}

CodeEditor::~CodeEditor()
{
    // vertical_tool_bar.reset();
    horizontal_tool_bar.reset();
    cmd_panel.reset();
    status_bar.reset();

    SafeDelete<ImFont>(DefaultFont);
    SafeDelete<ImFont>(CodeEditorFont);
    SafeDelete<ImFont>(FileTreeFont);
    SafeDelete<ImFont>(StatusBarFont);
    SafeDelete<ImFont>(TextFont);
}

void CodeEditor::InitializeEditor(const TwoStateIconPallete& two_states_icon)
{   
    ImGuiIO& io = ImGui::GetIO(); 
    
    float iconFontSize = 24; 
    static const ImWchar icons_ranges_CI[] = { ICON_MIN_CI, ICON_MAX_CI, 0 };
    static const ImWchar icons_ranges_MDI[] = { ICON_MIN_MDI, ICON_MAX_MDI, 0 };

    ImFontConfig icons_config; 
    icons_config.MergeMode = true; 
    icons_config.GlyphMinAdvanceX = iconFontSize;

    static std::mutex font_lock;
    auto font = std::async(std::launch::async, ([&]()
    {
        std::lock_guard<std::mutex> lock(font_lock);
        ICFont = io.Fonts->AddFontFromFileTTF( FONT_ICON_FILE_NAME_CI, iconFontSize, &icons_config, icons_ranges_CI );
        IMDIFont = io.Fonts->AddFontFromFileTTF( FONT_ICON_FILE_NAME_MDI, iconFontSize, &icons_config, icons_ranges_MDI);
        
        DefaultFont         = io.Fonts->AddFontFromFileTTF(m_Consolas_Font     , 14);
        CodeEditorFont      = io.Fonts->AddFontFromFileTTF(m_DroidSansMono_Font, 24);
        FileTreeFont        = io.Fonts->AddFontFromFileTTF(m_Menlo_Regular_Font, 24);
        StatusBarFont       = io.Fonts->AddFontFromFileTTF(m_MONACO_Font       , 11);
        TextFont            = io.Fonts->AddFontFromFileTTF(m_Menlo_Regular_Font, 18);
    }));
    font.wait();

//==================================Initializations===============================================================================================================
    FileHandler::GetInstance().SetFont(TextFont);

    horizontal_tool_bar = std::make_unique< ArmSimPro::ToolBar >("Horizontal", bg_col, 30, ImGuiAxis_X);
    {   
        horizontal_tool_bar->AppendTool("verify", two_states_icon[(int)TwoStateIconsIndex::Verify], [&](){  }, true);   horizontal_tool_bar->SetPaddingBefore("verify", 10);
        horizontal_tool_bar->AppendTool("Upload", two_states_icon[(int)TwoStateIconsIndex::Upload], [&](){  }, true);  horizontal_tool_bar->SetPaddingBefore("Upload", 5);
    }

    status_bar = std::make_unique< ArmSimPro::StatusBar >("status", 30, horizontal_tool_bar->GetbackgroundColor());
    cmd_panel = std::make_unique< ArmSimPro::CmdPanel >("Command Line", status_bar->GetHeight(), bg_col, highlighter_col);
}
//====================================================================================================================================================================================

float CodeEditor::SetupMenuTab()
{
    float main_menubar_height;
    if(ImGui::BeginMainMenuBar())
    {   
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("\tNew Window", "CTRL+Shift+N")) 
                ShellExecute(NULL, L"open", Path_To_Wstring(std::filesystem::current_path() / "ARMSIMPRO_core.exe").c_str(), NULL, NULL, SW_SHOWDEFAULT);
            
            ImGui::Separator();
            ImGui::MenuItem("\tAuto Save", "", &auto_save);
            if (ImGui::MenuItem("\tQuit", "CTRL+Q")){
                SaveUserData();
                ShouldCloseEditor = true;
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit"))
        {   
            static ArmSimPro::TextEditor *focused_editor = nullptr;
            if((selected_window_path != prev_selected_window_path) && !Opened_TextEditors.empty()){
                prev_selected_window_path = selected_window_path;
                auto iterator = std::find(Opened_TextEditors.begin(), Opened_TextEditors.end(), selected_window_path); 
                if(iterator != Opened_TextEditors.cend())
                    focused_editor = &(iterator->editor);
            }
            
            bool IsWindowShowed = (focused_editor != nullptr)? true : false;
            bool NoEditor_Selected = Opened_TextEditors.empty() || (focused_editor != nullptr && IsWindowShowed)? true : false;
            bool ro = (focused_editor != nullptr && IsWindowShowed)? focused_editor->IsReadOnly() : true;

            ArmSimPro::MenuItemData menu_item_arr[] = {
                ArmSimPro::MenuItemData("\tUndo", "CTRL+Z", nullptr, (!ro && (focused_editor != nullptr && IsWindowShowed)? focused_editor->CanUndo() : false), [&](){focused_editor->Undo();}),
                ArmSimPro::MenuItemData("\tRedo", "CTRL+Y", nullptr, (!ro && (focused_editor != nullptr && IsWindowShowed)? focused_editor->CanRedo() : false), [&](){focused_editor->Redo();}),
                //, seperator here
                ArmSimPro::MenuItemData("\tCut", "CTRL+X", nullptr, (!ro && (focused_editor != nullptr && IsWindowShowed)? focused_editor->HasSelection() : false), [&](){focused_editor->Cut();}),
                ArmSimPro::MenuItemData("\tCopy", "CTRL+C", nullptr, (!ro && (focused_editor != nullptr && IsWindowShowed)?  focused_editor->HasSelection() : false), [&](){focused_editor->Copy();}),
                ArmSimPro::MenuItemData("\tDelete", "Del", nullptr, (!ro && (focused_editor != nullptr && IsWindowShowed)? focused_editor->HasSelection() : false), [&](){focused_editor->Delete();}),
                ArmSimPro::MenuItemData("\tPaste", "Ctrl+V", nullptr, (!ro && ImGui::GetClipboardText() != nullptr), [&](){focused_editor->Paste();}),
                //,seperator here
                ArmSimPro::MenuItemData("\tSelect all", nullptr, nullptr, IsWindowShowed, [&](){focused_editor->SetSelection(ArmSimPro::Coordinates(), ArmSimPro::Coordinates(focused_editor->GetTotalLines(), 0));})
            };
            
            for(unsigned int i = 0; i < IM_ARRAYSIZE(menu_item_arr); i++)
            {
                ArmSimPro::MenuItem(menu_item_arr[i], true);
                if(i == 1 || i == 5)
                    ImGui::Separator();
            }
            ImGui::EndMenu();
        }

        main_menubar_height = ImGui::GetWindowHeight();
        ImGui::EndMainMenuBar();
    }
    return main_menubar_height;
}


void CodeEditor::RunEditor()
{
    // Create a project
    if(!SelectedProjectPath.empty() && project_root_node.FileName.empty() && project_root_node.FullPath.empty()){
        auto task = std::async(std::launch::async, CreateDirectryNodeTreeFromPath, SelectedProjectPath);
        project_root_node = task.get();
    }

    ImGui::PushFont(DefaultFont);
        
        float main_menubar_height = SetupMenuTab();
        horizontal_tool_bar->SetToolBar(main_menubar_height + 10);
        ShowFileExplorer(horizontal_tool_bar->GetThickness(), status_bar->GetHeight() + 17);
        //vertical_tool_bar->SetToolBar(horizontal_tool_bar->GetThickness(), status_bar->GetHeight() + 17);

//===================================================STATUS BAR==============================================================================================
        status_bar->BeginStatusBar();
        {
            float width = ImGui::GetWindowWidth();
            if(!current_editor.empty())
            {
                static ImVec2 textSize; 
                if(textSize.x == NULL)
                    textSize = ImGui::CalcTextSize(current_editor.c_str());
                ImGui::SetCursorPosX(width - (textSize.x));
                ImGui::Text(current_editor.c_str());
            }
        }
        status_bar->EndStatusBar();
//===========================================================================================================================================================
        const char* username = std::getenv("USERNAME");
        fs::path organizationPath;
        if (username != nullptr)
            organizationPath = "C:\\Users\\" + std::string(username);

        cmd_panel->SetPanel((SelectedProjectPath.empty())? organizationPath : SelectedProjectPath, 100, explorer_panel_width - 20);
        
    ImGui::PopFont(); //default font

    auto future = std::async(std::launch::async, &CodeEditor::EditorWithoutDockSpace, this, main_menubar_height);
    future.wait();
}

void CodeEditor::Build_Run_UserCode()
{
    std::string entry_point_file;
    if(!project_root_node.Children.empty())
        entry_point_file = Recursively_FindEntryPointFile_FromDirectory(project_root_node);
    
    if(entry_point_file.empty())
        return;
    
    const std::string str_path = SelectedProjectPath.u8string();
    switch(programming_type)
    {
        case  PT_CPP:
        {
            
        }
        case PT_PYTHON:
        {

        }
    }
}

std::string CodeEditor::Recursively_FindEntryPointFile_FromDirectory(const DirectoryNode& parentNode)
{
    if(parentNode.IsDirectory)
    {
        for(const DirectoryNode& child_node : parentNode.Children)
            Recursively_FindEntryPointFile_FromDirectory(child_node);
    }
    else
    {
        std::string text;
        std::ifstream file(parentNode.FullPath.c_str());
        if(file.good())
        {   
            file.seekg(0, std::ios::end);
                text.reserve(file.tellg());
            file.seekg(0, std::ios::beg);
            text.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            file.close();
        }

        //Look for the entry point; if found, exit the function
        if(!text.empty())
        {   
            std::string pattern = (programming_type == PT_CPP)? cpp_entry_point : python_entry_point;
            const auto it = std::search(text.begin(), text.end(), std::boyer_moore_searcher(pattern.begin(), pattern.begin()));
            if(it != text.end())
                return parentNode.FullPath;
        }
    }
}

CodeEditor::DirStatus CodeEditor::CreateProjectDirectory(const fs::path &path, const char *ProjectName, fs::path *out)
{
    const fs::path temp = path / ProjectName;
    if(fs::exists(temp))
        return DirStatus_AlreadyExist;
    
    //Create Project Directory
    if(fs::create_directory(temp)){
        *out = temp;
        FileHandler::GetInstance().CreateWholeProjectDirectory(temp);
        return DirStatus_Created;
    }
    
    return DirStatus_FailedToCreate;
}

CodeEditor::DirStatus CodeEditor::CreatesDefaultProjectDirectory(const fs::path& NewProjectPath, const char* ProjectName, fs::path* output_path)
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

void CodeEditor::RecursivelyDisplayDirectoryNode(DirectoryNode& parentNode)
{
    static std::set<ImGuiID> selections_storage;
    static ImGuiID selection;

    static std::string selected_folder;
    static std::string selected_file;
    static std::string selected_path; // For drag and drop or moving folders/files to other directories

    static bool ShouldRename = false;
    static bool ShouldAddNewFolder = false;  //Only available when right cicking on directory tree
    static bool ShouldAddNewFile = false;    //Only available when right cicking on directory tree

    ImGuiWindow* window = ImGui::GetCurrentWindow();

    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowOverlap;
    ImGui::PushID(&parentNode);
    
    switch(parentNode.IsDirectory)
    {
    case true: //Node is directory
        {   
            float offsetX = 0.0f; //FOR Input Text

            if(project_root_node.FileName == parentNode.FileName && project_root_node.FullPath == parentNode.FullPath)
                node_flags |= ImGuiTreeNodeFlags_DefaultOpen;
            
            ImGui::PushFont(FileTreeFont);

            const bool isRootProject = (ShouldAddNewFolder || ShouldAddNewFile) && selected_folder == parentNode.FullPath;
            bool isChildOpen = false;
            if(isRootProject)
                ImGui::SetNextItemOpen(true);  

            bool opened = ImGui::TreeNodeEx(parentNode.FileName.c_str(), node_flags);
            
            if(ImGui::IsItemClicked(ImGuiMouseButton_Left))
                selected_path = parentNode.FullPath;

            ImGui::PushFont(TextFont); 
                if(ImGui::IsItemClicked(ImGuiMouseButton_Right)){
                    ShouldRename = false;
                    ShouldAddNewFolder = false;
                    ShouldAddNewFile = false;

                    selected_file.clear();
                    selected_folder = parentNode.FullPath;
                    ImGui::OpenPopup("Edit Folder");
                }

                if(ImGui::BeginPopupContextItem("Edit Folder")) 
                    {
                        auto clipText = ImGui::GetClipboardText();

                        const ArmSimPro::MenuItemData popup_items[] = {
                            ArmSimPro::MenuItemData("\tNew File...\t", nullptr, nullptr, true, [&](){ ShouldAddNewFile = true; }),
                            ArmSimPro::MenuItemData("\tNew Folder...\t", nullptr, nullptr, true, [&](){ ShouldAddNewFolder = true; }),

                            ArmSimPro::MenuItemData("\tCut\t", nullptr, nullptr, true, [=](){ FileHandler::GetInstance().Cut(project_root_node, selected_folder); }),
                            ArmSimPro::MenuItemData("\tCopy\t", nullptr, nullptr, true, [=](){ FileHandler::GetInstance().Copy(project_root_node, selected_folder); }),
                            ArmSimPro::MenuItemData("\tPaste\t", nullptr, nullptr, (clipText != nullptr && strlen(clipText) > 0), [=](){ FileHandler::GetInstance().Paste(project_root_node, selected_folder, true); }),

                            ArmSimPro::MenuItemData("\tRename...\t", nullptr, nullptr, true, [&](){ ShouldRename = true; }),
                            ArmSimPro::MenuItemData("\tDelete\t", nullptr, nullptr, true, [=](){ FileHandler::GetInstance().DeleteSelectedFolder(project_root_node, selected_folder); })
                        };

                        for(int i = 0; i < IM_ARRAYSIZE(popup_items); i++)
                        {  
                            if(i == 3 || i == 7)
                                ImGui::Separator();
                
                            ArmSimPro::MenuItem(popup_items[i], true);
                        }
                        ImGui::EndPopup();
                    }
                ImGui::PopFont();

                // renaming widget
                if(ShouldRename && selected_folder == parentNode.FullPath &&  project_root_node.FullPath != selected_folder)
                {
                    offsetX = (opened)? ImGui::GetTreeNodeToLabelSpacing() - 30 : ImGui::GetTreeNodeToLabelSpacing();
                    NodeInputText(parentNode.FileName, &ShouldRename, offsetX, [&](const std::string& buffer){ FileHandler::GetInstance().Rename(parentNode.FullPath, buffer); });
                }
           
//==========================Drag and Drop of Files; For moving files/folders only======================================================
                if (ImGui::BeginDragDropTarget())
                {
                    if(const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_PATH"))
                    {
                        IM_ASSERT(payload->DataSize == sizeof(std::string));
                        const std::string payload_data = *(const std::string*)payload->Data;
                        FileHandler::GetInstance().Paste(project_root_node, payload_data, parentNode.FullPath);
                    }
                    ImGui::EndDragDropTarget();
                }
//======================================================================================================================================

                if (opened)
                {   
                    if(ShouldAddNewFolder && selected_folder == parentNode.FullPath )
                    {
                        ShouldAddNewFile = false;
                        ImGui::Dummy(ImVec2(0,0));
                        auto AddFolder = [&](const std::string& buffer){ FileHandler::GetInstance().CreateNewFolder(project_root_node, selected_folder, buffer.c_str()); };
                        NodeInputText(&ShouldAddNewFolder, ImGui::GetTreeNodeToLabelSpacing(), AddFolder, true);
                    }

                    if(ShouldAddNewFile && selected_folder == parentNode.FullPath )
                    {
                        ShouldAddNewFolder = false;
                        ImGui::Dummy(ImVec2(0,0));
                        auto AddFile = [&](const std::string& buffer){ FileHandler::GetInstance().CreateNewFile(project_root_node, selected_folder, buffer.c_str()); };
                        NodeInputText(&ShouldAddNewFile, ImGui::GetTreeNodeToLabelSpacing(), AddFile, false);
                    }

                    for (DirectoryNode& childNode : parentNode.Children)
                        RecursivelyDisplayDirectoryNode(childNode);

                    ImGui::TreePop();
                }
            ImGui::PopFont();  
        } break;
    
    case false: //Node is a file
        {   
            node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            ImGuiID pressed_id = window->GetID(parentNode.FullPath.c_str());

            if(!ShouldRename && (pressed_id == selection || selections_storage.find(pressed_id) != selections_storage.end()))
                node_flags |= ImGuiTreeNodeFlags_Selected;

            ImGui::PushFont(FileTreeFont);
                ImGui::TreeNodeEx(parentNode.FileName.c_str(), node_flags);
            ImGui::PopFont();
            
            if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {   
                if(ImGui::GetIO().KeyCtrl)
                    selections_storage.insert(pressed_id);
                else{
                    selection = pressed_id;
                    selections_storage.clear();
                }

                if(ImGui::IsMouseDoubleClicked(0)) //Add new tab
                {   
                    found_selected_editor.clear();

                    auto it = std::find(Opened_TextEditors.cbegin(), Opened_TextEditors.cend(), parentNode.FullPath);
                    if(it == Opened_TextEditors.cend())
                    {   //Does not exist. So create one
                        auto task = std::async(std::launch::async, &CodeEditor::LoadEditor, this, parentNode.FullPath);
                        task.wait();
                    }
                }
                else //Find the tab that is already opened and select it
                {
                    auto it = std::find(Opened_TextEditors.cbegin(), Opened_TextEditors.cend(), parentNode.FullPath);
                    if(it != Opened_TextEditors.cend()) //exist 
                        found_selected_editor = parentNode.FullPath;
                }

                selected_path = parentNode.FullPath;
            }    
            //right click
            else if(ImGui::IsItemClicked(ImGuiMouseButton_Right) && !ImGui::IsItemToggledOpen()){
                ShouldRename = false;
                selected_folder.clear();
                selected_file = parentNode.FullPath;
                ImGui::OpenPopup("Edit File");
            }
            
            ImGui::PushFont(TextFont);
                if(ImGui::BeginPopup("Edit File"))
                {
                    auto clipText = ImGui::GetClipboardText();

                    const ArmSimPro::MenuItemData items[] = {
                        ArmSimPro::MenuItemData("\tCut\t", nullptr, nullptr, true, [=](){ FileHandler::GetInstance().Cut(project_root_node, parentNode.FullPath); }),
                        ArmSimPro::MenuItemData("\tCopy\t", nullptr, nullptr, true, [=](){ FileHandler::GetInstance().Copy(project_root_node, parentNode.FullPath); }),
                        ArmSimPro::MenuItemData("\tPaste\t", nullptr, nullptr, (clipText != nullptr && strlen(clipText) > 0), [=](){ FileHandler::GetInstance().Paste(project_root_node, parentNode.FullPath, true); }),

                        ArmSimPro::MenuItemData("\tRename...\t", nullptr, nullptr, true, [&](){ ShouldRename = true; }),
                        ArmSimPro::MenuItemData("\tDelete\t", nullptr, nullptr, true, [&](){ FileHandler::GetInstance().DeleteSelectedFile(project_root_node, parentNode.FullPath); })
                    };

                    for(int i = 0; i < IM_ARRAYSIZE(items); i++)
                    {  
                        if(i == 3 || i == 7)
                            ImGui::Separator();
            
                        ArmSimPro::MenuItem(items[i], true);
                    }
                    ImGui::EndPopup();
                }
            ImGui::PopFont();

            // renaming widget
            if(ShouldRename && selected_file == parentNode.FullPath)
            {
                float offsetX = ImGui::GetTreeNodeToLabelSpacing();
                NodeInputText(parentNode.FileName, &ShouldRename, offsetX, [&](const std::string& buffer){ FileHandler::GetInstance().Rename(parentNode.FullPath, buffer); });
            }
        } break;
    }
	ImGui::PopID();

//==========================Drag and Drop of Files; For moving files/folders only======================================================
    ImGui::PushFont(TextFont);
        if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            std::string file_name;
            size_t lastSeparatorPos = selected_path.find_last_of("\\/");
            if (lastSeparatorPos != std::string::npos) 
                // Extract the substring starting from the position after the separator
                file_name = selected_path.substr(lastSeparatorPos + 1);
            else
                file_name = selected_path;

            ImGui::SetDragDropPayload("DND_PATH", &selected_path, sizeof(std::string));
            ImGui::Text(file_name.c_str());
            ImGui::EndDragDropSource();
        }
    ImGui::PopFont();
}

void CodeEditor::ImplementDirectoryNode()
{
    static bool is_Open = true;
    ImGui::Dummy(ImVec2(0.0f, 13.05f));
    ImGui::Dummy(ImVec2(6.0f, 13.05f));
    ImGui::SameLine();

    float width = ImGui::GetWindowWidth();
    if(SelectedProjectPath.empty())
    {
        static const char* file_dialog_key = nullptr;
        ImGui::PushFont(TextFont);
            const int posX = 14;
            ImGui::SetCursorPosX(posX);
            ImGui::TextWrapped("You have not opened a project folder.\n\nYou can open an existing project.\n\n");
            ImGui::SetCursorPosX(posX);
            if(ImGui::Button("Open Folder", ImVec2(width - 30, 0)))
                ArmSimPro::FileDialog::Instance().Open("SelectProject", "Select project directory", "");
            
            OpenFileDialog(SelectedProjectPath, "SelectProject");
//=========================================================================Create New Project============================================================================================================================================================== 
        
            ImGui::SetCursorPosX(posX);
            ImGui::TextWrapped("\nYou can create a new Project or explore the examples of Robotics Kit\n\n");
            ImGui::SetCursorPosX(posX);
            if(ImGui::Button("Create New Project", ImVec2(width - 30, 0)))
                ImGui::OpenPopup("Project Wizard ##2");

            ShowProjectWizard("Project Wizard ##2");

        ImGui::PopFont();
    }

    if(!project_root_node.FileName.empty() && !project_root_node.FullPath.empty())  
        RecursivelyDisplayDirectoryNode(project_root_node);
}

void CodeEditor::ShowFileExplorer(float top_margin, float bottom_margin)
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    const float explorer_panel_height = viewport->WorkSize.y - ((top_margin + bottom_margin));
    const float explorer_panel_posY = viewport->Pos.y + ((top_margin * 2) + 7);

    ImGui::SetNextWindowSize(ImVec2(explorer_panel_width, explorer_panel_height));
    ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x, explorer_panel_posY));
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, bg_col.GetCol());
    ImGui::Begin("File Explorer", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoDocking);
    {
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        const float splitter_thickness = 6;

        ImGui::PushStyleColor(ImGuiCol_ChildBg, bg_col.GetCol());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        
        ImGui::BeginChild("Explorer", ImVec2(windowSize.x - splitter_thickness, windowSize.y - splitter_thickness), false,   ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove);
            ImplementDirectoryNode();
        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImVec2 splitter_size(splitter_thickness, windowPos[ImGuiAxis_Y] + windowSize[ImGuiAxis_Y]);
        ImVec2 splitter_pos(windowPos.x + (explorer_panel_width - splitter_thickness), 0);

        ImGui::Splitter("Explorer splitter", ImGui::GetColorU32(bg_col.GetCol()),
                        ImGui::GetColorU32(highlighter_col.GetCol()), splitter_size, splitter_pos, 
                        &explorer_panel_width, ImGuiAxis_Y);
        
        ImGuiMouseCursor cursor = ImGuiMouseCursor_Arrow;
        if(ImGui::IsItemActive() || ImGui::IsItemHovered())
            cursor = ImGuiMouseCursor_ResizeEW;
        ImGui::SetMouseCursor(cursor);
        //size limit
        if(explorer_panel_width < 197.33f)
            explorer_panel_width = 197.33f;
        else if(explorer_panel_width > 579.33f)
            explorer_panel_width = 579.33f;
    }
    ImGui::PopStyleColor();
    ImGui::End();
    ImGui::PopStyleVar(2);
}

void CodeEditor::EditorWithoutDockSpace(float main_menubar_height)
{
    std::lock_guard<std::mutex> lock(editor_mutex);
    ImVec2 MainPanelSize;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 size, pos;
    {                       
        const float menubar_toolbar_total_thickness = horizontal_tool_bar->GetThickness() + (main_menubar_height + 10);

        pos[ImGuiAxis_X]  = viewport->Pos[ImGuiAxis_X] + explorer_panel_width;
        pos[ImGuiAxis_Y]  = viewport->Pos[ImGuiAxis_Y] + menubar_toolbar_total_thickness + 8;

        size[ImGuiAxis_X] = viewport->WorkSize.x - explorer_panel_width;
        size[ImGuiAxis_Y] = viewport->WorkSize.y - (cmd_panel->GetCurretnHeight() + status_bar->GetHeight() + 47);
    }

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse; 

    ImGui::PushStyleColor(ImGuiCol_WindowBg, bg_col.GetCol());
    ImGui::PushStyleColor(ImGuiCol_Tab, bg_col.GetCol());
    ImGui::Begin("##Main Panel", nullptr, window_flags);
    {   
        MainPanelSize = ImGui::GetWindowSize();
        float width = ImGui::GetWindowWidth() + 10;
        float height = ImGui::GetWindowHeight();

        if(ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs | ImGuiTabBarFlags_FittingPolicyScroll))
        {   
            ImGui::PushStyleColor(ImGuiCol_Tab, bg_col.GetCol());
            if(Opened_TextEditors.empty() && project_root_node.FileName.empty() && project_root_node.FullPath.empty())
            {
                if(ImGui::BeginTabItem("\tWelcome\t")){
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, bg_col.GetCol());
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 12.5);
                    ImGui::BeginChild("\tWelcome\t", ImVec2(width, height), false, ImGuiWindowFlags_NoDecoration);
                        WelcomPage();
                    ImGui::EndChild();
                    ImGui::PopStyleColor();
                    ImGui::PopStyleVar();

                    ImGui::EndTabItem();
                }
            }
            ImGui::PopStyleColor();

            if(!Opened_TextEditors.empty())
            {
                //make sure to have no duplicates      
                static size_t prev_size = 0;
                if(Opened_TextEditors.size() != prev_size)
                {
                    prev_size = Opened_TextEditors.size();
                    std::sort(Opened_TextEditors.begin(), Opened_TextEditors.end(), [](const ArmSimPro::TextEditorState& e1, const ArmSimPro::TextEditorState& e2){
                        return e1.editor.GetPath() < e2.editor.GetPath();
                    });
                    auto it = std::unique(Opened_TextEditors.begin(), Opened_TextEditors.end(), [](const ArmSimPro::TextEditorState& e1, const ArmSimPro::TextEditorState& e2){
                        return e1.editor.GetPath() == e2.editor.GetPath();
                    });
                    Opened_TextEditors.erase(it, Opened_TextEditors.end());
                }
                
                RenderTextEditors();
            }
            else
            {
                current_editor.clear();
                selected_window_path.clear();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::PopStyleColor(2);
    ImGui::End();

    search_replace_panel_offset = (cmd_panel->GetCurretnHeight() + MainPanelSize.y - horizontal_tool_bar->GetThickness() + 10) - main_menubar_height;
    Show_Find_Replace_Panel();
//=================================================For Reminding to save work===========================================================================================
    if(auto_save)
        return;

    //Update closing Queue
    static ImVector<ArmSimPro::TextEditorState*> close_queue;
    if(close_queue.empty())
    {
        // Close queue us locked once we started popup
        for(auto& editor : Opened_TextEditors)
        {
            if(editor.WantClose)
            {
                editor.WantClose = false;
                close_queue.push_back(&editor);
            }
        }
    }
    // Display a confirmation UI
    if(!close_queue.empty())
    {
        int close_queue_unsaved_documents = 0;
        for(int n = 0; n < close_queue.Size; n++)
        {
            if(close_queue[n]->IsModified)
                close_queue_unsaved_documents++;
            
            if(close_queue_unsaved_documents == 0)
            {
                // Close documents when all are unsaved
                for(int n = 0; n < close_queue.Size; n++)
                    close_queue[n]->DoForceClose();
                close_queue.clear();
            }
            else
            {
                if(!ImGui::IsPopupOpen("ArmSimPro ##Save Work"))
                    ImGui::OpenPopup("ArmSimPro ##Save Work");
                
                // ImGui::SetNextWindowSize(ImVec2(400,200));
                // ImGui::SetNextWindowPos(ImVec2((ImGui::GetWindowWidth()/2) + 200, (ImGui::GetWindowHeight()/2) + 100));
                if(ImGui::BeginPopupModal("ArmSimPro ##Save Work", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse))
                {
                    ImGui::Text("Save changes to the following items?");
                    float item_height = ImGui::GetTextLineHeightWithSpacing();
                    if(ImGui::BeginChildFrame(ImGui::GetID("Frae"), ImVec2(-FLT_MIN, 6.25f * item_height)))
                    {
                        for(int i = 0; i < close_queue.Size; i++)
                            if(close_queue[i]->IsModified)
                                ImGui::Text("%s", close_queue[i]->editor.GetTitle().c_str());
                    }
                    ImGui::EndChildFrame();

                    ImVec2 button_size(ImGui::GetFontSize() * 7.0f, 0.0f);
                    if (ImGui::Button("Save", button_size))
                    {
                        for(int i = 0; i < close_queue.Size; i++)
                        {
                            if(close_queue[i]->IsModified)
                                close_queue[i]->SaveChanges();
                            close_queue[i]->DoForceClose();
                        }
                        close_queue.clear();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Don't Save", button_size))
                    {
                        for(int i = 0; i < close_queue.Size; i++)
                            close_queue[i]->DoForceClose();
                        close_queue.clear();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", button_size))
                    {
                        close_queue.clear();
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                } // End of popup modal
            }
        }
    }
}

void CodeEditor::DisplayContents(TextEditors::iterator it)
{
    ImGui::PushFont(CodeEditorFont);
        it->editor.Render(use_search_panel, to_find, to_replace);
    ImGui::PopFont();

    if(it->editor.IsTextChanged())
        it->IsModified = true;

    if (it->editor.IsEditorFocused())
    {
        char buffer[255];
        selected_window_path = it->editor.GetPath();
        auto cpos = it->editor.GetCursorPosition();

        snprintf(buffer, sizeof(buffer), "Ln %d, Col %-6d %6d lines %s | %s ",
                    cpos.mLine + 1, cpos.mColumn + 1,
                    it->editor.GetTotalLines(),
                    it->editor.GetFileExtension().c_str(),
                    it->editor.GetFileName().c_str());
        
        current_editor = buffer;
    }
}

void CodeEditor::RenderTextEditors()
{
    std::vector<std::future<std::tuple<bool, std::string>>> m_futures;
    std::vector<std::string> ToDelete;
    // devide each tabs into chunks and start rendering concurently
    size_t i = 0;
    for(auto it = Opened_TextEditors.begin(); it != Opened_TextEditors.end();)
    {
        if(fs::exists(it->editor.GetPath()))
        {
            m_futures.push_back(std::async(std::launch::async, &CodeEditor::RenderTextEditorEx, this, it, i, (found_selected_editor == it->editor.GetPath())? ImGuiTabItemFlags_SetSelected : 0, auto_save));
            ++i;
            ++it;
        }
        else
            it = Opened_TextEditors.erase(it);   
    }
    
    // Use non-blocking approach to monitor task completion.
    while(!m_futures.empty())
    {   
        auto task = m_futures.begin();
        while(task != m_futures.end())
        {
            // check the status of each tasks 
            std::future_status task_stat = task->wait_for(std::chrono::milliseconds(5));
            if(task_stat == std::future_status::timeout)
                ++task; 
            else
            {
                auto result = task->get();
                const bool ShouldClose = !std::get<0>(result); //returns false when it's time to close widget
                if(ShouldClose)
                    ToDelete.push_back(std::get<1>(result));
            
                task = m_futures.erase(task); 
            }
        } // end loop
    }// end loop

    //https://stackoverflow.com/questions/39139341/how-to-efficiently-delete-elements-from-a-vector-given-an-another-vector
    //copy, delete, repopulate
    //using erase method causes some weird anomalies. will fix it later.
    if(!ToDelete.empty())
    {
        TextEditors tmp;
        std::copy_if(Opened_TextEditors.begin(), Opened_TextEditors.end(), std::back_inserter(tmp),
        [ToDelete](const ArmSimPro::TextEditorState& it){
            return std::find(ToDelete.begin(), ToDelete.end(), it.editor.GetPath()) == ToDelete.end();
        });

        Opened_TextEditors.clear();
        std::copy(tmp.begin(), tmp.end(),std::back_inserter(Opened_TextEditors));
    }
}

/**
 * Will return boolean and the file path of the window
 * true -> window is open
 * false -> window was closed and saved
*/
std::tuple<bool, std::string> CodeEditor::RenderTextEditorEx( TextEditors::iterator it, size_t i, ImGuiTabItemFlags flag, bool autosave)
{
    std::lock_guard<std::mutex> lock(opened_editor);
    
    if(!it->Open && !it->IsModified) 
        return std::tuple<bool, std::string>(std::make_pair(false, it->editor.GetPath()));

    if(it->IsModified && autosave){
        it->SaveChanges();
        it->IsModified = false;
    }
    else if(!it->Open && it->IsModified && !autosave){ // Aims to prevent closing without saving. Only available when auto save is deactivated
        it->Open = true;
        it->DoQueueClose();
    }

    ImGuiTabItemFlags tab_flag = (it->IsModified)? ImGuiTabItemFlags_UnsavedDocument : 0 ;
    bool visible = ImGui::BeginTabItem(std::string(it->editor.GetTitle() + "##" + std::to_string(i)).c_str(), &it->Open, tab_flag | flag);
    //Rendering of the Code Editor
    if (visible)
    {
        DisplayContents(it);
        ImGui::EndTabItem();
    }
    return std::tuple<bool, std::string>(std::make_pair(true, it->editor.GetPath()));
}

void CodeEditor::OpenFileDialog(fs::path& path, const char* key)
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

        std::string folder_name = path.filename().u8string();
        std::string full_path = path.u8string();
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

void CodeEditor::ProjectWizard()
{
     const char* DirCreateLog[] = {"None","Project already exist.", "Project Created.", "Failed To create project.", "Project name not specified."};

    
    ImGui::TextWrapped("This wizard allows you to create new project. In the last case, you need to uncheck \"Use default location\" and specify path to chosen directory");
        static DirStatus DirCreateStatus = DirStatus_None;

        if(DirCreateStatus == DirStatus_AlreadyExist || DirCreateStatus == DirStatus_FailedToCreate || DirCreateStatus == DirStatus_NameNotSpecified){
            ImGui::SetCursorPos(ImVec2(189, 110));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(255,0,0,255));
            ImGui::Text(DirCreateLog[DirCreateStatus]);
            ImGui::PopStyleColor();
        }
        ImGui::SetCursorPos(ImVec2(60, 130));
        ImGui::Text("Project Name:"); ImGui::SameLine();
        ImGui::InputText("##Project Name", &Project_Name);

        ImGui::SetCursorPosX(190);
        ImGui::RadioButton("Cpp", &programming_type, PT_CPP); ImGui::SameLine(); ImGui::RadioButton("Python", &programming_type, PT_PYTHON);

        ImGui::SetCursorPosX(97);
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

            if(DirCreateStatus != DirStatus_Created && DirCreateStatus != DirStatus_None && DirCreateStatus != DirStatus_NameNotSpecified)
                Project_Name.clear();
        }
    
}

void CodeEditor::ShowProjectWizard(const char *label)
{
    bool is_Open;
    ImGui::PushFont(TextFont);
        ImGui::SetNextWindowSize(ImVec2(700, 300));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(50.0f, 10.0f));
        ImGui::PushStyleColor(ImGuiCol_TitleBgActive, bg_col.GetCol());
        ImGui::PushStyleColor(ImGuiCol_TitleBg, bg_col.GetCol());
        if(ImGui::BeginPopupModal(label, &is_Open, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize))
        {   
            ProjectWizard();
            ImGui::EndPopup();
        }
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();
    ImGui::PopFont();
}

void CodeEditor::LoadEditor(const std::string& file)
{
    std::lock_guard<std::mutex> lock(LoadEditor_mutex);
    ArmSimPro::TextEditor editor(file,bg_col.GetCol(), TextFont, DefaultFont);
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

        t.close();
        editor.SetText(str);
    }

    // Changing color
    // ArmSimPro::TextEditor::Palette palette = editor.GetPalette();
    // palette[(int)ArmSimPro::TextEditor::PaletteIndex::Background] = ImGui::ColorConvertFloat4ToU32(child_col.GetCol());
    // palette[(int)ArmSimPro::TextEditor::PaletteIndex::Number] = ImGui::ColorConvertFloat4ToU32(RGBA(189, 219, 173, 255).GetCol());
    // editor.SetPalette(palette);

    Opened_TextEditors.push_back(ArmSimPro::TextEditorState(editor));
}

void CodeEditor::GetRecentlyOpenedProjects()
{
    std::lock_guard<std::mutex> lock(RecentFile_Mutex);

    typedef std::string Root_Path;
    typedef std::set<std::string> Prev_Files;
    std::map<Root_Path, Prev_Files> Application_data;

    const nlohmann::json data = LoadUserData();
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
                                m_futures.push_back(std::async(std::launch::async, &CodeEditor::LoadEditor, this,file));
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

void CodeEditor::WelcomPage()
{
    // Render Welcome Page
    float window_width = ImGui::GetWindowWidth();
    ImGui::Columns(2, "mycols", false);
        ImGui::SetCursorPos(ImVec2(60,80));

        ImGui::PushFont(FileTreeFont);
            ImGui::Text("Start");
        ImGui::PopFont();             

        ImGui::SetCursorPosY(140);
        if(ButtonWithIcon("New Project...", ICON_CI_ADD, "Create new project"))
            ImGui::OpenPopup("Project Wizard");

        ShowProjectWizard("Project Wizard");

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
            
            auto future = std::async(std::launch::async, &CodeEditor::GetRecentlyOpenedProjects, this);
            future.wait();
        }
    ImGui::Columns();
}

void CodeEditor::RenderArrow(ImGuiDir dir)
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

void CodeEditor::NodeInputText(std::string& FileName, bool* state, float offsetX, std::function<void(const std::string&)> ptr_to_func, bool IsDirectory )
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

void CodeEditor::NodeInputText(bool* state, float offsetX, std::function<void(const std::string&)> ptr_to_func, bool IsDirectory )
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

void CodeEditor::Show_Find_Replace_Panel()
{
    ImGuiIO& io = ImGui::GetIO();
    auto ctrl = io.KeyCtrl;

    ImGuiViewport* viewport = ImGui::GetMainViewport();
    static bool show_find_replace_panel = false;

    if(Opened_TextEditors.empty())
        return;

    if(ctrl && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_F)))
        use_search_panel = true;
    
    if(ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
        use_search_panel = false;
}

template<class T> void CodeEditor::SafeDelete(T*& pVal)
{
    delete pVal;
    pVal = NULL;
}

template<class T> void CodeEditor::SafeDeleteArray(T*& pVal)
{
    delete[] pVal;
    pVal = NULL;
}


bool CodeEditor::ShouldShowWelcomePage()
{
    return Opened_TextEditors.empty() && project_root_node.FileName.empty() && project_root_node.FullPath.empty();
}

bool CodeEditor::ButtonWithIconEx(const char* label, const char* icon, const char* definition)
{
    ImVec2 pos = ImGui::GetCursorPos();
    
    if(icon != nullptr)
    {
        ImGui::SetCursorPosX(64);
        ImGui::Text(icon);
    }
    pos.y -= 12;
    ImGui::SetCursorPos(ImVec2(95.64f, pos.y));
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

bool CodeEditor::ButtonWithIcon(const char* label, const char* icon, const char* definition)
{
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(39, 136, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,  IM_COL32(0, 0, 0, 0));
    ImGui::PushStyleColor(ImGuiCol_Button,  IM_COL32(0, 0, 0, 0));
        bool clicked = ButtonWithIconEx(label, icon, definition);
    ImGui::PopStyleColor(4);
    return clicked;
}


std::wstring CodeEditor::Path_To_Wstring(const std::filesystem::path& path)
{
    auto str = path.u8string();
    return std::wstring(str.begin(), str.end());
}

int CodeEditor::GetTextEditorIndex(const std::string txt_editor_path)
{
    int index = 0;
    auto iterator = std::find(Opened_TextEditors.cbegin(), Opened_TextEditors.cend(), txt_editor_path);
    if(iterator != Opened_TextEditors.cend())
        index = static_cast<int>(std::distance(Opened_TextEditors.cbegin(), iterator)) + 1;
    else
        index = -1;
    return index;
}

void CodeEditor::FilesHintWithKeys(const std::filesystem::path &path, const Search::Handler_KeyLocation& line)
{
    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap;
    node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    //ImGui::TreeNodeEx(line.m_Line.c_str(), node_flags);

}

//========================================Loading and Saving User Data Helper Functions==============================================
nlohmann::json CodeEditor::LoadUserData()
{
    fs::path current_exe_path = fs::current_path();
        std::string file_path(current_exe_path.u8string() + "\\ArmSim.json");
        nlohmann::json data;
        
        std::ifstream config_file(file_path);
            data = nlohmann::json::parse(config_file);
        config_file.close();

        return data;
}

bool CodeEditor::IsRootKeyExist(const std::string& root, const std::string& path)
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

void CodeEditor::SaveUserData()
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
