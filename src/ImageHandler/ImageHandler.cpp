#include "ImageHandler.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../imgui/stb_image.h"


void Free_STBI_Image(void *retval_from_stbi_load)
{
    stbi_image_free(retval_from_stbi_load);
}

unsigned char *Load_STBI(char const *filename, int *x, int *y, int *comp, int req_comp)
{
    return stbi_load(filename, x, y, comp, req_comp);
}
