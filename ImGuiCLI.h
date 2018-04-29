// ImGuiCLI.h

#pragma once

using namespace System;
using namespace Microsoft::Xna::Framework;

namespace ImGuiCLI {

    [System::Flags]
    public enum class ImGuiTreeNodeFlags_
    {
        Selected = 1 << 0,   // Draw as selected
        Framed = 1 << 1,   // Full colored frame (e.g. for CollapsingHeader)
        AllowItemOverlap = 1 << 2,   // Hit testing to allow subsequent widgets to overlap this one
        NoTreePushOnOpen = 1 << 3,   // Don't do a TreePush() when open (e.g. for CollapsingHeader) = no extra indent nor pushing on ID stack
        NoAutoOpenOnLog = 1 << 4,   // Don't automatically and temporarily open node when Logging is active (by default logging will automatically open tree nodes)
        DefaultOpen = 1 << 5,   // Default node to be open
        OpenOnDoubleClick = 1 << 6,   // Need double-click to open node
        OpenOnArrow = 1 << 7,   // Only open when clicking on the arrow part. If ImGuiTreeNodeFlags_OpenOnDoubleClick is also set, single-click arrow or double-click all box to open.
        Leaf = 1 << 8,   // No collapsing, no arrow (use as a convenience for leaf nodes). 
        Bullet = 1 << 9,   // Display a bullet instead of arrow
        FramePadding = 1 << 10,  // Use FramePadding (even for an unframed text node) to vertically align text baseline to regular widget height. Equivalent to calling AlignTextToFramePadding().
        //ImGuITreeNodeFlags_SpanAllAvailWidth  = 1 << 11,  // FIXME: TODO: Extend hit box horizontally even if not framed
        //ImGuiTreeNodeFlags_NoScrollOnOpen     = 1 << 12,  // FIXME: TODO: Disable automatic scroll on TreePop() if node got just open and contents is not visible
        CollapsingHeader = Framed | NoAutoOpenOnLog
    };

    [System::Flags]
    public enum class ImGuiWindowFlags_
    {
        NoTitleBar = 1 << 0,   // Disable title-bar
        NoResize = 1 << 1,   // Disable user resizing with the lower-right grip
        NoMove = 1 << 2,   // Disable user moving the window
        NoScrollbar = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programatically)
        NoScrollWithMouse = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
        NoCollapse = 1 << 5,   // Disable user collapsing window by double-clicking on it
        AlwaysAutoResize = 1 << 6,   // Resize every window to its content every frame
        //ImGuiWindowFlags_ShowBorders          = 1 << 7,   // Show borders around windows and items (OBSOLETE! Use e.g. style.FrameBorderSize=1.0f to enable borders).
        NoSavedSettings = 1 << 8,   // Never load/save settings in .ini file
        NoInputs = 1 << 9,   // Disable catching mouse or keyboard inputs, hovering test with pass through.
        MenuBar = 1 << 10,  // Has a menu-bar
        HorizontalScrollbar = 1 << 11,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(ImVec2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
        NoFocusOnAppearing = 1 << 12,  // Disable taking focus when transitioning from hidden to visible state
        NoBringToFrontOnFocus = 1 << 13,  // Disable bringing window to front when taking focus (e.g. clicking on it or programatically giving it focus)
        AlwaysVerticalScrollbar = 1 << 14,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
        AlwaysHorizontalScrollbar = 1 << 15,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
        AlwaysUseWindowPadding = 1 << 16,  // Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient)
        ResizeFromAnySide = 1 << 17,  // (WIP) Enable resize from any corners and borders. Your back-end needs to honor the different values of io.MouseCursor set by imgui.

        // [Internal]
        ChildWindow = 1 << 24,  // Don't use! For internal use by BeginChild()
        Tooltip = 1 << 25,  // Don't use! For internal use by BeginTooltip()
        Popup = 1 << 26,  // Don't use! For internal use by BeginPopup()
        Modal = 1 << 27,  // Don't use! For internal use by BeginPopupModal()
        ChildMenu = 1 << 28   // Don't use! For internal use by BeginMenu()
    };

    // A cardinal direction
    public enum class ImGuiDir_
    {
        None = -1,
        Left = 0,
        Right = 1,
        Up = 2,
        Down = 3,
        COUNT
    };

    public enum class ImGuiKey_
    {
        Tab,
        LeftArrow,
        RightArrow,
        UpArrow,
        DownArrow,
        PageUp,
        PageDown,
        Home,
        End,
        Insert,
        Delete,
        Backspace,
        Space,
        Enter,
        Escape,
        A,         // for text edit CTRL+A: select all
        C,         // for text edit CTRL+C: copy
        V,         // for text edit CTRL+V: paste
        X,         // for text edit CTRL+X: cut
        Y,         // for text edit CTRL+Y: redo
        Z,         // for text edit CTRL+Z: undo
        COUNT
    };

    public enum class ImGuiCol_
    {
        Text,
        TextDisabled,
        WindowBg,              // Background of normal windows
        ChildBg,               // Background of child windows
        PopupBg,               // Background of popups, menus, tooltips windows
        Border,
        BorderShadow,
        FrameBg,               // Background of checkbox, radio button, plot, slider, text input
        FrameBgHovered,
        FrameBgActive,
        TitleBg,
        TitleBgActive,
        TitleBgCollapsed,
        MenuBarBg,
        ScrollbarBg,
        ScrollbarGrab,
        ScrollbarGrabHovered,
        ScrollbarGrabActive,
        CheckMark,
        SliderGrab,
        SliderGrabActive,
        Button,
        ButtonHovered,
        ButtonActive,
        Header,
        HeaderHovered,
        HeaderActive,
        Separator,
        SeparatorHovered,
        SeparatorActive,
        ResizeGrip,
        ResizeGripHovered,
        ResizeGripActive,
        CloseButton,
        CloseButtonHovered,
        CloseButtonActive,
        PlotLines,
        PlotLinesHovered,
        PlotHistogram,
        PlotHistogramHovered,
        TextSelectedBg,
        ModalWindowDarkening,  // darken entire screen when a modal window is active
        DragDropTarget,
        COUNT
    };

    public enum class ImGuiStyleVar_
    {
        // Enum name ......................// Member in ImGuiStyle structure (see ImGuiStyle for descriptions)
        Alpha,               // float     Alpha
        WindowPadding,       // ImVec2    WindowPadding
        WindowRounding,      // float     WindowRounding
        WindowBorderSize,    // float     WindowBorderSize
        WindowMinSize,       // ImVec2    WindowMinSize
        ChildRounding,       // float     ChildRounding
        ChildBorderSize,     // float     ChildBorderSize
        PopupRounding,       // float     PopupRounding
        PopupBorderSize,     // float     PopupBorderSize
        FramePadding,        // ImVec2    FramePadding
        FrameRounding,       // float     FrameRounding
        FrameBorderSize,     // float     FrameBorderSize
        ItemSpacing,         // ImVec2    ItemSpacing
        ItemInnerSpacing,    // ImVec2    ItemInnerSpacing
        IndentSpacing,       // float     IndentSpacing
        GrabMinSize,         // float     GrabMinSize
        ButtonTextAlign,     // ImVec2    ButtonTextAlign
        Count_
    };

    public enum class ImGuiMouseCursor_
    {
        None = -1,
        Arrow = 0,
        TextInput,         // When hovering over InputText, etc.
        ResizeAll,         // Unused by imgui functions
        ResizeNS,          // When hovering over an horizontal border
        ResizeEW,          // When hovering over a vertical border or a column
        ResizeNESW,        // When hovering over the bottom-left corner of a window
        ResizeNWSE,        // When hovering over the bottom-right corner of a window
        COUNT
    };

    public ref class ImGuiStyle
    {
    public:
        static Color GetColor(ImGuiCol_ idx);
        static void SetColor(ImGuiCol_ idx, Color value);

        static property Vector2 TouchExtraPadding { Vector2 get(); void set(Vector2 v); }

        static property Vector2 WindowPadding { Vector2 get(); void set(Vector2 v); }
        static property float WindowBorderSize { float get(); void set(float v); }
        static property float WindowRounding { float get(); void set(float v); }

        static property float PopupBorderSize { float get(); void set(float v); }
        static property float PopupRounding { float get(); void set(float v); }

        static property Vector2 FramePadding { Vector2 get(); void set(Vector2 v); }
        static property float FrameBorderSize { float get(); void set(float v); }
        static property float FrameRounding { float get(); void set(float v); }

        static property float ScrollbarSize { float get(); void set(float v); }
        static property float ScrollbarRounding { float get(); void set(float v); }

        static property float ChildBorderSize { float get(); void set(float v); }
        static property float ChildRounding { float get(); void set(float v); }

        static property float GrabRounding { float get(); void set(float v); }
        static property float GrabMinSize { float get(); void set(float v); }

        static property float ColumnsMinSpacing { float get(); void set(float v); }

        static property Vector2 ItemInnerSpacing { Vector2 get(); void set(Vector2 v); }
        static property Vector2 ItemSpacing { Vector2 get(); void set(Vector2 v); }
        static property float IndentSpacing { float get(); void set(float v); }
    };

    public ref class ImGuiIO
    {
    public:
        static property float DeltaTime { float get(); void set(float val); }
        static property float Time { float get(); }

        static bool IsKeyDown(int keyCode);
        static bool IsKeyUp(int keyCode);
        static bool IsKeyPressed(int keyCode);
        static int GetKeyIndex(ImGuiKey_ key);

        static void SetKeyMap(ImGuiKey_ key, int code);
        static void SetKeyState(int keyCode, bool state);
        static void SetKeyState(ImGuiKey_ key, bool state);

        static void SetMousePos(float x, float y);
        static void SetMouseButton(int btn, bool state);
        static void SetMouseWheel(float delta);

        static void AddText(unsigned short text);

        static property bool KeyShift { bool get(); void set(bool value); }
        static property bool KeyCtrl { bool get(); void set(bool value); }
        static property bool KeyAlt { bool get(); void set(bool value); }

        static property ImGuiMouseCursor_ MouseCursor { ImGuiMouseCursor_ get(); }
        static property bool WantCaptureMouse { bool get(); }
        static property bool WantCaptureKeyboard { bool get(); }
        static property bool WantTextInput { bool get(); }
    };

	public ref class ImGuiCli
	{
    public:
        static void PushStyleColor(ImGuiCol_ col, Color c);
        static void PopStyleColor() { PopStyleColor(1); }
        static void PopStyleColor(int val);
        static void PushStyleVar(ImGuiStyleVar_ var, float v);
        static void PushStyleVar(ImGuiStyleVar_ var, Vector2 v);
        static void PopStyleVar() { PopStyleVar(1); }
        static void PopStyleVar(int val);

        static bool Begin(System::String^ title, int windowFlags);
        static bool Begin(System::String^ title, bool% open, int windowFlags);
        static void End();
        static bool BeginChildFrame(System::String^ label, Vector2 size);
        static void EndChildFrame();
        static bool BeginChild(System::String^ id, Vector2 size) { return BeginChild(id, size, false); }
        static bool BeginChild(System::String^ id, Vector2 size, bool border);
        static void EndChild();

        static Vector2 GetContentRegionMax();
        static Vector2 GetContentRegionAvail();
        static float GetContentRegionAvailWidth();
        static Vector2 GetWindowContentRegionMin();
        static Vector2 GetWindowContentRegionMax();
        static float GetWindowContentRegionWidth();
        static Vector2 GetWindowPos();
        static Vector2 GetWindowSize();
        static float GetWindowWidth();
        static float GetWindowHeight();
        static bool IsWindowCollapsed();
        static bool IsWindowAppearing();

        static float GetScrollX();
        static float GetScrollY();
        static float GetScrollMaxX();
        static float GetScrollMaxY();
        static void SetScrollX(float scroll_x);
        static void SetScrollY(float scroll_y);
        static void SetScrollHere() { SetScrollHere(0.5f); }
        static void SetScrollHere(float center_y_ratio);

        static bool CollapsingHeader(System::String^ title);
        static bool CollapsingHeader(System::String^ title, bool% opened);

        static bool InputText(System::String^ label, System::String^% text) { return InputText(label, text, 0); }
        static bool InputText(System::String^ label, System::String^% text, int flags);
        static bool InputTextMultiline(System::String^ label, System::String^% text) { return InputTextMultiline(label, text, Vector2(0,0)); }
        static bool InputTextMultiline(System::String^ label, System::String^% text, Vector2 size) { return InputTextMultiline(label, text, size, 0); }
        static bool InputTextMultiline(System::String^ label, System::String^% text, Vector2 size, int flags);
        static bool InputTextMultiline_Barbaric(System::String^ label, System::String^% text, int capacity) { return InputTextMultiline_Barbaric(label, text, capacity, Vector2(0, 0)); }
        static bool InputTextMultiline_Barbaric(System::String^ label, System::String^% text, int capacity, Vector2 size) { return InputTextMultiline_Barbaric(label, text, capacity, size, 0); }
        static bool InputTextMultiline_Barbaric(System::String^ label, System::String^% text, int capacity, Vector2 size, int flags);
        
        static bool InputInt(System::String^ label, int% val) { return InputInt(label, val, 0, 100, 0); }
        static bool InputInt(System::String^ label, int% val, int step, int stepFast) { return InputInt(label, val, step, stepFast, 0); }
        static bool InputInt(System::String^ label, int% val, int step, int stepFast, int flags);
        static bool DragInt(System::String^ label, int% val) { return DragInt(label, val, 1, 0, 0); }
        static bool DragInt(System::String^ label, int% val, int step, int min, int max);

        static bool InputFloat(System::String^ label, float% val) { return InputFloat(label, val, 0.0f, 0.0f, -1, 0); }
        static bool InputFloat(System::String^ label, float% val, float step, float stepFast) { return InputFloat(label, val, step, stepFast, -1, 0); }
        static bool InputFloat(System::String^ label, float% val, float step, float stepFast, int decimPrec, int flags);
        static bool DragFloat(System::String^ label, float% val) { return DragFloat(label, val, 1.0f, 0.0f, 0.0f); }
        static bool DragFloat(System::String^ label, float% val, float step, float min, float max);

        static bool InputFloat2(System::String^ label, Vector2% v);
        static bool InputFloat3(System::String^ label, Vector3% v);
        static bool InputFloat4(System::String^ label, Vector4% v);
        static bool DragFloat2(System::String^ label, Vector2% v) { return DragFloat2(label, v, 1.0f, 0.0f, 0.0f); }
        static bool DragFloat3(System::String^ label, Vector3% v) { return DragFloat3(label, v, 1.0f, 0.0f, 0.0f); }
        static bool DragFloat4(System::String^ label, Vector4% v) { return DragFloat4(label, v, 1.0f, 0.0f, 0.0f); }
        static bool DragFloat2(System::String^ label, Vector2% v, float speed) { return DragFloat2(label, v, speed, 0.0f, 0.0f); }
        static bool DragFloat3(System::String^ label, Vector3% v, float speed) { return DragFloat3(label, v, speed, 0.0f, 0.0f); }
        static bool DragFloat4(System::String^ label, Vector4% v, float speed) { return DragFloat4(label, v, speed, 0.0f, 0.0f); }
        static bool DragFloat2(System::String^ label, Vector2% v, float speed, float min, float max);
        static bool DragFloat3(System::String^ label, Vector3% v, float speed, float min, float max);
        static bool DragFloat4(System::String^ label, Vector4% v, float speed, float min, float max);
        static bool InputColor(System::String^ label, Color% v);

        static bool Selectable(System::String^ txt, bool selected);

        // Popups
        static void OpenPopup(System::String^ label);
        static bool BeginPopup(System::String^ label, int flags);
        static void EndPopup();
        static bool IsPopupOpen(System::String^ label);
        static void CloseCurrentPopup();

        // Drag and drop
        static bool BeginDragDropSource();
        static void EndDragDropSource();
        static bool BeginDragDropTarget();
        static void EndDragDropTarget();

        // Utilities
        static bool IsItemHovered();
        static bool IsItemActive();
        static bool IsItemClicked(int btn);
        static bool IsItemVisible();
        static bool IsAnyItemHovered();
        static bool IsAnyItemActive();
        static Vector2 GetItemRectMin();
        static Vector2 GetItemRectMax();
        static Vector2 GetItemRectSize();
        static float GetItemWidth();
        static void PushItemWidth(float w);
        static void PopItemWidth();
        static bool IsWindowFocused();
        static bool IsWindowHovered();
        static float GetTime();
        static Vector2 CalcTextSize(System::String^ label);

        static void Separator();
        static void SameLine() { SameLine(0.0f, -1.0f); }
        static void SameLine(float px) { SameLine(px, -1.0f); }
        static void SameLine(float px, float spacing_w);
        static void NewLine();
        static void Spacing();
        static void Dummy(Vector2 v);
        static void Indent() { Indent(0.0f); }
        static void Indent(float w);
        static void Unindent() { Unindent(0.0f); }
        static void Unindent(float w);
        static void BeginGroup();
        static void EndGroup();

        static Vector2       GetCursorPos();
        static float         GetCursorPosX();
        static float         GetCursorPosY();
        static void          SetCursorPos(Vector2 local_pos);
        static void          SetCursorPosX(float x);
        static void          SetCursorPosY(float y);
        static Vector2       GetCursorStartPos();
        static Vector2       GetCursorScreenPos();
        static void          SetCursorScreenPos(Vector2 pos);
        static void          AlignTextToFramePadding();
        static float         GetTextLineHeight();
        static float         GetTextLineHeightWithSpacing();
        static float         GetFrameHeight();
        static float         GetFrameHeightWithSpacing();

        static void PushID(System::String^ label);
        static void PushID(System::Object^ obj);
        static void PushID(int id);
        static void PopID();
        
        static void Label(System::String^ label, System::String^ text);
        static void Text(System::String^ text);
        static bool Button(System::String^ label);
        static bool Button(System::String^ label, Vector2 size);
        static bool ArrowButton(System::String^ id, ImGuiDir_ dir);
        static bool InvisibleButon(System::String^ label, Vector2 size);
        static void Image(Microsoft::Xna::Framework::Graphics::Texture2D^ texture, Vector2 size);
        static void Image(Microsoft::Xna::Framework::Graphics::Texture2D^ texture, Vector2 size, Vector2 uv0, Vector2 uv1);
        static void Image(Microsoft::Xna::Framework::Graphics::Texture2D^ texture, Vector2 size, Vector2 uv0, Vector2 uv1, Color tint);
        static bool ImageButton(Microsoft::Xna::Framework::Graphics::Texture2D^ texture, Vector2 size);
        static bool ImageButton(Microsoft::Xna::Framework::Graphics::Texture2D^ texture, Vector2 size, Vector2 uv0, Vector2 uv1);
        static bool Checkbox(System::String^ label, bool% selected);
        static bool RadioButton(System::String^ label, bool selected);
        static bool BeginCombo(System::String^ label, System::String^ preview);
        static void EndCombo();
        static bool Combo(System::String^ label, int% currentItem, array<System::String^>^ items) { return Combo(label, currentItem, items, 0); }
        static bool Combo(System::String^ label, int% currentItem, array<System::String^>^ items, int flags);
        /// Items must have ToString() to be meaningful.
        static bool Combo(System::String^ label, int% currentItem, array<System::Object^>^ items) { return Combo(label, currentItem, items, 0); }
        static bool Combo(System::String^ label, int% currentItem, array<System::Object^>^ items, int flags);
        static bool ListBoxHeader(System::String^ label, Vector2 size);
        static bool ListBoxHeader(System::String^ label, int itemCount, int heightInItems);
        static bool ListBox(System::String^ label, int% currentItem, array<System::String^>^ items);
        /// Items must have ToString() to be meaningful.
        static bool ListBox(System::String^ label, int% currentItem, array<System::Object^>^ items);
        static void ListBoxFooter();
        static void Bullet();

        static void Columns(int ct);
        static void NextColumn();

        static void SetTooltip(System::String^ label);
        static void BeginTooltip();
        static void EndTooltip();

        static bool BeginMainMenuBar();
        static void EndMainMenuBar();
        static bool BeginMenuBar();
        static void EndMenuBar();
        static bool BeginMenu(System::String^ label, bool enabled);
        static void EndMenu();
        static bool MenuItem(System::String^ label);
        static bool MenuItem(System::String^ label, bool% selected) { return MenuItem(label, selected, true); }
        static bool MenuItem(System::String^ label, bool% selected, bool enabled);
        static bool MenuItem(System::String^ label, System::String^ shortCut, bool% selected) { return MenuItem(label, shortCut, selected, true); }
        static bool MenuItem(System::String^ label, System::String^ shortCut, bool% selected, bool enabled);

        // Clipping
        static void PushClipRect(Vector2 min, Vector2 max, bool intersect);
        static void PopClipRect();

        // Tree
        static bool TreeNode(System::String^ label);
        static bool TreeNodeEx(System::String^ label, int flags);
        static void TreePop();
	};

    public ref class ImGuiEx
    {
    public:
        static bool RangeSliderFloat(System::String^ label, float% min, float% max, float vMin, float vMax);
        static bool BitField(System::String^ label, unsigned% bits);
        static bool BitField(System::String^ label, unsigned% bits, unsigned% hitBit);
        static bool DragFloatN_Colored(System::String^ label, Vector2% v) { return DragFloatN_Colored(label, v, 1.0f, 0.0f, 0.0f); }
        static bool DragFloatN_Colored(System::String^ label, Vector3% v) { return DragFloatN_Colored(label, v, 1.0f, 0.0f, 0.0f); }
        static bool DragFloatN_Colored(System::String^ label, Vector4% v) { return DragFloatN_Colored(label, v, 1.0f, 0.0f, 0.0f); }
        static bool DragFloatN_Colored(System::String^ label, Vector2% v, float speed) { return DragFloatN_Colored(label, v, speed, 0.0f, 0.0f); }
        static bool DragFloatN_Colored(System::String^ label, Vector3% v, float speed) { return DragFloatN_Colored(label, v, speed, 0.0f, 0.0f); }
        static bool DragFloatN_Colored(System::String^ label, Vector4% v, float speed) { return DragFloatN_Colored(label, v, speed, 0.0f, 0.0f); }
        static bool DragFloatN_Colored(System::String^ label, Vector2% v, float speed, float min, float max);
        static bool DragFloatN_Colored(System::String^ label, Vector3% v, float speed, float min, float max);
        static bool DragFloatN_Colored(System::String^ label, Vector4% v, float speed, float min, float max);

        static void BeginTabBar(System::String^ id);
        static void EndTabBar();
        static bool TabItem(System::String^ label, bool% open);
        static void SetTabItemSelected(System::String^ label);

        static void PushFont(int idx);
        static void PopFont();
        static void PushBoldFont();
        static void PushLargeFont();
        static void PushLargeBoldFont();
    };

    [System::Flags]
    public enum class DockFlags
    {
        NoTabs = 1,
        StartLeft = 1 << 1,
        StartRight = 1 << 2,
        StartTop = 1 << 3,
        StartBottom = 1 << 4,
        NoPad = 1 << 5,
        Hidden = 1 << 6,
        NoCloseButton = 1 << 7
    };

    public ref class ImGuiDock
    {
    public:
        static void RootDock(Vector2 pos, Vector2 size);
        static void ShutdownDock();
        static bool BeginDock(System::String^ label) { return BeginDock(label, 0); }
        static bool BeginDock(System::String^ label, int windowFlags) { return BeginDock(label, 0, 0); }
        static bool BeginDock(System::String^ label, int windowFlags, int dockFlags);
        static bool BeginDock(System::String^ label, bool% opened, int windowFlags, int dockFlags);
        static void EndDock();
        static void SetDockActive();
        static void LoadDock();
        static void SaveDock();
    };
}
