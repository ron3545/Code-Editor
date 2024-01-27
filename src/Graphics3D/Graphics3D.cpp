#include "Graphics3D.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"
#include "../ImageHandler/ImageHandler.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

using Microsoft::WRL::ComPtr;

/**
 * Rendering 3D model -> https://github.com/microsoft/DirectXTK/wiki/Rendering-a-model
 
 * DirectXTK Samples -> https://github.com/walbourn/directxtk-samples

 * DirectXTK Model Viewer Sample -> https://github.com/walbourn/directxtkmodelviewer
*/

GraphicsHandler::GraphicsHandler( HWND hWnd )
{
  
}

GraphicsHandler::~GraphicsHandler()
{
}
