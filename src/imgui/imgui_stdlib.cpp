// dear imgui: wrappers for C++ standard library (STL) types (std::string, etc.)
// This is also an example of how you may wrap your own similar types.

// Changelog:
// - v0.10: Initial version. Added InputText() / InputTextMultiline() calls with std::string

// See more C++ related extension (fmt, RAII, syntaxis sugar) on Wiki:
//   https://github.com/ocornut/imgui/wiki/Useful-Extensions#cness

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui.h"
#include "imgui_stdlib.h"


// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wsign-conversion"    // warning: implicit conversion changes signedness
#endif

struct InputTextCallback_UserData
{
    std::string*            Str;
    ImGuiInputTextCallback  ChainCallback;
    void*                   ChainCallbackUserData;
};

static int InputTextCallback(ImGuiInputTextCallbackData* data)
{
    InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
    {
        // Resize string callback
        // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
        std::string* str = user_data->Str;
        IM_ASSERT(data->Buf == str->c_str());
        str->resize(data->BufTextLen);
        data->Buf = (char*)str->c_str();
    }
    else if (user_data->ChainCallback)
    {
        // Forward to user callback, if any
        data->UserData = user_data->ChainCallbackUserData;
        return user_data->ChainCallback(data);
    }
    return 0;
}

bool ImGui::InputText(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

bool ImGui::InputTextMultiline(const char* label, std::string* str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputTextMultiline(label, (char*)str->c_str(), str->capacity() + 1, size, flags, InputTextCallback, &cb_user_data);
}

bool ImGui::InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
{
    IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
    flags |= ImGuiInputTextFlags_CallbackResize;

    InputTextCallback_UserData cb_user_data;
    cb_user_data.Str = str;
    cb_user_data.ChainCallback = callback;
    cb_user_data.ChainCallbackUserData = user_data;
    return InputTextWithHint(label, hint, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
}

bool ImGui::Splitter(const char* label, const ImU32& OffState, const ImU32& OnState, const ImVec2& size_arg, const ImVec2& pos_arg,float* thickness, ImGuiAxis axis)
{
    //TODO: when there is multiple splitter, mouse cursor on the other cannot be set.
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    IM_ASSERT(size_arg.x != 0.0f && size_arg.y != 0.0f);

    ImGuiContext& g = *GImGui;
    g.IO.MouseDrawCursor = true;
    const ImGuiID id = window->GetID(label);
    ImVec2 size = CalcItemSize(size_arg, 0.0f, 0.0f);

    const ImRect bb(pos_arg, pos_arg + size);

    ItemSize(size);
    if (!ItemAdd(bb, id))
        return false;
    
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_MouseButtonDefault_);
    
    const ImU32 col = (pressed || hovered ||held)? OnState : OffState;
    RenderNavHighlight(bb, id);
    RenderFrame(bb.Min, bb.Max, col, true, 0.0f);
    window->DrawList->AddRectFilled(bb.Min, bb.Max, GetColorU32(col));

    if(IsItemActive())
    {   
        ImGui::MarkItemEdited(id);
        if(axis == ImGuiAxis_X)
            *thickness -= ImGui::GetIO().MouseDelta.y;
        else if (axis == ImGuiAxis_Y)
            *thickness += ImGui::GetIO().MouseDelta.x;
    }
    
    return pressed;
}

#if defined(__clang__)
#pragma clang diagnostic pop
#endif
