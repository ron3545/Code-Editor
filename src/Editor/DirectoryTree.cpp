#include "DirectoryTree.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

enum DirStatus
{
    DirStatus_None,
    DirStatus_AlreadyExist,
    DirStatus_Created,
    DirStatus_FailedToCreate,
    DirStatus_NameNotSpecified
};

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

void RecursivelyAddDirectoryNodes(DirectoryNode& parentNode, std::filesystem::directory_iterator directoryIterator)
{
	for (const std::filesystem::directory_entry& entry : directoryIterator)
	{
		DirectoryNode& childNode = parentNode.Children.emplace_back();
		childNode.FullPath = entry.path().u8string();
		childNode.FileName = entry.path().filename().u8string();
		if (childNode.IsDirectory = entry.is_directory(); childNode.IsDirectory)
			RecursivelyAddDirectoryNodes(childNode, std::filesystem::directory_iterator(entry));
	}

	auto moveDirectoriesToFront = [](const DirectoryNode& a, const DirectoryNode& b) { return (a.IsDirectory > b.IsDirectory); };
	std::sort(parentNode.Children.begin(), parentNode.Children.end(), moveDirectoriesToFront);
}

static std::mutex dir_tree;
DirectoryNode CreateDirectryNodeTreeFromPath(const fs::path& rootPath)
{   
    std::lock_guard<std::mutex> lock_dir_tree(dir_tree);
    
    DirectoryNode rootNode;
	rootNode.FullPath = rootPath.u8string();
	rootNode.FileName = rootPath.filename().u8string();

	if (rootNode.IsDirectory = fs::is_directory(rootPath); rootNode.IsDirectory)
        RecursivelyAddDirectoryNodes(rootNode, fs::directory_iterator(rootPath));
	return rootNode;
}

void RecursivelyDisplayDirectoryNode(DirectoryNode& parentNode)
{   
    static std::set<ImGuiID> selections_storage;
    static ImGuiID selection;
    static bool ShouldRename = false;
    
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_SpanFullWidth | ImGuiTreeNodeFlags_AllowOverlap;
    ImGui::PushID(&parentNode);
    
    switch(parentNode.IsDirectory)
    {
    case true: //Node is directory
        {   
            float offsetX = 0.0f; //FOR RENAMING
            static std::string selected_folder;

            if(project_root_node.FileName == parentNode.FileName && project_root_node.FullPath == parentNode.FullPath)
                node_flags |= ImGuiTreeNodeFlags_DefaultOpen;

            ImGui::PushFont(FileTreeFont);
            bool right_clicked = false;
            bool opened = ImGui::TreeNodeEx(parentNode.FileName.c_str(), node_flags);

            ImGui::PushFont(TextFont); 
            if(ImGui::IsItemClicked(ImGuiMouseButton_Right) || right_clicked){
                ShouldRename = false;
                selected_folder = parentNode.FullPath;
                ImGui::OpenPopup("Edit Folder");
            }

            if(ImGui::BeginPopupContextItem("Edit Folder")) 
            {
                bool has_clipText = false;
                auto clipText = ImGui::GetClipboardText();

                const ArmSimPro::MenuItemData popup_items[] = {
                    ArmSimPro::MenuItemData("\tNew File...\t", nullptr, nullptr, true, nullptr),
                    ArmSimPro::MenuItemData("\tNew Folder...\t", nullptr, nullptr, true, nullptr),
                    ArmSimPro::MenuItemData("\tReveal in File Explorer\t", nullptr, nullptr, true, nullptr),

                    ArmSimPro::MenuItemData("\tCut\t", nullptr, nullptr, true, [=](){ FileHandler::GetInstance().CutFile_Folder(parentNode.FullPath); }),
                    ArmSimPro::MenuItemData("\tCopy\t", nullptr, nullptr, true, [=](){ FileHandler::GetInstance().CopyFile_Folder(parentNode.FullPath); }),
                    ArmSimPro::MenuItemData("\tPaste\t", nullptr, nullptr, (clipText != nullptr && strlen(clipText) > 0), [=](){ FileHandler::GetInstance().PasteFile(parentNode.FullPath); }),
                    //ArmSimPro::MenuItemData("\tCopy Relative Path\t", nullptr, nullptr, true, nullptr),

                    ArmSimPro::MenuItemData("\tRename...\t", nullptr, nullptr, true, [&](){ ShouldRename = true; }),
                    ArmSimPro::MenuItemData("\tDelete\t", nullptr, nullptr, true, [](){ FileHandler::GetInstance().DeleteSelectedFolder(selected_folder); })
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

            if((ImGui::IsItemClicked(ImGuiMouseButton_Left) && parentNode.FullPath != selected_folder) || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
                ShouldRename = false;

            // Sowinga renaming widget
            if(ShouldRename && selected_folder == parentNode.FullPath &&  project_root_node.FullPath != selected_folder)
            {
                offsetX = (opened)? ImGui::GetTreeNodeToLabelSpacing() - 20 : ImGui::GetTreeNodeToLabelSpacing();
                ImGui::SameLine();
                ImGui::Indent(offsetX);
                std::string buffer = parentNode.FileName;

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5,0));
                ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1);
                ImGui::PushStyleColor(ImGuiCol_FrameBg, bg_col.GetCol());
                ImGui::PushStyleColor(ImGuiCol_Border, RGBA(0, 120, 212, 255).GetCol());

                ImGui::PushItemWidth(ImGui::GetWindowSize().x - (offsetX + 35));
                if(ImGui::InputText("###rename", &buffer,   ImGuiInputTextFlags_AutoSelectAll    | 
                                                            ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    FileHandler::GetInstance().Rename(parentNode.FullPath, buffer);
                    parentNode.FileName = buffer;
                    ShouldRename = false;
                }
                ImGui::PopItemWidth();
                ImGui::PopStyleColor(2);
                ImGui::PopStyleVar(2);
                ImGui::Unindent(offsetX);
            }
            
            if (opened)
            {   
                right_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
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

            if(pressed_id == selection || selections_storage.find(pressed_id) != selections_storage.end())
                node_flags |= ImGuiTreeNodeFlags_Selected;

            ImGui::PushFont(FileTreeFont);
            ImGui::TreeNodeEx(parentNode.FileName.c_str(), node_flags);
            ImGui::PopFont();
            if(ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
            {   
                ShouldRename = false;
                if(ImGui::GetIO().KeyCtrl)
                    selections_storage.insert(pressed_id);
                else{
                    selection = pressed_id;
                    selections_storage.clear();
                }

                if(ImGui::IsMouseDoubleClicked(0))
                {  
                    auto it = std::find(Opened_TextEditors.cbegin(), Opened_TextEditors.cend(), parentNode.FullPath);
                    if(it == Opened_TextEditors.cend()){
                        auto task = std::async(std::launch::async, LoadEditor, parentNode.FullPath);
                        task.wait();
                    }
                }
            }    
            //right click
            else if(ImGui::IsItemClicked(ImGuiMouseButton_Right) && !ImGui::IsItemToggledOpen()){
                ShouldRename = false;
                ImGui::OpenPopup("Edit File");
            }
            
            ImGui::PushFont(DefaultFont);
            if(ImGui::BeginPopup("Edit File"))
            {
                // s => Seperator
                const char* popup_items[]   = {"\tCut\t", "\tCopy\t", "s", "\tCopy Path\t", "\tCopy Relative Path\t", "s", "\tRename...\t", "\tDelete\t"};
                const char* key_shortcuts[] = {"Ctrl+X", "Ctrl+C", "Shift+Alt+C", "Ctrl+K Ctrl+Shift+C", "F2", "Delete"}; 
                int k = 0;
                for(int i = 0; i < IM_ARRAYSIZE(popup_items); i++)
                {
                    if(strcmp(popup_items[i], "s") == 0){
                        ImGui::Separator();
                        continue;
                    }
                    ImGui::MenuItem(popup_items[i], key_shortcuts[k]);
                    k += 1;
                }
                ImGui::EndPopup();
            }
            ImGui::PopFont();

        } break;
    }
	ImGui::PopID();
}

void ImplementDirectoryNode()
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
        ImGui::TextWrapped("You have not opened a project folder.\n\nYou can open an existing PlatformIO-based project (a folder that contains platformio.ini file).\n\n");
        ImGui::SetCursorPosX(posX);
        if(ImGui::Button("Open Folder", ImVec2(width - 30, 0)))
            ArmSimPro::FileDialog::Instance().Open("SelectProject", "Select project directory", "");
        
        OpenFileDialog(SelectedProjectPath, "SelectProject");
//=========================================================================Create New Project============================================================================================================================================================== 
        
        ImGui::SetCursorPosX(posX);
        ImGui::TextWrapped("\nYou can create a new PlatformIo based Project or explore the examples of ArmSim Kit\n\n");
        ImGui::SetCursorPosX(posX);
        if(ImGui::Button("Create New Project", ImVec2(width - 30, 0)))
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

        ImGui::PopFont();
    }

    if(!project_root_node.FileName.empty() && !project_root_node.FullPath.empty())  
        RecursivelyDisplayDirectoryNode(project_root_node);
}

