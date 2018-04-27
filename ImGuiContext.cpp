#include "ImGuiContext.h"

#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>

#include <d3d.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#include <stdio.h>

#include "imgui.h"
#include "imgui_internal.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "imm32.lib")

extern HWND g_hWnd;
extern ID3D11DeviceContext* g_pd3dDeviceContext;
extern ID3D11Device* g_pd3dDevice;
extern IDXGIFactory1* g_pFactory;
static ID3D11RenderTargetView* g_mainRenderTargetView;

//extern LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
extern void ImGui_ImplWin32_InitPlatformInterface();
extern void ImGui_ImplDX11_InitPlatformInterface();

namespace ImGuiCLI
{
    ImGuiContext::ImGuiContext(System::IntPtr hwnd, System::IntPtr devicePtr, System::IntPtr mainDeviceContext, System::IntPtr renderTarget)
    {
        g_hWnd = (HWND)hwnd.ToInt64();
        g_pd3dDeviceContext = (ID3D11DeviceContext*)mainDeviceContext.ToPointer();
        g_pd3dDevice = (ID3D11Device*)devicePtr.ToPointer();

        ImGui::CreateContext();

        ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

        g_mainRenderTargetView = (ID3D11RenderTargetView*)renderTarget.ToPointer();

        // Setup ImGui binding
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
        io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
        io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls

        // Setup back-end capabilities flags
        io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;         // We can honor GetMouseCursor() values (optional)
        io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
        io.BackendFlags |= ImGuiBackendFlags_PlatformHasViewports;    // We can create multi-viewports on the Platform side (optional)
        io.BackendFlags |= ImGuiBackendFlags_HasMouseHoveredViewport; // We can set io.MouseHoveredViewport correctly (optional, not easy)
        
        ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        main_viewport->PlatformHandle = (void*)g_hWnd;
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            ImGui_ImplWin32_InitPlatformInterface();
        //ImGui::GetIO().ImeWindowHandle = g_hWnd;

        // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array that we will update during the application lifetime.
        io.KeyMap[ImGuiKey_Tab] = VK_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
        io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
        io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
        io.KeyMap[ImGuiKey_Home] = VK_HOME;
        io.KeyMap[ImGuiKey_End] = VK_END;
        io.KeyMap[ImGuiKey_Insert] = VK_INSERT;
        io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
        io.KeyMap[ImGuiKey_Space] = VK_SPACE;
        io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
        io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
        io.KeyMap[ImGuiKey_A] = 'A';
        io.KeyMap[ImGuiKey_C] = 'C';
        io.KeyMap[ImGuiKey_V] = 'V';
        io.KeyMap[ImGuiKey_X] = 'X';
        io.KeyMap[ImGuiKey_Y] = 'Y';
        io.KeyMap[ImGuiKey_Z] = 'Z';

        io.Fonts->AddFontDefault();

        // Setup back-end capabilities flags
        io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;    // We can create multi-viewports on the Renderer side (optional)
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            ImGui_ImplDX11_InitPlatformInterface();

        // Setup style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;

        ImGui_ImplDX11_CreateDeviceObjects();
    }

    ImGuiContext::~ImGuiContext()
    {
    }

    void ImGuiContext::NewFrame(int w, int h)
    {
        extern ID3D11SamplerState* g_pFontSampler;
        if (!g_pFontSampler)
            ImGui_ImplDX11_CreateDeviceObjects();
        auto& io = ImGui::GetIO();
        io.DisplaySize.x = w;
        io.DisplaySize.y = h;
        extern void ImGui_ImplDX11_NewFrame();
        extern void ImGui_ImplWin32_NewFrame();
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        static float color[] = { 0, 0, 1, 1 };
        {
            static float f = 0.0f;
            static int counter = 0;
            ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
            ImGui::ColorEdit3("clear color", color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        ImGui::ShowDemoWindow();
    }

    void ImGuiContext::Shutdown()
    {
        extern void ImGui_ImplDX11_ShutdownPlatformInterface();
        extern void ImGui_ImplDX11_InvalidateDeviceObjects();
        ImGui_ImplDX11_ShutdownPlatformInterface();
        ImGui_ImplDX11_InvalidateDeviceObjects();

        extern void ImGui_ImplWin32_ShutdownPlatformInterface();
        ImGui_ImplWin32_ShutdownPlatformInterface();

        ImGui::DestroyContext();
    }

    void ImGuiContext::RenderNoDraw()
    {
        auto& io = ImGui::GetIO();
        ImGui::Render();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            ImGui::UpdatePlatformWindows();
    }

    void ImGuiContext::RenderAndDraw(System::IntPtr renderTarget)
    {
        auto& io = ImGui::GetIO();
        g_mainRenderTargetView = (ID3D11RenderTargetView*)renderTarget.ToPointer();
        extern void ImGui_ImplDX11_RenderDrawData(ImDrawData*);
        // Rendering
        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    }

    void ImGuiContext::Draw(System::IntPtr renderTarget)
    {
        g_mainRenderTargetView = (ID3D11RenderTargetView*)renderTarget.ToPointer();
        extern void ImGui_ImplDX11_RenderDrawData(ImDrawData*);

        ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, (float*)&clear_color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            ImGui::RenderPlatformWindowsDefault();
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    }

    void ImGuiContext::ResizeMain(int width, int height, System::IntPtr mainRenderTarget)
    {

    }
}