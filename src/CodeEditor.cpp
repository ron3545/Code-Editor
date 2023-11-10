#pragma warning ( disable : 4244 )

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_stdlib.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui_impl_dx11.h"

#include <d3d11.h>
#include <tchar.h>
#include <dwmapi.h>

#include <mutex>
#include <thread>
#include <numeric>
#include <future>

#include <fstream>
#include <streambuf>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>

#include "MenuBar/File.h"
#include "MenuBar/Edit.h"
#include "ToolBar/ToolBar.h"
#include "ImageHandler/ImageHandler.h"
#include "StatusBar/StatusBar.h"
#include "Editor/CmdPanel.h"
#include "Editor/TextEditor.h"

constexpr wchar_t* SOFTWARE_NAME = L"ArmSim Pro";
const char* LOGO = "";
HWND hwnd = NULL;
//=============================================================================================
static const char* Consolas_Font        = "../../../Utils/Fonts/Consolas.ttf";
static const char* DroidSansMono_Font   = "../../../Utils/Fonts/DroidSansMono.ttf";
static const char* Menlo_Regular_Font   = "../../../Utils/Fonts/Menlo-Regular.ttf";
static const char* MONACO_Font          = "../../../Utils/Fonts/MONACO.TTF";

//======================================CLASS DECLARATION======================================
static ArmSimPro::ToolBar* vertical_tool_bar   = nullptr;
static ArmSimPro::ToolBar* horizontal_tool_bar = nullptr;

static ArmSimPro::StatusBar* status_bar = nullptr;
static ArmSimPro::CmdPanel* cmd_panel = nullptr;
//=============================================================================================
       ID3D11Device*            g_pd3dDevice = nullptr; //should be non-static. Other translation units will be using this
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
   
    io.DisplaySize = ImVec2(1280, 720) / io.DisplayFramebufferScale;
    
    ImGui::StyleColorsDark();

    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

//======================================Load Icons/Images/Fonts==========================================================================================================
    static ImageData Compile_image;
    static ImageData Verify_image;
 
    static ImageData Folder_image;
    static ImageData Debug_image;
    static ImageData Robot_image;
    static ImageData Search_image;
    static ImageData Settings_image;

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

    //IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/ON/Settings.png", &Settings_image.ON_textureID, &Settings_image.width, &Settings_image.height));
    //IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/OFF/Settings.png", &Settings_image.OFF_textureID));

    static const float code_editor_font_size = 24;
    static const float GUI_font_size = 14;
    static ImFont* Consolas      = io.Fonts->AddFontFromFileTTF(Consolas_Font     , GUI_font_size); //default
    static ImFont* DroidSansMono = io.Fonts->AddFontFromFileTTF(DroidSansMono_Font, code_editor_font_size);
    static ImFont* Menlo_Regular = io.Fonts->AddFontFromFileTTF(Menlo_Regular_Font, code_editor_font_size);
    static ImFont* MONACO        = io.Fonts->AddFontFromFileTTF(MONACO_Font       , code_editor_font_size);
//================================================================================================================================================================
   
//==================================Initializations===============================================================================================================
    const RGBA bg_col = RGBA(24, 24, 24, 255);
    const RGBA highlighter_col = RGBA(0, 120, 212, 255);
    vertical_tool_bar = new ArmSimPro::ToolBar("Vertical", bg_col, 30, ImGuiAxis_Y);
    {
        vertical_tool_bar->AppendTool("Folder", Folder_image, nullptr);                
        vertical_tool_bar->AppendTool("Search", Search_image, nullptr);                
        vertical_tool_bar->AppendTool("Debug", Debug_image, nullptr);                  
        vertical_tool_bar->AppendTool("Simulate", Robot_image, nullptr);               
        //vertical_tool_bar->AppendTool("Settings", Settings_image, nullptr, true);      
    }

    horizontal_tool_bar = new ArmSimPro::ToolBar("Horizontal", bg_col, 30, ImGuiAxis_X);
    {
        horizontal_tool_bar->AppendTool("verify", Verify_image, nullptr, true);   horizontal_tool_bar->SetPaddingBefore("verify", 10);
        horizontal_tool_bar->AppendTool("Upload", Compile_image, nullptr, true);  horizontal_tool_bar->SetPaddingBefore("Upload", 5);
    }

    status_bar = new ArmSimPro::StatusBar("status", 30, horizontal_tool_bar->GetbackgroundColor());
    cmd_panel = new ArmSimPro::CmdPanel("Command Line", status_bar->GetHeight(), bg_col, highlighter_col);

//==============================================Setting up Code Editor======================================================================================
    ArmSimPro::TextEditor txt_editor = ArmSimPro::TextEditor(bg_col.GetCol());
    ArmSimPro::TextEditor::Palette colors;
    {
        colors[static_cast<unsigned int>(ArmSimPro::TextEditor::PaletteIndex::Background)] = ImGui::GetColorU32(RGBA(31, 31, 31, 255).GetCol());
        colors[static_cast<unsigned int>(ArmSimPro::TextEditor::PaletteIndex::CurrentLineFill)] = ImGui::GetColorU32(RGBA(31, 31, 31, 80).GetCol());
        colors[static_cast<unsigned int>(ArmSimPro::TextEditor::PaletteIndex::CurrentLineFillInactive)] = ImGui::GetColorU32(RGBA(31, 31, 31, 80).GetCol());
        colors[static_cast<unsigned int>(ArmSimPro::TextEditor::PaletteIndex::CurrentLineEdge)] = ImGui::GetColorU32(RGBA(117, 117, 117, 255).GetCol());

    }
    //txt_editor->SetPalette(colors);

    //language selection
    static bool _use_cpp_lang = true; 
    auto programming_lang = (_use_cpp_lang)? ArmSimPro::TextEditor::LanguageDefinition::CPlusPlus() : ArmSimPro::TextEditor::LanguageDefinition::C();

    static const char* ppnames[] = { "NULL", "PM_REMOVE",
		"ZeroMemory", "DXGI_SWAP_EFFECT_DISCARD", "D3D_FEATURE_LEVEL", "D3D_DRIVER_TYPE_HARDWARE", "WINAPI","D3D11_SDK_VERSION", "assert" };
    
    static const char* ppvalues[] = { 
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
    
    for (int i = 0; i < sizeof(ppnames) / sizeof(ppnames[0]); ++i)
	{
		ArmSimPro::TextEditor::Identifier id;
		id.mDeclaration = ppvalues[i];
		programming_lang.mPreprocIdentifiers.insert(std::make_pair(std::string(ppnames[i]), id));
	}

	// set your own identifiers
	static const char* identifiers[] = {
		"HWND", "HRESULT", "LPRESULT","D3D11_RENDER_TARGET_VIEW_DESC", "DXGI_SWAP_CHAIN_DESC","MSG","LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "TextEditor" };
	static const char* idecls[] = 
	{
		"typedef HWND_* HWND", "typedef long HRESULT", "typedef long* LPRESULT", "struct D3D11_RENDER_TARGET_VIEW_DESC", "struct DXGI_SWAP_CHAIN_DESC",
		"typedef tagMSG MSG\n * Message structure","typedef LONG_PTR LRESULT","WPARAM", "LPARAM","UINT","LPVOID",
		"ID3D11Device", "ID3D11DeviceContext", "ID3D11Buffer", "ID3D11Buffer", "ID3D10Blob", "ID3D11VertexShader", "ID3D11InputLayout", "ID3D11Buffer",
		"ID3D10Blob", "ID3D11PixelShader", "ID3D11SamplerState", "ID3D11ShaderResourceView", "ID3D11RasterizerState", "ID3D11BlendState", "ID3D11DepthStencilState",
		"IDXGISwapChain", "ID3D11RenderTargetView", "ID3D11Texture2D", "class TextEditor" };
	for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i)
	{
		ArmSimPro::TextEditor::Identifier id;
		id.mDeclaration = std::string(idecls[i]);
		programming_lang.mIdentifiers.insert(std::make_pair(std::string(identifiers[i]), id));
	}
	txt_editor.SetLanguageDefinition(programming_lang);
	

	// error markers
	//ArmSimPro::TextEditor::ErrorMarkers markers;
	//markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
	//markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
	//txt_editor->SetErrorMarkers(markers);

	// "breakpoint" markers
	//ArmSimPro::TextEditor::Breakpoints bpts;
	//bpts.insert(24);
	//bpts.insert(47);
	//txt_editor->SetBreakpoints(bpts);

	static const char* fileToEdit = "../../../src/Editor/TextEditor.cpp";
    std::string path = fileToEdit;
    std::string result;

    size_t found = path.find_last_of("/");
    if (found != std::string::npos) 
        result = path.substr(found + 1);
    else 
        result = path;

    //read file and display
	{
		std::ifstream t(fileToEdit);
		if (t.good())
		{
			std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
			txt_editor.SetText(str);
		}
	}
    

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
            float main_menubar_height;
            if(ImGui::BeginMainMenuBar())
            {   
                if (ImGui::BeginMenu("File"))
                {
                    if (ImGui::MenuItem("New Sketch", "CTRL+N")) {}
                    if (ImGui::MenuItem("New file", "CTRL+Alt+Windows+N")) {}
                    if (ImGui::MenuItem("New Window", "CTRL+Shift+N")) {}
                    ImGui::Separator();
                    if (ImGui::MenuItem("Open", "CTRL+O")) {} 
                    if (ImGui::MenuItem("Close", "CTRL+W")) {} 
                    if (ImGui::MenuItem("Save", "CTRL+S")) 
                    {
                        auto textToSave = txt_editor.GetText();
                        /// save text....
                    } 

                    if (ImGui::MenuItem("Save As", "CTRL+Shift+S")) {} 
                    ImGui::Separator();
                    if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                    if (ImGui::MenuItem("Copy", "CTRL+C")) {}

                    ImGui::Separator();
                    if (ImGui::MenuItem("Quit", "CTRL+Q")) 
                        break;
                    
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Edit"))
                {   
                    bool ro = txt_editor.IsReadOnly();
                    if (ImGui::MenuItem("Undo", "CTRL+Z", nullptr, !ro && txt_editor.CanUndo())) 
                        txt_editor.Undo();
                    if (ImGui::MenuItem("Redo", "CTRL+Y", nullptr, !ro && txt_editor.CanRedo())) 
                        txt_editor.Redo();
                    ImGui::Separator();

                    if (ImGui::MenuItem("Cut", "CTRL+X", nullptr, !ro && txt_editor.HasSelection())) 
                        txt_editor.Cut();
                    if (ImGui::MenuItem("Copy", "CTRL+C", nullptr, !ro && txt_editor.HasSelection()))
                        txt_editor.Copy();
                    if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && txt_editor.HasSelection())) 
                        txt_editor.Delete();
                    if (ImGui::MenuItem("Paset", "Ctrl+V", nullptr, !ro && ImGui::GetClipboardText() != nullptr)) 
                        txt_editor.Paste();
                    ImGui::Separator();

                    if (ImGui::MenuItem("Select all", nullptr, nullptr))
					    txt_editor.SetSelection(ArmSimPro::TextEditor::Coordinates(), ArmSimPro::TextEditor::Coordinates(txt_editor.GetTotalLines(), 0));

                    ImGui::EndMenu();
                }
                main_menubar_height = ImGui::GetWindowHeight();
                ImGui::EndMainMenuBar();
            }
            auto cpos = txt_editor.GetCursorPosition();

            horizontal_tool_bar->SetToolBar(main_menubar_height + 10);
            vertical_tool_bar->SetToolBar(horizontal_tool_bar->GetThickness(), status_bar->GetHeight() + 18);

            status_bar->BeginStatusBar();
            {
                float width = ImGui::GetWindowWidth();
                char buffer[255];

                snprintf(buffer, sizeof(buffer), "%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, txt_editor.GetTotalLines(),
                            txt_editor.IsOverwrite() ? "Ovr" : "Ins",
                            txt_editor.CanUndo() ? "*" : " ",
                            txt_editor.GetLanguageDefinition().mName.c_str(), fileToEdit
                        );

                static ImVec2 textSize; 
                if(textSize.x == NULL)
                    textSize = ImGui::CalcTextSize(buffer); //this is a bottleneck function. should prevent it from always calculating.

                ImGui::SetCursorPosX(width - (textSize.x + 40));
                ImGui::Text(buffer);
            }
            status_bar->EndStatusBar();

            cmd_panel->SetPanel(100, vertical_tool_bar->GetTotalWidth());

            //Code Editor dockable panel
            {
                ImGuiViewport* viewport = ImGui::GetMainViewport();

                ImVec2 size, pos;
                {                       
                    const float menubar_toolbar_total_thickness = horizontal_tool_bar->GetThickness() + (main_menubar_height + 10);

                    pos[ImGuiAxis_X]  = viewport->Pos[ImGuiAxis_X] + vertical_tool_bar->GetTotalWidth() + 20;
                    pos[ImGuiAxis_Y]  = viewport->Pos[ImGuiAxis_Y] + menubar_toolbar_total_thickness + 8;

                    size[ImGuiAxis_X] = viewport->WorkSize.x - vertical_tool_bar->GetTotalWidth() - 18;
                    size[ImGuiAxis_Y] = viewport->WorkSize.y - (cmd_panel->GetCurretnHeight() + status_bar->GetHeight() + 47);
                }

                ImGui::SetNextWindowPos(pos);
                ImGui::SetNextWindowSize(size);
                ImGui::SetNextWindowViewport(viewport->ID);

                //use the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
                // because it would be confusing to have two docking targets within each others.
                ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
                window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus; 
                
                static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;
                if(dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
                    window_flags |= ImGuiWindowFlags_NoBackground;

                ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 10)); //used to change window titlebar height;
                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
                ImGui::PushStyleColor(ImGuiCol_TitleBgActive, bg_col.GetCol());
                ImGui::PushStyleColor(ImGuiCol_TitleBg, bg_col.GetCol());
                ImGui::Begin("DockSpace", nullptr, window_flags);
                {   
                    
                    ImGuiIO& io = ImGui::GetIO();
                    if(io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
                    {
                        ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
                        const ImVec2 dockspace_size = ImGui::GetContentRegionAvail();
                        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags | ImGuiDockNodeFlags_NoWindowMenuButton);

                        static bool first_run = true;
                        if(first_run)
                        {
                            first_run = false;
                            
                            ImGui::DockBuilderRemoveNode(dockspace_id); // clear any previous layout
                            ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
                            ImGui::DockBuilderSetNodeSize(dockspace_id, dockspace_size);

                            ImGui::DockBuilderDockWindow(result.c_str(), dockspace_id);
                            ImGui::DockBuilderFinish(dockspace_id);
                        }
                    }
                }
                ImGui::PopStyleColor(2);
                ImGui::End();
                ImGui::PopStyleVar(4);

                ImGui::PushFont(DroidSansMono);
                std::async(std::launch::async, [&txt_editor, result](){
                    txt_editor.Render(result.c_str());
                });
                ImGui::PopFont();
            }
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

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    delete vertical_tool_bar;
    delete horizontal_tool_bar;
    delete cmd_panel;
    delete status_bar;

    delete Consolas; 
    delete DroidSansMono;
    delete Menlo_Regular;
    delete MONACO;       

    return 0;
}

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