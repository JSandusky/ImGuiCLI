#pragma once

#include <msclr/marshal.h>

namespace ImGuiCLI
{

    class ImGuiNative;

    /// Manages platform status of dear ImGui.
    public ref class ImGuiContext
    {
    public:
        /// Initialize interfaces and setup DX11 objects.
        ImGuiContext(System::IntPtr hwnd, System::IntPtr devicePtr, System::IntPtr mainDeviceContext, System::IntPtr mainRenderTarget);
        /// Release objects, except those given.
        ~ImGuiContext();

        void NewFrame(int w, int h);
        void Shutdown();
        void RenderNoDraw();
        void Draw(System::IntPtr renderTarget);
        void RenderAndDraw(System::IntPtr renderTarget);

        void ResizeMain(int width, int height, System::IntPtr mainRenderTarget);
    };

}