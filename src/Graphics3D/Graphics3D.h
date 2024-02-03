#pragma once
#include "DeviceResourcesPC.h"
#include "StepTimer.h"
#include "RenderTexture.h"

class GraphicsHandler : public DX::IDeviceNotify
{
public:
    GraphicsHandler( HWND hWnd, ID3D11Device* device, const char* model_path ) noexcept(false);

    GraphicsHandler( const GraphicsHandler& ) = delete;
    GraphicsHandler& operator=( const GraphicsHandler& ) = delete;

    GraphicsHandler( GraphicsHandler& ) = delete;
    GraphicsHandler& operator=( GraphicsHandler& ) = delete;

    ~GraphicsHandler();

    // IDeviceNotify
    void OnDeviceLost() override;
    void OnDeviceRestored() override;

private:
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

private:
    DirectX::SimpleMath::Matrix m_world;
    DirectX::SimpleMath::Matrix m_view;
    DirectX::SimpleMath::Matrix m_proj;

    std::unique_ptr<DirectX::CommonStates> m_states;
    std::unique_ptr<DirectX::IEffectFactory> m_fxFactory;
    std::unique_ptr<DirectX::Model> m_model;

    // Rendering loop timer.
    DX::StepTimer                                   m_timer;

};