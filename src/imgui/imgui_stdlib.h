// dear imgui: wrappers for C++ standard library (STL) types (std::string, etc.)
// This is also an example of how you may wrap your own similar types.

// Changelog:
// - v0.10: Initial version. Added InputText() / InputTextMultiline() calls with std::string

// See more C++ related extension (fmt, RAII, syntaxis sugar) on Wiki:
//   https://github.com/ocornut/imgui/wiki/Useful-Extensions#cness

#pragma once

#include <string>
#include "imgui_internal.h"
#include "../CodeEditor/AppLog.hpp"

namespace ImGui
{
    // ImGui::InputText() with std::string
    // Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
    IMGUI_API bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
    IMGUI_API bool InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
    IMGUI_API bool InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
    IMGUI_API bool Splitter(const char* label, const ImU32& OffState, const ImU32& OnState, const ImVec2& size_arg, const ImVec2& pos_arg, float* thickness, ImGuiAxis axis);
    
    IMGUI_API bool ColoredTreeNode(const char* text, const char* keyword_to_highlght, unsigned int keyword_X_loc, const ImVec4& highlighter_col = ImVec4(93, 4, 16, 30), ImGuiTreeNodeFlags flags = 0);
    IMGUI_API bool ColoredTreeNodeBehavior(ImGuiID id, const char* text, const char* keyword_to_highlght, unsigned int keyword_X_loc, const ImVec4& highlighter_col, const char* label_end = NULL, ImGuiTreeNodeFlags flags = 0);
}
