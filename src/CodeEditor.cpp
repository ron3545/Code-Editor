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

#include "MenuBar/File.h"
#include "MenuBar/Edit.h"
#include "ToolBar/ToolBar.h"
#include "ImageHandler/ImageHandler.h"
#include "StatusBar/StatusBar.h"

constexpr wchar_t* SOFTWARE_NAME = L"ArmSim Pro";
const char* LOGO = "";
 HWND hwnd = NULL;
//======================================MENU ITEM==============================================
ImGui::ToolBar* vertical_tool_bar   = nullptr;
ImGui::ToolBar* horizontal_tool_bar = nullptr;

ImGui::StatusBar* status_bar = nullptr;
//=============================================================================================
ID3D11Device*                   g_pd3dDevice = nullptr;
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
                    100, 
                    100, 
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
    io.IniFilename = NULL;  //prevents from saving the last states and position of each widgets
   
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

//======================================Load Icons/Images==========================================================================================================
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

    IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/ON/Settings.png", &Settings_image.ON_textureID, &Settings_image.width, &Settings_image.height));
    IM_ASSERT(LoadTextureFromFile("../../../Utils/icons/OFF/Settings.png", &Settings_image.OFF_textureID));
//================================================================================================================================================================

//==================================Initializations===============================================================================================================
    const RGBA bg_col = RGBA(24, 24, 24, 255);
    const RGBA highlighter_col = RGBA(0, 120, 212, 255);
    vertical_tool_bar = new ImGui::ToolBar("Vertical", ImVec2(25, 25), bg_col, highlighter_col, 5, 30, ImGuiAxis_Y);
    {
        vertical_tool_bar->AppendTool("Folder", Folder_image, nullptr);
        vertical_tool_bar->AppendTool("Search", Search_image, nullptr);
        vertical_tool_bar->AppendTool("Debug", Debug_image, nullptr);
        vertical_tool_bar->AppendTool("Simulate", Robot_image, nullptr);
        vertical_tool_bar->AppendTool("Settings", Settings_image, nullptr, false);  
    }

    horizontal_tool_bar = new ImGui::ToolBar("Horizontal", ImVec2(25, 25), bg_col, 30);
    {
        horizontal_tool_bar->AppendTool("verify", Verify_image, nullptr);
        horizontal_tool_bar->AppendTool("Upload", Compile_image, nullptr);
    }

    status_bar = new ImGui::StatusBar("status", 30, horizontal_tool_bar->GetColor(), RGBA(51,57,60,200));
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
                    if (ImGui::MenuItem("Save", "CTRL+S")) {} 
                    if (ImGui::MenuItem("Save As", "CTRL+Shift+S")) {} 
                    ImGui::Separator();
                    if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                    if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                    ImGui::Separator();
                    if (ImGui::MenuItem("Quit", "CTRL+Q")) {}
                    ImGui::EndMenu();
                }

                if (ImGui::BeginMenu("Edit"))
                {
                    if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
                    if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
                    ImGui::Separator();
                    if (ImGui::MenuItem("Cut", "CTRL+X")) {}
                    if (ImGui::MenuItem("Copy", "CTRL+C")) {}
                    if (ImGui::MenuItem("Paste", "CTRL+V")) {}
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            vertical_tool_bar->SetPaddingBefore("Settings", ImGui::GetWindowHeight() - vertical_tool_bar->GetToolSize().y);

            status_bar->SetStatusBar();
            horizontal_tool_bar->SetToolBar();
            vertical_tool_bar->SetToolBar(horizontal_tool_bar->GetThickness(), status_bar->GetThickness());

            static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode;

            // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
            // because it would be confusing to have two docking targets within each others.
            ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->Pos);
            ImGui::SetNextWindowSize(viewport->Size);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

            // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
            if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

            RGBA bg = RGBA(31, 39,42, 255);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
            ImGui::PushStyleColor(ImGuiCol_WindowBg, bg.GetCol());
            ImGui::Begin("DockSpace", nullptr, window_flags);
            {
                ImGui::PopStyleVar();
                ImGui::PopStyleVar(2);

                
            }
            ImGui::End();
            ImGui::PopStyleColor();
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

    delete file;
    delete edit;

    delete vertical_tool_bar;
    delete horizontal_tool_bar;

    delete status_bar;
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