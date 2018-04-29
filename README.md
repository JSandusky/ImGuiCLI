# ImGuiCLI

**WARNING:** this is a 4 hour mash-together project. Really just a viability test.

C++/CLI wrapper for DearImGui Viewports branch, specifically for MonoGame.

Mashes the Dear ImGui Win32 DirectX11 demo into a C++/CLI wrapper. The crap toss is found in `class ImGuiContext` located in ImGuiContext.h/.cpp.

`class ImGuiCli` can be used with vanilla ImGui (some extension types will need to be ripped out, nothing major). Located in ImGuiCLI.h/.cpp

Mashing the demo is fairly advantageous given the WIP status of the Viewports branch. Updating to the latest only takes around 15 minutes to merge.

## Deviations

Access is entirely global single-instance.

DearImGui is tweaked to not own the ID3D11Device, ID3D11DeviceContext, or ID3D11RenderTargetView for the main render target (or functions that would try to own those are not called). MonoGame instead passes those to it where needed.

Enumerations/flags are wrapped as their underscored name (ie. `ImGuiDir_`) with their values mapped without the prefix.

`class ImGuiIO` contains some access to Dear ImGui's ImGuiContext menu, largely just convenience as binding the ImGuiContext type isn't necessary for realistic cases (custom widgets should be written in C and bound to C#).

Render/Draw has three functions, two of which are intended to be used together (for periodic updates re-rendering old draw-data).

- `ImGuiCLI::ImGuiContext::RenderAndDraw(IntPtr backBuffer)` does both the ImGui::Render and draws to the given render-target.
- `ImGuiCLI::ImGuiContext::RenderNoDraw()` only does the ImGui::Render call for building up the draw lists and platform updates.
- `ImGuiCLI::ImGuiContext::Draw(IntPtr backBuffer)` only does the graphics device rendering dispatch calls for existing draw lists (and platform window draws).

## C# Usage

Complete example [gist](https://gist.github.com/JSandusky/11b6a6ea85d42c9ab8606378a78c50cf).

### Initialize

       System.IntPtr device = ((SharpDX.Direct3D11.Device)GraphicsDevice.Handle).NativePointer;
        System.IntPtr deviceContext = ((SharpDX.Direct3D11.DeviceContext)GraphicsDevice.ContextHandle).NativePointer;
        System.IntPtr backBuffer = ((SharpDX.Direct3D11.RenderTargetView)GraphicsDevice.BackBuffer).NativePointer;
    
        var form = ((MonoGame.Framework.WinFormsGameWindow)Window).Form;
        form.SignalNativeMessages.Add(WM_MOUSEHWHEEL);
        form.SignalNativeMessages.Add(WM_KEYDOWN);
        form.SignalNativeMessages.Add(WM_KEYUP);
        form.SignalNativeMessages.Add(WM_CHAR);
        form.NotifyMessage += (o, e) =>
        {
            if (e.Msg == WM_KEYDOWN)
                ImGuiIO.SetKeyState(e.WParam.ToInt32(), true);
            else if (e.Msg == WM_KEYUP)
                ImGuiIO.SetKeyState(e.WParam.ToInt32(), false);
            else if (e.Msg == WM_CHAR && e.WParam.ToInt64() > 0 && e.WParam.ToInt64() < 0x10000)
                ImGuiIO.AddText((ushort)e.WParam.ToInt64());
        };
        imguiContext_ = new ImGuiContext(this.Window.Handle, device, deviceContext, backBuffer);

### Run

        float td = gameTime.ElapsedGameTime.Milliseconds / 1000.0f;
        ImGuiIO.DeltaTime = td;
        int newScroll = Mouse.GetState().ScrollWheelValue;
        int scrollDelta = newScroll - scrollValue;
        scrollValue = newScroll;
        if (IsFocused)
            ImGuiIO.SetMouseWheel(scrollDelta / 120.0f);
    
        ImGuiIO.SetMouseButton(0, Mouse.GetState().LeftButton == ButtonState.Pressed);
        ImGuiIO.SetMouseButton(1, Mouse.GetState().RightButton == ButtonState.Pressed);
        ImGuiIO.SetMouseButton(2, Mouse.GetState().MiddleButton == ButtonState.Pressed);
        switch (ImGuiIO.MouseCursor)
        {
        case ImGuiMouseCursor_.Arrow:
        case ImGuiMouseCursor_.None:
            Mouse.PlatformSetCursor(MouseCursor.Arrow);
            break;
        case ImGuiMouseCursor_.ResizeAll:
            Mouse.PlatformSetCursor(MouseCursor.SizeAll);
            break;
        case ImGuiMouseCursor_.ResizeEW:
            Mouse.PlatformSetCursor(MouseCursor.SizeWE);
            break;
        case ImGuiMouseCursor_.ResizeNS:
            Mouse.PlatformSetCursor(MouseCursor.SizeNS);
            break;
        case ImGuiMouseCursor_.ResizeNESW:
            Mouse.PlatformSetCursor(MouseCursor.SizeNESW);
            break;
        case ImGuiMouseCursor_.ResizeNWSE:
            Mouse.PlatformSetCursor(MouseCursor.SizeNWSE);
            break;
        case ImGuiMouseCursor_.TextInput:
            Mouse.PlatformSetCursor(MouseCursor.IBeam);
            break;
        }
        imguiContext_.NewFrame(GraphicsDevice.Viewport.Width, GraphicsDevice.Viewport.Height);
    
        if (ImGuiCli.Begin("Trial window", 0))
        {
            ImGuiCli.Text("Drawing some text");
            ImGuiCli.Checkbox("Checkbox", ref someValue);
        }
        ImGuiCli.End();
    
            imguiContext_.RenderAndDraw(((SharpDX.Direct3D11.RenderTargetView)GraphicsDevice.BackBuffer).NativePointer);

### Shutdown

`imguiContext_.Shutdown();`

## Wrapper Files

- ImGuiCLI.h
- ImGuiCLI.cpp
- ImGuiContext.h
- ImGuiContext.cpp

## MonoGame.Framework.Windows Changes

- Change visibility of Microsoft.Xna.Framework.Windows.**WinFormsGameWindow to public**

- Change visibility of Microsoft.Xna.Framework.Windows.**WinFormsGameForm to public**

- Change visibility of Microsoft.Xna.Framework.Windows.**HorizontalMouseWheelEventArgs to public**
    - This is just to deal with *less visible than XXX* and not actually a meaningful change

- Expose ID3DDeviceContext and BackBuffer IntPtr's in **GraphicsDevice.DirectX.cs**

        public object BackBuffer
        {
            get { return _renderTargetView; }
        }
        
        public object ContextHandle
        {
            get { return _d3dContext; }
        }

- Use Windows style message loop in **WinFormsGameWindow**'s RunLoop()

        [System.Security.SuppressUnmanagedCodeSecurity] // We won't use this maliciously
        [DllImport("user32.dll")]
        public static extern bool TranslateMessage([In] ref NativeMessage lpMsg);
        
        [System.Security.SuppressUnmanagedCodeSecurity] // We won't use this maliciously
        [DllImport("user32.dll")]
        public static extern IntPtr DispatchMessage([In] ref NativeMessage lpmsg);
        
        internal void RunLoop()
        {
            // https://bugzilla.novell.com/show_bug.cgi?id=487896
            // Since there's existing bug from implementation with mono WinForms since 09'
            // Application.Idle is not working as intended
            // So we're just going to emulate Application.Run just like Microsoft implementation
            Form.Show();
        
            var nativeMsg = new NativeMessage();
            while (Form != null && Form.IsDisposed == false)
            {
                if (PeekMessage(out nativeMsg, IntPtr.Zero, 0, 0, 1))
                {
                    // !!!CHANGES HERE!!, was Application.DoEvents()
                    TranslateMessage(ref nativeMsg);
                    DispatchMessage(ref nativeMsg);
        
                    if (nativeMsg.msg == WM_QUIT)
                        break;
        
                    continue;
                }
                UpdateWindows();
                Game.Tick();
            }
            ... continue old code ...

- Add message routing to **WinFormsGameForm**, add event signal to `WndProc`

        public System.Collections.Generic.HashSet<int> SignalNativeMessages { get; private set; } = new System.Collections.Generic.HashSet<int>();
        
        public event EventHandler<Message> NotifyMessage;
        
        protected override void WndProc(ref Message m)
        {
            var state = TouchLocationState.Invalid;
        
            if (SignalNativeMessages.Contains(m.Msg) && NotifyMessage != null)
                NotifyMessage(this, m);
            ... continue old code ...

- Expose Graphics.**Texture.DirectX.cs** IntPtr for the SRV (*for ImGui::Image())

        public IntPtr GetNativeHandle()
        {
            return GetShaderResourceView().NativePointer;
        }
