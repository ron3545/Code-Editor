#define _CRT_SECURE_NO_WARNINGS
#include "CodeEditor.hpp"
#include "imgui/imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <unordered_set>

using namespace std;  
//======================================CLASS DECLARATION================================================================================
static ArmSimPro::ToolBar* vertical_tool_bar   = nullptr;
static ArmSimPro::ToolBar* horizontal_tool_bar = nullptr;

static ArmSimPro::StatusBar* status_bar = nullptr;
static ArmSimPro::CmdPanel* cmd_panel = nullptr;
//========================================================================================================================================
       HWND                     hwnd = NULL;
       ID3D11Device*            g_pd3dDevice = nullptr; //should be non-static. Other translation units will be using this
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
    ::RegisterClassExW(&wc);
    hwnd = ::CreateWindowW(wc.lpszClassName, 
                    SOFTWARE_NAME, 
                    WS_OVERLAPPEDWINDOW, 
                    0, 
                    0, 
                    1280, 
                    800, 
                    nullptr, 
                    nullptr, 
                    hInstance, 
                    nullptr 
                );
    // Make the title bar of the window dark mode           
    {
        BOOL USE_DARK_MODE = true;
        BOOL SET_IMMERSIVE_DARK_MODE_SUCCESS = SUCCEEDED(DwmSetWindowAttribute(hwnd, 
                DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE,
                &USE_DARK_MODE, 
                sizeof(USE_DARK_MODE))
        );
    }

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
    io.IniFilename = NULL;                                      // manage loading/saving by myself
    
    io.Fonts->AddFontDefault();
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

//======================================Load Icons/Images/Fonts==========================================================================================================
    static std::mutex icons_lock;
    auto load_icons = std::async(std::launch::async, [&]()
    {
        std::lock_guard<std::mutex> lock(icons_lock);
        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/ON/Upload.png", &Compile_image.ON_textureID, &Compile_image.width, &Compile_image.height));
        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/OFF/Upload.png", &Compile_image.OFF_textureID));

        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/ON/Verify.png", &Verify_image.ON_textureID, &Verify_image.width, &Verify_image.height));
        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/OFF/Verify.png", &Verify_image.OFF_textureID));

        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/ON/Folder.png", &Folder_image.ON_textureID, &Folder_image.width, &Folder_image.height));
        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/OFF/Folder.png", &Folder_image.OFF_textureID));

        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/ON/Debug.png", &Debug_image.ON_textureID, &Debug_image.width, &Debug_image.height));
        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/OFF/Debug.png", &Debug_image.OFF_textureID));

        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/ON/RobotArm.png", &Robot_image.ON_textureID, &Robot_image.width, &Robot_image.height));
        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/OFF/RobotArm.png", &Robot_image.OFF_textureID));

        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/ON/Search.png", &Search_image.ON_textureID, &Search_image.width, &Search_image.height));
        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/OFF/Search.png", &Search_image.OFF_textureID));

        IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/process-error.png", &ErroSymbol.textureID, &ErroSymbol.width, &ErroSymbol.height));
    });
    load_icons.wait();

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
        
        DefaultFont         = io.Fonts->AddFontFromFileTTF(Consolas_Font     , 14);
        CodeEditorFont      = io.Fonts->AddFontFromFileTTF(DroidSansMono_Font, 24);
        FileTreeFont        = io.Fonts->AddFontFromFileTTF(Menlo_Regular_Font, 24);
        StatusBarFont       = io.Fonts->AddFontFromFileTTF(MONACO_Font       , 11);
        TextFont            = io.Fonts->AddFontFromFileTTF(Menlo_Regular_Font, 18);
    }));
    font.wait();
//================================================================================================================================================================
   
//==================================Initializations===============================================================================================================

    vertical_tool_bar = new ArmSimPro::ToolBar("Vertical", bg_col, 30, ImGuiAxis_Y);
    {
        vertical_tool_bar->AppendTool("Explorer", Folder_image, ImplementDirectoryNode, false, true);                
        vertical_tool_bar->AppendTool("Search", Search_image, SearchOnCodeEditor);                
        vertical_tool_bar->AppendTool("Debug", Debug_image, nullptr);                  
        vertical_tool_bar->AppendTool("Simulate", Robot_image, nullptr);                    
    }

    horizontal_tool_bar = new ArmSimPro::ToolBar("Horizontal", bg_col, 30, ImGuiAxis_X);
    {
        horizontal_tool_bar->AppendTool("verify", Verify_image, nullptr);   horizontal_tool_bar->SetPaddingBefore("verify", 10);
        horizontal_tool_bar->AppendTool("Upload", Compile_image, nullptr);  horizontal_tool_bar->SetPaddingBefore("Upload", 5);
    }

    status_bar = new ArmSimPro::StatusBar("status", 30, horizontal_tool_bar->GetbackgroundColor());
    cmd_panel = new ArmSimPro::CmdPanel("Command Line", status_bar->GetHeight(), bg_col, highlighter_col);

//==================================================Texture for File Dialog==============================================================  
    
    ArmSimPro::FileDialog::Instance().CreateTexture = [](uint8_t* data, int w, int h, char fmt){
        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = w;
        desc.Height = h;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = fmt==0 ? DXGI_FORMAT_B8G8R8A8_UNORM : DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = 0;

        ID3D11Texture2D* pTexture = NULL;
        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = data;
        subResource.SysMemPitch = desc.Width * 4;
        subResource.SysMemSlicePitch = 0;
        g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);

        // Create texture view
        ID3D11ShaderResourceView* out_srv=NULL;
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = desc.MipLevels;
        srvDesc.Texture2D.MostDetailedMip = 0;
        g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &out_srv);
        pTexture->Release();

        return (void*)out_srv;
    };

    ArmSimPro::FileDialog::Instance().DeleteTexture = [](void* tex){
        ID3D11ShaderResourceView* texture = static_cast<ID3D11ShaderResourceView*>(tex);
        texture->Release();
    };

//==================================================================================================================================================================
    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        { 
            // Create a project
            if(!SelectedProjectPath.empty() && project_root_node.FileName.empty() && project_root_node.FullPath.empty()){
                auto task = std::async(std::launch::async, CreateDirectryNodeTreeFromPath, SelectedProjectPath);
                auto status = task.wait_for(std::chrono::milliseconds(1));
                if(status != std::future_status::timeout)
                {
                    
                }
            }

            ImGui::PushFont(DefaultFont);
                float main_menubar_height;
                if(ImGui::BeginMainMenuBar())
                {   
                    if (ImGui::BeginMenu("File"))
                    {
                        if (ImGui::MenuItem("\tNew Sketch", "CTRL+N")) {}
                        if (ImGui::MenuItem("\tNew file", "CTRL+Alt+Windows+N")) {}
                        if (ImGui::MenuItem("\tNew Window", "CTRL+Shift+N")) {}
                        ImGui::Separator();
                        if (ImGui::MenuItem("\tOpen", "CTRL+O")) 
                        {} 
                        if (ImGui::MenuItem("\tClose", "CTRL+W")) {}

                        ImGui::MenuItem("\tAuto Save", "", &auto_save);
                        if (auto_save) {}

                        if (ImGui::MenuItem("\tSave", "CTRL+S")) 
                        {
                            //auto textToSave = focused_editor->second.GetText();
                            /// save text....
                        } 

                        if (ImGui::MenuItem("\tSave As", "CTRL+Shift+S")) {} 
                        ImGui::Separator();
                        if (ImGui::MenuItem("\tCut", "CTRL+X")) {}
                        if (ImGui::MenuItem("\tCopy", "CTRL+C")) {}

                        ImGui::Separator();
                        if (ImGui::MenuItem("\tQuit", "CTRL+Q")) 
                            break;
                        
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
                            ArmSimPro::MenuItemData("\tSelect all", nullptr, nullptr, focused_editor != nullptr && IsWindowShowed, [&](){focused_editor->SetSelection(ArmSimPro::TextEditor::Coordinates(), ArmSimPro::TextEditor::Coordinates(focused_editor->GetTotalLines(), 0));})
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
                
                horizontal_tool_bar->SetToolBar(main_menubar_height + 10);
                vertical_tool_bar->SetToolBar(horizontal_tool_bar->GetThickness(), status_bar->GetHeight() + 17);

                status_bar->BeginStatusBar();
                {
                    float width = ImGui::GetWindowWidth();
                    char buffer[255];

                    if(!current_editor.empty())
                    {
                        static ImVec2 textSize; 
                        if(textSize.x == NULL)
                            textSize = ImGui::CalcTextSize(buffer); //this is a bottleneck function. should prevent it from always calculatin
                        ImGui::SetCursorPosX(width - (textSize.x - 650));
                        ImGui::Text(current_editor.c_str());
                    }
                }
                status_bar->EndStatusBar();

                cmd_panel->SetPanel(100, vertical_tool_bar->GetTotalWidth());
            ImGui::PopFont(); //default font

            EditorWithoutDockSpace(main_menubar_height);
        }
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        g_pSwapChain->Present(1, 0);
    }

    ArmSimPro::SaveUserDataTo_ini();
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    SafeDelete<ArmSimPro::ToolBar>(vertical_tool_bar);
    SafeDelete<ArmSimPro::ToolBar>(horizontal_tool_bar);
    SafeDelete<ArmSimPro::CmdPanel>(cmd_panel);
    SafeDelete<ArmSimPro::StatusBar>(status_bar);

    SafeDelete<ImFont>(DefaultFont);
    SafeDelete<ImFont>(CodeEditorFont);
    SafeDelete<ImFont>(FileTreeFont);
    SafeDelete<ImFont>(StatusBarFont);
    SafeDelete<ImFont>(TextFont);
    return 0;
}

void SearchOnCodeEditor()
{   
    static std::string buffer;

    ImGui::Dummy(ImVec2(0.0f, 13.05f));
    ImGui::Dummy(ImVec2(5.0f, 13.05f));
    ImGui::SameLine();

    ImGui::PushFont(TextFont);
    {   
        static bool Show_Replace_InputText = false;
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,255,255,255));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(49,49,49,255));
            if(ImGui::Button(">"))
                Show_Replace_InputText = true;
            ImGui::SetItemTooltip("Toggle Replace");
            ImGui::SameLine();
            ImGui::Dummy(ImVec2(3.0f, 13.05f));
            ImGui::SameLine();
        ImGui::PopStyleColor(2);

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255,255,255,255));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(49,49,49,255));
        ImGui::PushItemWidth(-1);
            ImGui::InputTextWithHint("##Search", "Search", &buffer);
            ImGui::SetItemTooltip("Search");
        ImGui::PopItemWidth();
        ImGui::PopStyleColor(2);
    }
    ImGui::PopFont();
}

//===============================================Tree View of directory Impl=====================================================

void RecursivelyAddDirectoryNodes(DirectoryNode& parentNode, std::filesystem::directory_iterator& directoryIterator)
{   
    for (const std::filesystem::directory_entry& entry : directoryIterator)
	{
		DirectoryNode& childNode = parentNode.Children.emplace_back();
		childNode.FullPath = entry.path().u8string();
		childNode.FileName = entry.path().filename().u8string();
        childNode.Selected = false;
		if (childNode.IsDirectory = entry.is_directory(); childNode.IsDirectory)
			RecursivelyAddDirectoryNodes(childNode, std::filesystem::directory_iterator(entry));
	}
	auto moveDirectoriesToFront = [](const DirectoryNode& a, const DirectoryNode& b) { return (a.IsDirectory > b.IsDirectory); };
	std::sort(parentNode.Children.begin(), parentNode.Children.end(), moveDirectoriesToFront);
}

static std::mutex dir_tree;
DirectoryNode CreateDirectryNodeTreeFromPath(const std::filesystem::path& rootPath)
{   
    std::lock_guard<std::mutex> lock_dir_tree(dir_tree);
    
    DirectoryNode rootNode;
	rootNode.FullPath = rootPath.u8string();
	rootNode.FileName = rootPath.filename().u8string();

	if (rootNode.IsDirectory = std::filesystem::is_directory(rootPath); rootNode.IsDirectory)
        RecursivelyAddDirectoryNodes(rootNode, std::filesystem::directory_iterator(rootPath));

	return rootNode;
}

void RecursivelyDisplayDirectoryNode(DirectoryNode& parentNode)
{   
    static std::set<ImGuiID> selections_storage;
    static ImGuiID selection;
    
    ImGuiWindow* window = ImGui::GetCurrentWindow();

    ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_SpanFullWidth;
    ImGui::PushID(&parentNode);
    
    switch(parentNode.IsDirectory)
    {
    case true: //Node is directory
        {   
            if(project_root_node.FileName == parentNode.FileName && project_root_node.FullPath == parentNode.FullPath)
                node_flags |= ImGuiTreeNodeFlags_DefaultOpen;

            ImGui::PushFont(FileTreeFont);
            bool right_clicked = false;
            if (ImGui::TreeNodeEx(parentNode.FileName.c_str(), node_flags))
            {   
                right_clicked = ImGui::IsItemClicked(ImGuiMouseButton_Right);
                for (DirectoryNode& childNode : parentNode.Children)
                    RecursivelyDisplayDirectoryNode(childNode);
                
                ImGui::TreePop();
            }
            ImGui::PopFont();

            ImGui::PushFont(TextFont);
            if(ImGui::IsItemClicked(ImGuiMouseButton_Right) || right_clicked)
                ImGui::OpenPopup("Edit Folder");

            if(ImGui::BeginPopupContextItem("Edit Folder")) 
            {
                const ArmSimPro::MenuItemData popup_items[] = {
                    ArmSimPro::MenuItemData("\tNew File...\t", nullptr, nullptr, true, nullptr),
                    ArmSimPro::MenuItemData("\tNew Folder...\t", nullptr, nullptr, true, nullptr),
                    ArmSimPro::MenuItemData("\tReveal in File Explorer\t", nullptr, nullptr, true, nullptr),

                    ArmSimPro::MenuItemData("\tCut\t", nullptr, nullptr, true, nullptr),
                    ArmSimPro::MenuItemData("\tCopy\t", nullptr, nullptr, true, nullptr),
                    ArmSimPro::MenuItemData("\tPaste\t", nullptr, nullptr, true, nullptr),
                    ArmSimPro::MenuItemData("\tCopy Relative Path\t", nullptr, nullptr, true, nullptr),

                    ArmSimPro::MenuItemData("\tRename...\t", nullptr, nullptr, true, nullptr),
                    ArmSimPro::MenuItemData("\tDelete\t", nullptr, nullptr, true, nullptr)
                };
                for(int i = 0; i < IM_ARRAYSIZE(popup_items); i++)
                {  
                    if(i == 3 || i == 7){
                        ImGui::Separator();
                        continue;
                    }
                    ArmSimPro::MenuItem(popup_items[i], true);
                }
                ImGui::EndPopup();
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
                        LoadEditor(parentNode.FullPath);
                    }
                }
            }    
            //right click
            else if(ImGui::IsItemClicked(ImGuiMouseButton_Right) && !ImGui::IsItemToggledOpen())
                ImGui::OpenPopup("Edit File");
            
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

static void ImplementDirectoryNode()
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
//=====================================================Code Editor Reletad====================================
void DisplayContents(TextEditors::iterator it)
{
    ImGui::PushFont(CodeEditorFont);
    it->editor.Render();
    ImGui::PopFont();

    if(it->editor.IsTextChanged())
        it->IsModified = true;

    if (it->editor.IsChildWindowFocused())
    {
        char buffer[255];
        selected_window_path = it->editor.GetPath();
        auto cpos = it->editor.GetCursorPosition();

        snprintf(buffer, sizeof(buffer), "Ln %d, Col %-6d %6d lines %s | %s ",
                    cpos.mLine + 1, cpos.mColumn + 1,
                    it->editor.GetTotalLines(),
                    it->editor.GetFileExtension().c_str(),
                    it->editor.GetFileName().c_str());

        current_editor = std::string(buffer);
    }
}

/**
 * Will return boolean and the file path of the window
 * true -> window is open
 * false -> window was closed and saved
*/
static std::mutex opened_editor;
std::tuple<bool, std::string> RenderTextEditorEx(TextEditors::iterator it, size_t i)
{
    std::lock_guard<std::mutex> lock(opened_editor);
    
    if(!it->Open && !it->IsModified) 
        return std::tuple<bool, std::string>(std::make_pair(false, it->editor.GetPath()));

    //automatically save the contents of the window after closing.
    if(!it->Open && auto_save){
        it->SaveChanges();
        return std::tuple<bool, std::string>(std::make_pair(false, it->editor.GetPath()));
    }
    else if(!it->Open && it->IsModified && !auto_save){ // Aims to prevent closing without saving. Only available when auto save is deactivated
        it->Open = true;
        it->DoQueueClose();
    }

    ImGuiTabItemFlags tab_flag = (it->IsModified)? ImGuiTabItemFlags_UnsavedDocument : 0 ;
    bool visible = ImGui::BeginTabItem(std::string(it->editor.GetTitle() + "##" + std::to_string(i)).c_str(), &it->Open, tab_flag);
    //Rendering of the Code Editor
    if (visible)
    {
        DisplayContents(it);
        ImGui::EndTabItem();
    }
    return std::tuple<bool, std::string>(std::make_pair(true, it->editor.GetPath()));;
}

static void RenderTextEditors()
{
    std::vector<std::future<std::tuple<bool, std::string>>> m_futures;
    std::vector<std::string> ToDelete;
    // devide each tabs into chunks and start rendering concurently
    size_t i = 0;
    for(auto it = Opened_TextEditors.begin(); it != Opened_TextEditors.end(); ++it){
        m_futures.push_back(std::async(std::launch::async, RenderTextEditorEx, it, i));
        ++i;
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
                try{
                    auto result = task->get();
                    const bool ShouldClose = !std::get<0>(result); //returns false when it's time to close widget
                    if(ShouldClose)
                        ToDelete.push_back(std::get<1>(result));
                }
                catch(const std::exception& e){
                    //write a logger class for this to log errors
                }
                task = m_futures.erase(task); 
            }
        } // end loop
    }// end loop

    //https://stackoverflow.com/questions/39139341/how-to-efficiently-delete-elements-from-a-vector-given-an-another-vector
    
    //copy, delete, repopulate
    TextEditors tmp;
    std::copy_if(Opened_TextEditors.begin(), Opened_TextEditors.end(), std::back_inserter(tmp),
    [ToDelete](const ArmSimPro::TextEditorState& it){
        return std::find(ToDelete.begin(), ToDelete.end(), it.editor.GetPath()) == ToDelete.end();
    });

}

static void EditorWithoutDockSpace(float main_menubar_height)
{
    static bool show_welcome = true;
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 size, pos;
    {                       
        const float menubar_toolbar_total_thickness = horizontal_tool_bar->GetThickness() + (main_menubar_height + 10);

        pos[ImGuiAxis_X]  = viewport->Pos[ImGuiAxis_X] + vertical_tool_bar->GetTotalWidth() + 20;
        pos[ImGuiAxis_Y]  = viewport->Pos[ImGuiAxis_Y] + menubar_toolbar_total_thickness + 8;

        size[ImGuiAxis_X] = viewport->WorkSize.x - vertical_tool_bar->GetTotalWidth() - 20;
        size[ImGuiAxis_Y] = viewport->WorkSize.y - (cmd_panel->GetCurretnHeight() + status_bar->GetHeight() + 47);
    }

    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSize(size);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse; 

    ImGui::PushStyleColor(ImGuiCol_WindowBg, bg_col.GetCol());
    ImGui::Begin("DockSpace", nullptr, window_flags);
    {
        float width = ImGui::GetWindowWidth() + 10;
        float height = ImGui::GetWindowHeight();
        if(ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_AutoSelectNewTabs))
        {   
            ImGui::PushStyleColor(ImGuiCol_Tab, bg_col.GetCol());
            if(Opened_TextEditors.empty() && project_root_node.FileName.empty() && project_root_node.FullPath.empty() || show_welcome)
            {
                if(ImGui::BeginTabItem(WELCOME_PAGE, &show_welcome)){
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, child_col.GetCol());
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 12.5);
                    ImGui::BeginChild(WELCOME_PAGE, ImVec2(width, height), false, ImGuiWindowFlags_NoDecoration);
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
                show_welcome = false;
                //make sure to have no duplicates      
                // static size_t prev_size = 0;
                // if(Opened_TextEditors.size() != prev_size)
                // {
                //     prev_size = Opened_TextEditors.size();
                //     std::sort(Opened_TextEditors.begin(), Opened_TextEditors.end(), [](const ArmSimPro::TextEditorState& e1, const ArmSimPro::TextEditorState& e2){
                //         return e1.editor.GetPath() < e2.editor.GetPath();
                //     });
                //     auto it = std::unique(Opened_TextEditors.begin(), Opened_TextEditors.end(), [](const ArmSimPro::TextEditorState& e1, const ArmSimPro::TextEditorState& e2){
                //         return e1.editor.GetPath() == e2.editor.GetPath();
                //     });
                //     Opened_TextEditors.erase(it, Opened_TextEditors.end());
                // }
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
    ImGui::PopStyleColor();
    ImGui::End();

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
        size_t close_queue_unsaved_documents = 0;
        for(size_t n = 0; n < close_queue.Size; n++)
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
//================================================================================================================================
bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

#ifndef WM_DPICHANGED
#define WM_DPICHANGED 0x02E0 // From Windows SDK 8.1+ headers
#endif

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);

        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    case WM_DPICHANGED:
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
        {
            //const int dpi = HIWORD(wParam);
            //printf("WM_DPICHANGED to %d (%.0f%%)\n", dpi, (float)dpi / 96.0f * 100.0f);
            const RECT* suggested_rect = (RECT*)lParam;
            ::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
        }
        break;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}