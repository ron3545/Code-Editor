#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include "../imgui/imgui.h"
#include "../imgui/imgui_stdlib.h"
#include "../imgui/imgui_internal.h"

namespace ImGui
{
    IMGUI_API bool Splitter(const char* label, const ImU32& OffState, const ImU32& OnState, const ImVec2& size_arg, const ImVec2& pos_arg, float* thickness, ImGuiAxis axis);
    IMGUI_API bool ColoredTreeNode(const char* text, const char* keyword_to_highlght, unsigned int keyword_X_loc, const ImVec4& highlighter_col = ImVec4(93, 4, 16, 30), ImGuiTreeNodeFlags flags = 0);
    IMGUI_API bool ColoredTreeNodeBehavior(ImGuiID id, const char* text, const char* keyword_to_highlght, unsigned int keyword_X_loc, const ImVec4& highlighter_col, const char* label_end = NULL, ImGuiTreeNodeFlags flags = 0);
}