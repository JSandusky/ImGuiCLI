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

struct VERTEX_CONSTANT_BUFFER
{
    float        mvp[4][4];
};

struct ImGuiViewportDataDx11
{
    IDXGISwapChain*             SwapChain;
    ID3D11RenderTargetView*     RTView;

    ImGuiViewportDataDx11() { SwapChain = NULL; RTView = NULL; }
    ~ImGuiViewportDataDx11() { IM_ASSERT(SwapChain == NULL && RTView == NULL); }
};

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

    class ImGuiNative
    {
    public:
        static ImGuiNative* inst_;

        ID3D11DeviceContext* deviceContext_;
        ID3D11Device* device_;
        IDXGIFactory1* factory_;

        ID3D11Buffer*            g_pVB = NULL;
        ID3D11Buffer*            g_pIB = NULL;
        ID3D10Blob *             g_pVertexShaderBlob = NULL;
        ID3D11VertexShader*      g_pVertexShader = NULL;
        ID3D11InputLayout*       g_pInputLayout = NULL;
        ID3D11Buffer*            g_pVertexConstantBuffer = NULL;
        ID3D10Blob *             g_pPixelShaderBlob = NULL;
        ID3D11PixelShader*       g_pPixelShader = NULL;
        ID3D11SamplerState*      g_pFontSampler = NULL;
        ID3D11ShaderResourceView*g_pFontTextureView = NULL;
        ID3D11RasterizerState*   g_pRasterizerState = NULL;
        ID3D11BlendState*        g_pBlendState = NULL;
        ID3D11DepthStencilState* g_pDepthStencilState = NULL;
        int                      g_VertexBufferSize = 5000, g_IndexBufferSize = 10000;

        ImGuiNative()
        {
            inst_ = this;
        }

        bool CreateDeviceObjects()
        {
            if (!device_)
                return false;
            if (g_pFontSampler)
                InvalidateDeviceObjects();

            // By using D3DCompile() from <d3dcompiler.h> / d3dcompiler.lib, we introduce a dependency to a given version of d3dcompiler_XX.dll (see D3DCOMPILER_DLL_A)
            // If you would like to use this DX11 sample code but remove this dependency you can: 
            //  1) compile once, save the compiled shader blobs into a file or source code and pass them to CreateVertexShader()/CreatePixelShader() [preferred solution]
            //  2) use code to detect any version of the DLL and grab a pointer to D3DCompile from the DLL. 
            // See https://github.com/ocornut/imgui/pull/638 for sources and details.

            // Create the vertex shader
            {
                static const char* vertexShader =
                    "cbuffer vertexBuffer : register(b0) \
            {\
            float4x4 ProjectionMatrix; \
            };\
            struct VS_INPUT\
            {\
            float2 pos : POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            \
            struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            \
            PS_INPUT main(VS_INPUT input)\
            {\
            PS_INPUT output;\
            output.pos = mul( ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\
            output.col = input.col;\
            output.uv  = input.uv;\
            return output;\
            }";

                D3DCompile(vertexShader, strlen(vertexShader), NULL, NULL, NULL, "main", "vs_4_0", 0, 0, &g_pVertexShaderBlob, NULL);
                if (g_pVertexShaderBlob == NULL) // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
                    return false;
                if (device_->CreateVertexShader((DWORD*)g_pVertexShaderBlob->GetBufferPointer(), g_pVertexShaderBlob->GetBufferSize(), NULL, &g_pVertexShader) != S_OK)
                    return false;

                // Create the input layout
                D3D11_INPUT_ELEMENT_DESC local_layout[] =
                {
                    { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (size_t)(&((ImDrawVert*)0)->uv),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
                    { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (size_t)(&((ImDrawVert*)0)->col), D3D11_INPUT_PER_VERTEX_DATA, 0 },
                };
                if (device_->CreateInputLayout(local_layout, 3, g_pVertexShaderBlob->GetBufferPointer(), g_pVertexShaderBlob->GetBufferSize(), &g_pInputLayout) != S_OK)
                    return false;

                // Create the constant buffer
                {
                    D3D11_BUFFER_DESC desc;
                    desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
                    desc.Usage = D3D11_USAGE_DYNAMIC;
                    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
                    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
                    desc.MiscFlags = 0;
                    device_->CreateBuffer(&desc, NULL, &g_pVertexConstantBuffer);
                }
            }

            // Create the pixel shader
            {
                static const char* pixelShader =
                    "struct PS_INPUT\
            {\
            float4 pos : SV_POSITION;\
            float4 col : COLOR0;\
            float2 uv  : TEXCOORD0;\
            };\
            sampler sampler0;\
            Texture2D texture0;\
            \
            float4 main(PS_INPUT input) : SV_Target\
            {\
            float4 out_col = input.col * texture0.Sample(sampler0, input.uv); \
            return out_col; \
            }";

                D3DCompile(pixelShader, strlen(pixelShader), NULL, NULL, NULL, "main", "ps_4_0", 0, 0, &g_pPixelShaderBlob, NULL);
                if (g_pPixelShaderBlob == NULL)  // NB: Pass ID3D10Blob* pErrorBlob to D3DCompile() to get error showing in (const char*)pErrorBlob->GetBufferPointer(). Make sure to Release() the blob!
                    return false;
                if (device_->CreatePixelShader((DWORD*)g_pPixelShaderBlob->GetBufferPointer(), g_pPixelShaderBlob->GetBufferSize(), NULL, &g_pPixelShader) != S_OK)
                    return false;
            }

            // Create the blending setup
            {
                D3D11_BLEND_DESC desc;
                ZeroMemory(&desc, sizeof(desc));
                desc.AlphaToCoverageEnable = false;
                desc.RenderTarget[0].BlendEnable = true;
                desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
                desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
                desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
                desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
                desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
                desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
                desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
                device_->CreateBlendState(&desc, &g_pBlendState);
            }

            // Create the rasterizer state
            {
                D3D11_RASTERIZER_DESC desc;
                ZeroMemory(&desc, sizeof(desc));
                desc.FillMode = D3D11_FILL_SOLID;
                desc.CullMode = D3D11_CULL_NONE;
                desc.ScissorEnable = true;
                desc.DepthClipEnable = true;
                device_->CreateRasterizerState(&desc, &g_pRasterizerState);
            }

            // Create depth-stencil State
            {
                D3D11_DEPTH_STENCIL_DESC desc;
                ZeroMemory(&desc, sizeof(desc));
                desc.DepthEnable = false;
                desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
                desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
                desc.StencilEnable = false;
                desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
                desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
                desc.BackFace = desc.FrontFace;
                device_->CreateDepthStencilState(&desc, &g_pDepthStencilState);
            }

            CreateFontsTexture();

            return true;
        }

        void CreateFontsTexture()
        {
            // Build texture atlas
            ImGuiIO& io = ImGui::GetIO();
            unsigned char* pixels;
            int width, height;
            io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

            // Upload texture to graphics system
            {
                D3D11_TEXTURE2D_DESC desc;
                ZeroMemory(&desc, sizeof(desc));
                desc.Width = width;
                desc.Height = height;
                desc.MipLevels = 1;
                desc.ArraySize = 1;
                desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                desc.SampleDesc.Count = 1;
                desc.Usage = D3D11_USAGE_DEFAULT;
                desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
                desc.CPUAccessFlags = 0;

                ID3D11Texture2D *pTexture = NULL;
                D3D11_SUBRESOURCE_DATA subResource;
                subResource.pSysMem = pixels;
                subResource.SysMemPitch = desc.Width * 4;
                subResource.SysMemSlicePitch = 0;
                device_->CreateTexture2D(&desc, &subResource, &pTexture);

                // Create texture view
                D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
                ZeroMemory(&srvDesc, sizeof(srvDesc));
                srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
                srvDesc.Texture2D.MipLevels = desc.MipLevels;
                srvDesc.Texture2D.MostDetailedMip = 0;
                device_->CreateShaderResourceView(pTexture, &srvDesc, &g_pFontTextureView);
                pTexture->Release();
            }

            // Store our identifier
            io.Fonts->TexID = (void *)g_pFontTextureView;

            // Create texture sampler
            {
                D3D11_SAMPLER_DESC desc;
                ZeroMemory(&desc, sizeof(desc));
                desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
                desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
                desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
                desc.MipLODBias = 0.f;
                desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
                desc.MinLOD = 0.f;
                desc.MaxLOD = 0.f;
                device_->CreateSamplerState(&desc, &g_pFontSampler);
            }
        }

        void InvalidateDeviceObjects()
        {
            if (!device_)
                return;

            if (g_pFontSampler) { g_pFontSampler->Release(); g_pFontSampler = NULL; }
            if (g_pFontTextureView) { g_pFontTextureView->Release(); g_pFontTextureView = NULL; ImGui::GetIO().Fonts->TexID = NULL; } // We copied g_pFontTextureView to io.Fonts->TexID so let's clear that as well.
            if (g_pIB) { g_pIB->Release(); g_pIB = NULL; }
            if (g_pVB) { g_pVB->Release(); g_pVB = NULL; }

            if (g_pBlendState) { g_pBlendState->Release(); g_pBlendState = NULL; }
            if (g_pDepthStencilState) { g_pDepthStencilState->Release(); g_pDepthStencilState = NULL; }
            if (g_pRasterizerState) { g_pRasterizerState->Release(); g_pRasterizerState = NULL; }
            if (g_pPixelShader) { g_pPixelShader->Release(); g_pPixelShader = NULL; }
            if (g_pPixelShaderBlob) { g_pPixelShaderBlob->Release(); g_pPixelShaderBlob = NULL; }
            if (g_pVertexConstantBuffer) { g_pVertexConstantBuffer->Release(); g_pVertexConstantBuffer = NULL; }
            if (g_pInputLayout) { g_pInputLayout->Release(); g_pInputLayout = NULL; }
            if (g_pVertexShader) { g_pVertexShader->Release(); g_pVertexShader = NULL; }
            if (g_pVertexShaderBlob) { g_pVertexShaderBlob->Release(); g_pVertexShaderBlob = NULL; }
        }

        void NewFrame()
        {
            if (!g_pFontSampler)
                CreateDeviceObjects();
        }

    };

    ImGuiNative* ImGuiNative::inst_ = nullptr;

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
        delete pData_;
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