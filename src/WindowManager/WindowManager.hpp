#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define IMGUI_DEFINE_MATH_OPERATORS
//#define GLFW_DLL

#if defined(__WIN32) || defined(_WIN64)
    #define ANSI
    #define WIN32_LEAN_AND_MEAN
    #define _WIN32_WINNT   0x0501

    #include "../imgui/imgui_impl_win32.h"
    #include "../imgui/imgui_impl_dx11.h"

    #include <windows.h>
    #include <winuser.h>
    #include <initguid.h>
    #include <usbiodef.h>
    #include <Dbt.h>
    
#else
    #include <GL/glew.h>
    #include <SDL2/SDL.h>
    
    #include "../imgui/imgui.h"
    #include "../imgui/imgui_impl_sdl2.h"
    #include "../imgui/imgui_impl_opengl3.h"
#endif

#include "../CodeEditor/CodeEditor.hpp"
#include <vector>
#include <future>
#include <iterator>
#include "../ImageHandler/ImageHandler.h"

#include <cmath>
#include <stdio.h>

#include <stdexcept>
#include <string>
#include <stdexcept>
#include "../CodeEditor/AppLog.hpp"

#define HID_CLASSGUID {0x4d1e55b2, 0xf16f, 0x11cf,{ 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}}

#define ICONS "./Utils/icons/"
#define LIBFOLDER "./Utils/LibraryFolder/"
#define FONTS "./Utils/Fonts/"

static const char* Consolas_Font        = FONTS"Consolas.ttf";
static const char* DroidSansMono_Font   = FONTS"DroidSansMono.ttf";
static const char* Menlo_Regular_Font   = FONTS"Menlo-Regular.ttf";
static const char* MONACO_Font          = FONTS"MONACO.TTF"; 

static std::unique_ptr<CodeEditor> code_editor;

static CodeEditor::SingleStateIconPallete single_states_images;
static CodeEditor::TwoStateIconPallete two_states_images;


#if defined(__WIN32) || defined(_WIN64)
    constexpr wchar_t* SOFTWARE_NAME = L"RobLy";
    const char* LOGO = "";

    HWND                     hwnd = NULL;
    ID3D11Device*            g_pd3dDevice = nullptr;    
    ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
    IDXGISwapChain*          g_pSwapChain = nullptr;
    UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
    ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;
    WNDCLASSEXW wc;

    LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();
    bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width = nullptr, int* out_height = nullptr);
#else
    static SDL_Window* window = nullptr;
    static SDL_GLContext gl_context;

    bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width = nullptr, int* out_height = nullptr);
#endif


TwoStateImageData LoadTwoStateTextureFromFile(const char* filename1, const char* filename2);
SingleStateImageData LoadSingleStateTextureFromFile(const char* filename);

bool Initialize_Window();
void Execute_Window();
void Clean_Window();