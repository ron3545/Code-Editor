#include "Graphics3D.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../ImageHandler/ImageHandler.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

//helper function 
const wchar_t * GetWC(const char* str)
{
    //taken from https://stackoverflow.com/questions/8032080/how-to-convert-char-to-wchar-t
    const size_t cSize = strlen(str) + 1;
    wchar_t* wc = new wchar_t[cSize];
    mbstowcs(wc, str, cSize);
    return wc;
}

/**
 * Rendering 3D model -> https://github.com/microsoft/DirectXTK/wiki/Rendering-a-model
 
 * DirectXTK Samples -> https://github.com/walbourn/directxtk-samples

 * DirectXTK Model Viewer Sample -> https://github.com/walbourn/directxtkmodelviewer
*/

GraphicsHandler::GraphicsHandler( HWND hWnd, ID3D11Device* device, const char* model_path ) noexcept(false) 
{
    m_states = std::make_unique<CommonStates>(device);
    m_fxFactory = std::make_unique<EffectFactory>(device);
    m_model = Model::CreateFromSDKMESH(device, GetWC(model_path), *m_fxFactory);
    m_world = Matrix::Identity;
}

GraphicsHandler::~GraphicsHandler()
{
}

void GraphicsHandler::OnDeviceLost() 
{
    m_states.reset();
    m_fxFactory.reset();
    m_model.reset();
}

void GraphicsHandler::OnDeviceRestored() 
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}

void GraphicsHandler::CreateDeviceDependentResources()
{
    
}

void GraphicsHandler::CreateWindowSizeDependentResources()
{
    
}
