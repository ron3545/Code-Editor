#pragma once
#include <d3d11.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

struct RGBA
{   
    typedef unsigned int uint;
    uint R, G, B, A;
    
    RGBA() : R(NULL), G(NULL), B(NULL), A(NULL) {}
    RGBA(uint r, uint g, uint b, uint a) : R(r), G(g), B(b), A(a) {}

    RGBA operator=(const RGBA& other)
    {
        this->R = other.R;
        this->G = other.G;
        this->B = other.B;
        this->A = other.A;

        return *this;
    }

    inline ImVec4 GetCol()
    {
        return ImVec4((float)R/255, (float)G/255, (float)B/255, (float)A/255);
    }

    inline ImVec4 GetCol() const
    {
        return ImVec4((float)R/255, (float)G/255, (float)B/255, (float)A/255);
    }
};

struct ImageData
{
    ID3D11ShaderResourceView* ON_textureID;
    ID3D11ShaderResourceView* OFF_textureID;
    int width, height;
};

struct SingleImageData
{
    ID3D11ShaderResourceView* textureID;
    int width, height;
};

bool LoadTextureFromFile(const char* filename, ID3D11ShaderResourceView** out_srv, int* out_width = nullptr, int* out_height = nullptr);
