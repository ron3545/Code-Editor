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

struct TwoStateImageData
{
    ID3D11ShaderResourceView* ON_textureID;
    ID3D11ShaderResourceView* OFF_textureID;
    int width, height;
};

struct SingleStateImageData
{
    ID3D11ShaderResourceView* textureID;
    int width, height;

    void SafeDelete()
    {
        delete this->textureID;
        this->textureID = NULL;
    }
};

void Free_STBI_Image(void* retval_from_stbi_load);
unsigned char* Load_STBI(char const *filename, int *x, int *y, int *comp, int req_comp);