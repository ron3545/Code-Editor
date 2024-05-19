#pragma once
#if defined(__WIN32) || defined(_WIN64)
    #include <d3d11.h>
    typedef ID3D11ShaderResourceView* TEXTURE_TYPE;
#else
    #include <GLFW/glfw3.h>
    typedef GLuint TEXTURE_TYPE;
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_stdlib.h"

struct RGBA
{   
    typedef unsigned int uint;
    uint R, G, B, A;
    
    RGBA() : R(0), G(0), B(0), A(0) {}
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
    TEXTURE_TYPE ON_textureID;
    TEXTURE_TYPE OFF_textureID;
    int width, height;
};

struct SingleStateImageData
{
    TEXTURE_TYPE textureID;
    int width, height;
};

void Free_STBI_Image(void* retval_from_stbi_load);
unsigned char* Load_STBI(char const *filename, int *x, int *y, int *comp, int req_comp);