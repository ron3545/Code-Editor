#pragma once
#include <d3d11.h>
#include "../imgui/imgui.h"

struct RGBA
{
    float R, G, B, A;
    typedef unsigned int uint;
    RGBA() {}
    RGBA(uint r, uint g, uint b, uint a) : R(r), G(g), B(b), A(a) {}

    RGBA operator=(const RGBA& other)
    {
        this->R = other.R;
        this->G = other.G;
        this->B = other.B;
        this->A = other.A;

        return *this;
    }

    ImVec4 GetCol()
    {
        return ImVec4(R/255, G/255, B/255, A/255);
    }

    ImVec4 GetCol() const
    {
        return ImVec4(R/255, G/255, B/255, A/255);
    }
};

struct ImageData
{
    ID3D11ShaderResourceView* ON_textureID;
    ID3D11ShaderResourceView* OFF_textureID;
    int width, height;
};

bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width = nullptr, int* out_height = nullptr);
