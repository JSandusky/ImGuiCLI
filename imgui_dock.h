#pragma once

#include "imgui.h"


namespace ImGui
{

typedef unsigned ImGuiDockFlags;

IMGUI_API void ShutdownDock();
IMGUI_API void RootDock(const ImVec2& pos, const ImVec2& size);
IMGUI_API bool BeginDock(const char* label, bool* opened = nullptr, ImGuiWindowFlags extra_flags = 0, ImGuiDockFlags dock_flags = 0);
IMGUI_API void EndDock();
IMGUI_API void SetDockActive();
IMGUI_API void LoadDock();
IMGUI_API void SaveDock();
IMGUI_API void Print();

} // namespace ImGui

enum
{
    ImGuiDockFlags_NoTabs = 1,
    ImGuiDockFlags_StartLeft = 1 << 1,
    ImGuiDockFlags_StartRight = 1 << 2,
    ImGuiDockFlags_StartTop = 1 << 3,
    ImGuiDockFlags_StartBottom = 1 << 4,
    ImGuiDockFlags_NoPad = 1 << 5,
    ImGuiDockFlags_Hidden = 1 << 6,
    ImGuiDockFlags_NoCloseButton = 1 << 7
};