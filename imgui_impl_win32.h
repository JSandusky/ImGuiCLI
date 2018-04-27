#pragma once
#include "imgui.h"
// ImGui Platform Binding for: Windows (standard windows API for 32 and 64 bits applications)
// This needs to be used along with a Renderer (e.g. DirectX11, OpenGL3, Vulkan..)

IMGUI_API bool        ImGui_ImplWin32_Init(void* hwnd);
IMGUI_API void        ImGui_ImplWin32_Shutdown();
IMGUI_API void        ImGui_ImplWin32_NewFrame();

// DPI-related helpers (which run and compile without requiring 8.1 or 10, neither Windows version, neither associated SDK)
IMGUI_API void        ImGui_ImplWin32_EnableDpiAwareness();
IMGUI_API float       ImGui_ImplWin32_GetDpiScaleForHwnd(void* hwnd);       // HWND hwnd
IMGUI_API float       ImGui_ImplWin32_GetDpiScaleForMonitor(void* monitor); // HMONITOR monitor
IMGUI_API float       ImGui_ImplWin32_GetDpiScaleForRect(int x1, int y1, int x2, int y2);

// Handler for Win32 messages, update mouse/keyboard data.
// You may or not need this for your implementation, but it can serve as reference for handling inputs.
// Intentionally commented out to avoid dragging dependencies on <windows.h> types. You can copy the extern declaration in your code.
/*
IMGUI_API LRESULT   ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
*/
