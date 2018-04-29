// This is the main DLL file.

#include "ImGuiCLI.h"

#include "imgui.h"
#include "imgui_dock.h"
#include "imgui_ext.h"
#include "imgui_internal.h"
#include "imgui_tabs.h"

#include <string>

using namespace System::Runtime::InteropServices;

inline int GetPointer(System::Object^ obj)
{
    void* voidPtr = GCHandle::ToIntPtr(GCHandle::Alloc(obj)).ToPointer();
    return (intptr_t)voidPtr;
}

inline std::string ToSTLString(System::String^ str)
{
    IntPtr p = Marshal::StringToHGlobalAnsi(str);
    const char* linkStr = static_cast<char*>(p.ToPointer());
    std::string ret(linkStr);
    Marshal::FreeHGlobal(p);
    return ret;
}

inline void CopyStrBuff(System::String^ str, char* target)
{
    IntPtr p = Marshal::StringToHGlobalAnsi(str);
    const char* linkStr = static_cast<char*>(p.ToPointer());
    memcpy(target, linkStr, strlen(linkStr));
    Marshal::FreeHGlobal(p);
}

namespace ImGuiCLI
{

    static char BUFF[4096];
#define RESET_TBUF() memset(BUFF, 0, 4096)
#define COPY_TBUF(VAR) CopyStrBuff(VAR, BUFF)
#define LBL ToSTLString(label).c_str()

    void ImGuiCli::PushStyleColor(ImGuiCol_ col, Color c) { 
        auto v = c.ToVector4();
        ImGui::PushStyleColor((int)col, ImVec4(v.X, v.Y, v.Z, v.W));
    }
    void ImGuiCli::PopStyleColor(int val) { ImGui::PopStyleColor(val); }
    void ImGuiCli::PushStyleVar(ImGuiStyleVar_ var, float v) { ImGui::PushStyleVar((int)var, v); }
    void ImGuiCli::PushStyleVar(ImGuiStyleVar_ var, Vector2 v) { ImGui::PushStyleVar((int)var, ImVec2(v.X, v.Y)); }
    void ImGuiCli::PopStyleVar(int val) { ImGui::PopStyleVar(val); }

    bool ImGuiCli::Begin(System::String^ title, int winFlags)
    {
        return ImGui::Begin(ToSTLString(title).c_str(), 0x0, winFlags);
    }
    bool ImGuiCli::Begin(System::String^ title, bool% open, int winFlags)
    {
        bool o = open;
        bool ret = ImGui::Begin(ToSTLString(title).c_str(), &o, winFlags);
        open = o;
        return ret;
    }
    void ImGuiCli::End()
    {
        return ImGui::End();
    }

    bool ImGuiCli::BeginChildFrame(System::String^ label, Vector2 size)
    {
        //return ImGui::BeginChildFrame(ImGui::MakeID(LBL, ImVec2(size.X, size.Y));
        return false;
    }
    void ImGuiCli::EndChildFrame()
    {
        ImGui::EndChildFrame();
    }

    bool ImGuiCli::BeginChild(System::String^ label, Vector2 size, bool border) { return ImGui::BeginChild(LBL, ImVec2(size.X, size.Y), border); }
    void ImGuiCli::EndChild() { ImGui::EndChild(); }

    Vector2 ImGuiCli::GetContentRegionMax()
    {
        auto v = ImGui::GetContentRegionMax();
        return Vector2(v.x, v.y);
    }
    Vector2 ImGuiCli::GetContentRegionAvail()
    {
        auto v = ImGui::GetContentRegionAvail();
        return Vector2(v.x, v.y);
    }
    float ImGuiCli::GetContentRegionAvailWidth() { return ImGui::GetContentRegionAvailWidth(); }
    Vector2 ImGuiCli::GetWindowContentRegionMin()
    {
        auto v = ImGui::GetWindowContentRegionMin();
        return Vector2(v.x, v.y);
    }
    Vector2 ImGuiCli::GetWindowContentRegionMax()
    {
        auto v = ImGui::GetWindowContentRegionMax();
        return Vector2(v.x, v.y);
    }
    float ImGuiCli::GetWindowContentRegionWidth() { return ImGui::GetWindowContentRegionWidth(); }
    Vector2 ImGuiCli::GetWindowPos()
    {
        auto v = ImGui::GetWindowPos();
        return Vector2(v.x, v.y);
    }
    Vector2 ImGuiCli::GetWindowSize()
    {
        auto v = ImGui::GetWindowSize();
        return Vector2(v.x, v.y);
    }
    float ImGuiCli::GetWindowWidth() { return ImGui::GetWindowWidth(); }
    float ImGuiCli::GetWindowHeight() { return ImGui::GetWindowHeight(); }
    bool ImGuiCli::IsWindowCollapsed() { return ImGui::IsWindowCollapsed(); }
    bool ImGuiCli::IsWindowAppearing() { return ImGui::IsWindowAppearing(); }

    float ImGuiCli::GetScrollX() { return ImGui::GetScrollX(); }
    float ImGuiCli::GetScrollY() { return ImGui::GetScrollY(); }
    float ImGuiCli::GetScrollMaxX() { return ImGui::GetScrollMaxX(); }
    float ImGuiCli::GetScrollMaxY() { return ImGui::GetScrollMaxY(); }
    void ImGuiCli::SetScrollX(float scroll_x) { ImGui::SetScrollX(scroll_x); }
    void ImGuiCli::SetScrollY(float scroll_y) { ImGui::SetScrollY(scroll_y); }
    void ImGuiCli::SetScrollHere(float center_y_ratio) { ImGui::SetScrollHere(center_y_ratio); }

    bool ImGuiCli::CollapsingHeader(System::String^ label) { return ImGui::CollapsingHeader(LBL); }
    bool ImGuiCli::CollapsingHeader(System::String^ label, bool% opened) {
        bool v = opened;
        if (ImGui::CollapsingHeader(LBL, &v))
        {
            opened = v;
            return true;
        }
        return false;
    }

    bool ImGuiCli::InputText(System::String^ label, System::String^% text, int flags)
    {
        COPY_TBUF(text);
        if (ImGui::InputText(ToSTLString(label).c_str(), BUFF, 4096, flags))
        {
            text = gcnew System::String(BUFF);
            return true;
        }
        return false;
    }

    bool ImGuiCli::InputTextMultiline(System::String^ label, System::String^% text, Vector2 size, int flags)
    {
        COPY_TBUF(text);
        if (ImGui::InputTextMultiline(LBL, BUFF, 4096, ImVec2(size.X, size.Y), flags))
        {
            text = gcnew System::String(BUFF);
            return true;
        }
        return false;
    }

    bool ImGuiCli::InputTextMultiline_Barbaric(System::String^ label, System::String^% text, int capacity, Vector2 size, int flags)
    {
        // resize as needed
        static char* barbBuff = nullptr;
        static size_t barbBuffSize = 0;
        if (barbBuff == nullptr || barbBuffSize < capacity)
        {
            if (barbBuff)
                delete[] barbBuff;
            barbBuff = new char[barbBuffSize = capacity];
        }
        memset(barbBuff, 0, barbBuffSize);
        CopyStrBuff(text, barbBuff);
        if (ImGui::InputTextMultiline(LBL, barbBuff, capacity, ImVec2(size.X, size.Y), flags))
        {
            text = gcnew System::String(barbBuff);
            return true;
        }
        return false;
    }

    bool ImGuiCli::InputInt(System::String^ label, int% val, int step, int stepFast, int flags)
    {
        int v = val;
        if (ImGui::InputInt(ToSTLString(label).c_str(), &v, step, stepFast, flags))
        {
            val = v;
            return true;
        }
        return false;
    }

    bool ImGuiCli::DragInt(System::String^ label, int% val, int step, int min, int max)
    {
        int v = val;
        if (ImGui::DragInt(ToSTLString(label).c_str(), &v, step, min, max))
        {
            val = v;
            return true;
        }
        return false;
    }

    bool ImGuiCli::InputFloat(System::String^ label, float% val, float step, float stepFast, int decimPrec, int flags)
    {
        float v = val;
        if (ImGui::InputFloat(ToSTLString(label).c_str(), &v, step, stepFast, decimPrec, flags))
        {
            val = v;
            return true;
        }
        return false;
    }

    bool ImGuiCli::DragFloat(System::String^ label, float% val, float step, float min, float max)
    {
        float v = val;
        if (ImGui::DragFloat(LBL, &v, step, min, max))
        {
            val = v;
            return true;
        }
        return false;
    }

    bool ImGuiCli::InputFloat2(System::String^ label, Vector2% v)
    {
        Vector2 val = v;
        if (ImGui::InputFloat2(LBL, &val.X))
        {
            v = val;
            return true;
        }
        return false;
    }

    bool ImGuiCli::InputFloat3(System::String^ label, Vector3% val)
    {
        Vector3 v = val;
        if (ImGui::InputFloat3(LBL, &v.X))
        {
            val = v;
            return true;
        }
        return false;
    }

    bool ImGuiCli::InputFloat4(System::String^ label, Vector4% val)
    {
        Vector4 v = val;
        if (ImGui::InputFloat4(LBL, &v.X))
        {
            val = v;
            return true;
        }
        return false;
    }

    bool ImGuiCli::DragFloat2(System::String^ label, Vector2% val, float speed, float min, float max) {
        Vector2 v = val;
        if (ImGui::DragFloat2(LBL, &v.X, speed, min, max))
        {
            val = v;
            return true;
        }
        return false;
    }

    bool ImGuiCli::DragFloat3(System::String^ label, Vector3% val, float speed, float min, float max) {
        Vector3 v = val;
        if (ImGui::DragFloat3(LBL, &v.X, speed, min, max))
        {
            val = v;
            return true;
        }
        return false;
    }
    
    bool ImGuiCli::DragFloat4(System::String^ label, Vector4% val, float speed, float min, float max) {
        Vector4 v = val;
        if (ImGui::DragFloat4(LBL, &v.X, speed, min, max))
        {
            val = v;
            return true;
        }
        return false;
    }

    bool ImGuiCli::InputColor(System::String^ label, Color% v)
    {
        auto col = v.ToVector4();
        if (ImGui::ColorEdit4(LBL, &col.X, ImGuiColorEditFlags_AlphaBar))
        {
            v = Color(col.X, col.Y, col.Z, col.W);
            return true;
        }
        return false;
    }

    bool ImGuiCli::Selectable(System::String^ label, bool selected) { return ImGui::Selectable(LBL, selected); }

    // Popups
    void ImGuiCli::OpenPopup(System::String^ label) { ImGui::OpenPopup(LBL); }
    bool ImGuiCli::BeginPopup(System::String^ label, int flags) { return ImGui::BeginPopup(LBL, flags); }    
    void ImGuiCli::EndPopup() { ImGui::EndPopup(); }
    bool ImGuiCli::IsPopupOpen(System::String^ label) { return ImGui::IsPopupOpen(LBL); }
    void ImGuiCli::CloseCurrentPopup() { ImGui::CloseCurrentPopup(); }

    // Drag and drop
    bool ImGuiCli::BeginDragDropSource() { return ImGui::BeginDragDropSource(); }
    void ImGuiCli::EndDragDropSource() { ImGui::EndDragDropSource(); }
    bool ImGuiCli::BeginDragDropTarget() { return ImGui::BeginDragDropTarget(); }
    void ImGuiCli::EndDragDropTarget() { ImGui::EndDragDropTarget(); }

    bool ImGuiCli::IsItemHovered() { return ImGui::IsItemHovered(); }
    bool ImGuiCli::IsItemActive() { return ImGui::IsItemActive(); }
    bool ImGuiCli::IsItemClicked(int btn) { return ImGui::IsItemClicked(btn); }
    bool ImGuiCli::IsItemVisible() { return ImGui::IsItemVisible(); }
    bool ImGuiCli::IsAnyItemHovered() { return ImGui::IsAnyItemHovered(); }
    bool ImGuiCli::IsAnyItemActive() { return ImGui::IsAnyItemActive(); }

    Vector2 ImGuiCli::GetItemRectMin()
    {
        auto v = ImGui::GetItemRectMin();
        return Vector2(v.x, v.y);
    }
    Vector2 ImGuiCli::GetItemRectMax()
    {
        auto v = ImGui::GetItemRectMax();
        return Vector2(v.x, v.y);
    }
    Vector2 ImGuiCli::GetItemRectSize()
    {
        auto v = ImGui::GetItemRectSize();
        return Vector2(v.x, v.y);
    }

    float ImGuiCli::GetItemWidth() { return ImGui::GetItemRectSize().x; }
    void ImGuiCli::PushItemWidth(float w) { ImGui::PushItemWidth(w); }
    void ImGuiCli::PopItemWidth() { ImGui::PopItemWidth(); }
    bool ImGuiCli::IsWindowFocused() { return ImGui::IsWindowFocused(); }
    bool ImGuiCli::IsWindowHovered() { return ImGui::IsWindowHovered(); }
    float ImGuiCli::GetTime() { return ImGui::GetTime(); }
    Vector2 ImGuiCli::CalcTextSize(System::String^ label)
    {
        auto v = ImGui::CalcTextSize(LBL);
        return Vector2(v.x, v.y);
    }

    void ImGuiCli::Separator() { ImGui::Separator(); }

    void ImGuiCli::SameLine(float px, float spacing_w) { ImGui::SameLine(px, spacing_w); }
    void ImGuiCli::NewLine() { ImGui::NewLine(); }
    void ImGuiCli::Spacing() { ImGui::Spacing(); }
    void ImGuiCli::Dummy(Vector2 v) { ImGui::Dummy(ImVec2(v.X, v.Y)); }
    void ImGuiCli::Indent(float w) { ImGui::Indent(w); }
    void ImGuiCli::Unindent(float w) { ImGui::Unindent(w); }
    void ImGuiCli::BeginGroup() { ImGui::BeginGroup(); }
    void ImGuiCli::EndGroup() { ImGui::EndGroup(); }

    Vector2 ImGuiCli::GetCursorPos() { auto v = ImGui::GetCursorPos(); return Vector2(v.x, v.y); }
    float   ImGuiCli::GetCursorPosX() { return ImGui::GetCursorPosX(); }
    float   ImGuiCli::GetCursorPosY() { return ImGui::GetCursorPosY(); }
    void    ImGuiCli::SetCursorPos(Vector2 local_pos) { ImGui::SetCursorPos(ImVec2(local_pos.X, local_pos.Y)); }
    void    ImGuiCli::SetCursorPosX(float x) { ImGui::SetCursorPosX(x); }
    void    ImGuiCli::SetCursorPosY(float y) { ImGui::SetCursorPosX(y); }
    Vector2 ImGuiCli::GetCursorStartPos() { auto v = ImGui::GetCursorStartPos(); return Vector2(v.x, v.y); }
    Vector2 ImGuiCli::GetCursorScreenPos() { auto v = ImGui::GetCursorScreenPos(); return Vector2(v.x, v.y); }
    void    ImGuiCli::SetCursorScreenPos(Vector2 pos) { ImGui::SetCursorScreenPos(ImVec2(pos.X, pos.Y)); }
    void    ImGuiCli::AlignTextToFramePadding() { ImGui::AlignTextToFramePadding(); }
    float   ImGuiCli::GetTextLineHeight() { return ImGui::GetTextLineHeight(); }
    float   ImGuiCli::GetTextLineHeightWithSpacing() { return ImGui::GetTextLineHeightWithSpacing(); }
    float   ImGuiCli::GetFrameHeight() { return ImGui::GetFrameHeight(); }
    float   ImGuiCli::GetFrameHeightWithSpacing() { return ImGui::GetFrameHeightWithSpacing(); }

    void ImGuiCli::PushID(System::String^ label) { ImGui::PushID(LBL); }
    void ImGuiCli::PushID(System::Object^ obj) { PushID(GetPointer(obj)); }
    void ImGuiCli::PushID(int id) { ImGui::PushID(id); }
    void ImGuiCli::PopID() { ImGui::PopID(); }

    void ImGuiCli::Label(System::String^ label, System::String^ text) { ImGui::LabelText(LBL, ToSTLString(text).c_str()); }
    void ImGuiCli::Text(System::String^ label) { return ImGui::Text(LBL); }
    bool ImGuiCli::Button(System::String^ label) { return ImGui::Button(LBL); }
    bool ImGuiCli::Button(System::String^ label, Vector2 size) { return ImGui::Button(LBL, ImVec2(size.X, size.Y)); }
    bool ImGuiCli::ArrowButton(System::String^ label, ImGuiDir_ dir) { return ImGui::ArrowButton(LBL, (ImGuiDir)dir); }
    bool ImGuiCli::InvisibleButon(System::String^ label, Vector2 size) { return ImGui::InvisibleButton(LBL, ImVec2(size.X, size.Y)); }
    void ImGuiCli::Image(Microsoft::Xna::Framework::Graphics::Texture2D^ texture, Vector2 size)
    {
        ImGui::Image(texture->GetNativeHandle().ToPointer(), ImVec2(size.X, size.Y));
    }
    void ImGuiCli::Image(Microsoft::Xna::Framework::Graphics::Texture2D^ texture, Vector2 size, Vector2 uv0, Vector2 uv1)
    {
        ImGui::Image(texture->GetNativeHandle().ToPointer(), ImVec2(size.X, size.Y), ImVec2(uv0.X, uv0.Y), ImVec2(uv1.X, uv1.Y));
    }
    void ImGuiCli::Image(Microsoft::Xna::Framework::Graphics::Texture2D^ texture, Vector2 size, Vector2 uv0, Vector2 uv1, Color tint)
    {
        ImGui::Image(texture->GetSharedHandle().ToPointer(), ImVec2(size.X, size.Y), ImVec2(uv0.X, uv0.Y), ImVec2(uv1.X, uv1.Y), ImColor(tint.R, tint.G, tint.B, tint.G));
    }
    bool ImGuiCli::ImageButton(Microsoft::Xna::Framework::Graphics::Texture2D^ texture, Vector2 size)
    {
        return ImGui::ImageButton(texture->GetNativeHandle().ToPointer(), ImVec2(size.X, size.Y));
    }
    bool ImGuiCli::ImageButton(Microsoft::Xna::Framework::Graphics::Texture2D^ texture, Vector2 size, Vector2 uv0, Vector2 uv1)
    {
        return ImGui::ImageButton(texture->GetNativeHandle().ToPointer(), ImVec2(size.X, size.Y), ImVec2(uv0.X, uv0.Y), ImVec2(uv1.X, uv1.Y));
    }
    bool ImGuiCli::Checkbox(System::String^ label, bool% selected) { 
        bool sel = selected;
        if (ImGui::Checkbox(LBL, &sel))
        {
            selected = sel;
            return true;
        }
        return false;
    }
    bool ImGuiCli::RadioButton(System::String^ label, bool selected) { return ImGui::RadioButton(LBL, selected);  }
    bool ImGuiCli::BeginCombo(System::String^ label, System::String^ preview) { return ImGui::BeginCombo(LBL, ToSTLString(preview).c_str()); }
    void ImGuiCli::EndCombo() { ImGui::EndCombo(); }

    bool ImGuiCli::Combo(System::String^ label, int% currentItem, array<System::String^>^ items, int flags)
    {
        if (!ImGui::BeginCombo(LBL, ToSTLString(items[currentItem]).c_str(), flags))
            return false;

        bool value_changed = false;
        for (int i = 0; i < items->Length; i++)
        {
            ImGui::PushID((void*)(intptr_t)i);
            const bool item_selected = (i == currentItem);
            if (ImGui::Selectable(ToSTLString(items[i]).c_str(), item_selected))
            {
                value_changed = true;
                currentItem = i;
            }
            if (item_selected)
                ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }

        ImGui::EndCombo();
        return value_changed;
    }
    
    bool ImGuiCli::Combo(System::String^ label, int% currentItem, array<System::Object^>^ items, int flags)
    {
        if (!ImGui::BeginCombo(LBL, ToSTLString(items[currentItem]->ToString()).c_str(), flags))
            return false;

        bool value_changed = false;
        for (int i = 0; i < items->Length; i++)
        {
            ImGui::PushID((void*)(intptr_t)i);
            const bool item_selected = (i == currentItem);
            if (ImGui::Selectable(ToSTLString(items[i]->ToString()).c_str(), item_selected))
            {
                value_changed = true;
                currentItem = i;
            }
            if (item_selected)
                ImGui::SetItemDefaultFocus();
            ImGui::PopID();
        }

        ImGui::EndCombo();
        return value_changed;
    }

    bool ImGuiCli::ListBoxHeader(System::String^ label, Vector2 size)
    {
        return ImGui::ListBoxHeader(LBL, ImVec2(size.X, size.Y));
    }
    bool ImGuiCli::ListBoxHeader(System::String^ label, int itemCount, int heightInItems)
    {
        return ImGui::ListBoxHeader(LBL, itemCount, heightInItems);
    }
    bool ImGuiCli::ListBox(System::String^ label, int% currentItem, array<System::String^>^ items)
    {
        if (!ImGui::ListBoxHeader(LBL, items->Length, -1))
            return false;

        bool value_changed = false;
        for (int i = 0; i < items->Length; i++)
        {
            const bool item_selected = (i == currentItem);
                        
            ImGui::PushID(i);
            if (ImGui::Selectable(ToSTLString(items[i]).c_str(), item_selected))
            {
                currentItem = i;
                value_changed = true;
            }
            ImGui::PopID();
        }

        ImGui::ListBoxFooter();
        return value_changed;
    }
    bool ImGuiCli::ListBox(System::String^ label, int% currentItem, array<System::Object^>^ items)
    {
        if (!ImGui::ListBoxHeader(LBL, items->Length, -1))
            return false;

        bool value_changed = false;
        for (int i = 0; i < items->Length; i++)
        {
            const bool item_selected = (i == currentItem);

            ImGui::PushID(i);
            if (ImGui::Selectable(ToSTLString(items[i]->ToString()).c_str(), item_selected))
            {
                currentItem = i;
                value_changed = true;
            }
            ImGui::PopID();
        }

        ImGui::ListBoxFooter();
        return value_changed;
    }
    void ImGuiCli::ListBoxFooter()
    {
        ImGui::ListBoxFooter();
    }

    void ImGuiCli::Bullet() { ImGui::Bullet(); }

    void ImGuiCli::Columns(int ct) { ImGui::Columns(ct); }
    void ImGuiCli::NextColumn() { ImGui::NextColumn(); }

    void ImGuiCli::SetTooltip(System::String^ label) { ImGui::SetTooltip(LBL); }
    void ImGuiCli::BeginTooltip() { ImGui::BeginTooltip(); }
    void ImGuiCli::EndTooltip() { ImGui::EndTooltip(); }

    bool ImGuiCli::BeginMainMenuBar() { return ImGui::BeginMainMenuBar(); }
    void ImGuiCli::EndMainMenuBar() { ImGui::EndMainMenuBar(); }
    bool ImGuiCli::BeginMenuBar() { return ImGui::BeginMenuBar(); }
    void ImGuiCli::EndMenuBar() { ImGui::EndMenuBar(); }
    bool ImGuiCli::BeginMenu(System::String^ label, bool enabled) { return ImGui::BeginMenu(LBL, enabled); }
    void ImGuiCli::EndMenu() { ImGui::EndMenu(); }
    bool ImGuiCli::MenuItem(System::String^ label) { return ImGui::MenuItem(LBL); }
    bool ImGuiCli::MenuItem(System::String^ label, bool% selected, bool enabled) 
    {
        bool r = selected;
        if (ImGui::MenuItem(LBL, 0x0, &r, enabled))
        {
            selected = r;
            return true;
        }
        return false;
    }
    bool ImGuiCli::MenuItem(System::String^ label, System::String^ shortCut, bool% selected, bool enabled)
    {
        bool r = selected;
        if (ImGui::MenuItem(LBL, ToSTLString(shortCut).c_str(), &r, enabled))
        {
            selected = r;
            return true;
        }
        return false;
    }

    void ImGuiCli::PushClipRect(Vector2 min, Vector2 max, bool intersect) { ImGui::PushClipRect(ImVec2(min.X, min.Y), ImVec2(max.X, max.Y), intersect); }
    void ImGuiCli::PopClipRect() { ImGui::PopClipRect(); }

    // Tree
    bool ImGuiCli::TreeNode(System::String^ label) { return ImGui::TreeNode(LBL); }
    bool ImGuiCli::TreeNodeEx(System::String^ label, int flags) { return ImGui::TreeNodeEx(LBL, flags); }
    void ImGuiCli::TreePop() { ImGui::TreePop(); }

    bool ImGuiEx::RangeSliderFloat(System::String^ label, float% min, float% max, float vMin, float vMax)
    {
        float mi = min; float mx = max;
        if (ImGui::RangeSliderFloat(LBL, &mi, &mx, vMin, vMax))
        {
            min = mi; max = mx;
            return true;
        }
        return false;
    }
    bool ImGuiEx::BitField(System::String^ label, unsigned% bits)
    {
        unsigned v = bits;
        if (ImGui::BitField(LBL, &v))
        {
            bits = v;
            return true;
        }
        return false;
    }
    bool ImGuiEx::BitField(System::String^ label, unsigned% bits, unsigned% hitBit)
    {
        unsigned v = bits, hit = 0;
        if (ImGui::BitField(LBL, &v, &hit))
        {
            bits = v;
            hitBit = hit;
            return true;
        }
        return false;
    }
    bool ImGuiEx::DragFloatN_Colored(System::String^ label, Vector2% val, float speed, float min, float max)
    {
        Vector2 v = val;
        if (ImGui::DragFloatN_Colored(LBL, &v.X, 2, speed, min, max))
        {
            val = v;
            return true;
        }
        return false;
    }
    bool ImGuiEx::DragFloatN_Colored(System::String^ label, Vector3% val, float speed, float min, float max)
    {
        Vector3 v = val;
        if (ImGui::DragFloatN_Colored(LBL, &v.X, 3, speed, min, max))
        {
            val = v;
            return true;
        }
        return false;
    }
    bool ImGuiEx::DragFloatN_Colored(System::String^ label, Vector4% val, float speed, float min, float max)
    {
        Vector4 v = val;
        if (ImGui::DragFloatN_Colored(LBL, &v.X, 4, speed, min, max))
        {
            val = v;
            return true;
        }
        return false;
    }
    void ImGuiEx::BeginTabBar(System::String^ label) { ImGui::BeginTabBar(LBL); }
    void ImGuiEx::EndTabBar() { ImGui::EndTabBar(); }
    bool ImGuiEx::TabItem(System::String^ label, bool% open) 
    { 
        bool v = open;
        if (ImGui::TabItem(LBL, &v))
        {
            open = v;
            return true;
        }
        return false;
    }
    void ImGuiEx::SetTabItemSelected(System::String^ label) { ImGui::SetTabItemSelected(LBL); }
    void ImGuiEx::PushFont(int idx) { ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[idx]); }
    void ImGuiEx::PopFont() { ImGui::PopFont(); }
    void ImGuiEx::PushBoldFont() { PushFont(2); }
    void ImGuiEx::PushLargeFont() { PushFont(1); }
    void ImGuiEx::PushLargeBoldFont() { PushFont(3); }
}

// ImGuiIO functions placed outside of ImGuiCLI namespace as fully qualifying the name makes collisions with
// DearImGui names less *casty*
bool ImGuiCLI::ImGuiIO::IsKeyDown(int keyCode) { return ImGui::IsKeyDown(keyCode); }
bool ImGuiCLI::ImGuiIO::IsKeyUp(int keyCode) { return !ImGui::IsKeyDown(keyCode); }
bool ImGuiCLI::ImGuiIO::IsKeyPressed(int keyCode) { return ImGui::IsKeyPressed(keyCode); }
int ImGuiCLI::ImGuiIO::GetKeyIndex(ImGuiKey_ key) { return ImGui::GetKeyIndex((int)key); }
void ImGuiCLI::ImGuiIO::SetKeyState(int keyCode, bool state) { ImGui::GetIO().KeysDown[keyCode] = state; }
void ImGuiCLI::ImGuiIO::SetKeyState(ImGuiKey_ key, bool state) { ImGui::GetIO().KeysDown[ImGui::GetKeyIndex((int)key)] = state; }
void ImGuiCLI::ImGuiIO::SetKeyMap(ImGuiKey_ key, int code) { ImGui::GetIO().KeyMap[(int)key] = code; }
void ImGuiCLI::ImGuiIO::SetMousePos(float x, float y) { ImGui::GetIO().MousePos = ImVec2(x, y); }
void ImGuiCLI::ImGuiIO::SetMouseButton(int btn, bool state) { ImGui::GetIO().MouseDown[btn] = state; }
void ImGuiCLI::ImGuiIO::SetMouseWheel(float delta) { ImGui::GetIO().MouseWheel = delta; }
float ImGuiCLI::ImGuiIO::DeltaTime::get() { return ImGui::GetIO().DeltaTime; }
float ImGuiCLI::ImGuiIO::Time::get() { return ImGui::GetCurrentContext()->Time; }
void ImGuiCLI::ImGuiIO::DeltaTime::set(float td) { ImGui::GetIO().DeltaTime = td; }
bool ImGuiCLI::ImGuiIO::KeyShift::get() { return ImGui::GetIO().KeyShift; }
void ImGuiCLI::ImGuiIO::KeyShift::set(bool val) { ImGui::GetIO().KeyShift = val; }
bool ImGuiCLI::ImGuiIO::KeyCtrl::get() { return ImGui::GetIO().KeyCtrl; }
void ImGuiCLI::ImGuiIO::KeyCtrl::set(bool val) { ImGui::GetIO().KeyCtrl = val; }
bool ImGuiCLI::ImGuiIO::KeyAlt::get() { return ImGui::GetIO().KeyAlt; }
void ImGuiCLI::ImGuiIO::KeyAlt::set(bool val) { ImGui::GetIO().KeyAlt = val; }
void ImGuiCLI::ImGuiIO::AddText(unsigned short text) { ImGui::GetIO().AddInputCharacter(text); }
ImGuiCLI::ImGuiMouseCursor_ ImGuiCLI::ImGuiIO::MouseCursor::get() { return (ImGuiMouseCursor_)ImGui::GetCurrentContext()->MouseCursor; }
bool ImGuiCLI::ImGuiIO::WantCaptureMouse::get() { return ImGui::GetIO().WantCaptureMouse; }
bool ImGuiCLI::ImGuiIO::WantCaptureKeyboard::get() { return ImGui::GetIO().WantCaptureKeyboard; }
bool ImGuiCLI::ImGuiIO::WantTextInput::get() { return ImGui::GetIO().WantTextInput; }

// Docking
void ImGuiCLI::ImGuiDock::RootDock(Vector2 pos, Vector2 size) { ImGui::RootDock(ImVec2(pos.X, pos.Y), ImVec2(size.X, size.Y)); }
void ImGuiCLI::ImGuiDock::ShutdownDock() { ImGui::ShutdownDock(); }
bool ImGuiCLI::ImGuiDock::BeginDock(System::String^ label, int windowFlags, int dockFlags) { return ImGui::BeginDock(LBL, 0x0, windowFlags, dockFlags); }
bool ImGuiCLI::ImGuiDock::BeginDock(System::String^ label, bool% opened, int flags, int dockFlags)
{
    bool t = opened;
    if (ImGui::BeginDock(LBL, &t, flags, dockFlags))
    {
        opened = t;
        return true;
    }
    return false;
}
void ImGuiCLI::ImGuiDock::EndDock() { ImGui::EndDock(); }
void ImGuiCLI::ImGuiDock::SetDockActive() { ImGui::SetDockActive(); }
void ImGuiCLI::ImGuiDock::LoadDock() { ImGui::LoadDock(); }
void ImGuiCLI::ImGuiDock::SaveDock() { ImGui::SaveDock(); }

/// ImGuiStyle
Vector2 ImGuiCLI::ImGuiStyle::WindowPadding::get()
{
    auto v = ImGui::GetStyle().WindowPadding;
    return Vector2(v.x, v.y);
}
void ImGuiCLI::ImGuiStyle::WindowPadding::set(Vector2 v)
{
    ImGui::GetStyle().WindowPadding = ImVec2(v.X, v.Y);
}
float ImGuiCLI::ImGuiStyle::WindowBorderSize::get() { return ImGui::GetStyle().WindowBorderSize; }
void ImGuiCLI::ImGuiStyle::WindowBorderSize::set(float v) { ImGui::GetStyle().WindowBorderSize = v; }
float ImGuiCLI::ImGuiStyle::WindowRounding::get() { return ImGui::GetStyle().WindowRounding; }
void ImGuiCLI::ImGuiStyle::WindowRounding::set(float v) { ImGui::GetStyle().WindowRounding = v; }

Vector2 ImGuiCLI::ImGuiStyle::FramePadding::get()
{
    auto v = ImGui::GetStyle().FramePadding;
    return Vector2(v.x, v.y);
}
void ImGuiCLI::ImGuiStyle::FramePadding::set(Vector2 v) { ImGui::GetStyle().FramePadding = ImVec2(v.X, v.Y); }
float ImGuiCLI::ImGuiStyle::FrameBorderSize::get() { return ImGui::GetStyle().FrameBorderSize; }
void ImGuiCLI::ImGuiStyle::FrameBorderSize::set(float v) { ImGui::GetStyle().FrameBorderSize = v; }
float ImGuiCLI::ImGuiStyle::FrameRounding::get() { return ImGui::GetStyle().FrameRounding; }
void ImGuiCLI::ImGuiStyle::FrameRounding::set(float v) { ImGui::GetStyle().FrameRounding = v; }

float ImGuiCLI::ImGuiStyle::ChildBorderSize::get() { return ImGui::GetStyle().ChildBorderSize; }
void ImGuiCLI::ImGuiStyle::ChildBorderSize::set(float v) { ImGui::GetStyle().ChildBorderSize = v; }
float ImGuiCLI::ImGuiStyle::ChildRounding::get() { return ImGui::GetStyle().ChildRounding; }
void ImGuiCLI::ImGuiStyle::ChildRounding::set(float v) { ImGui::GetStyle().ChildRounding = v; }

float ImGuiCLI::ImGuiStyle::GrabRounding::get() { return ImGui::GetStyle().GrabRounding; }
void ImGuiCLI::ImGuiStyle::GrabRounding::set(float v) { ImGui::GetStyle().GrabRounding = v; }
float ImGuiCLI::ImGuiStyle::GrabMinSize::get() { return ImGui::GetStyle().GrabMinSize; }
void ImGuiCLI::ImGuiStyle::GrabMinSize::set(float v) { ImGui::GetStyle().GrabMinSize = v; }

Vector2 ImGuiCLI::ImGuiStyle::ItemInnerSpacing::get()
{
    auto v = ImGui::GetStyle().ItemInnerSpacing;
    return Vector2(v.x, v.y);
}
void ImGuiCLI::ImGuiStyle::ItemInnerSpacing::set(Vector2 v)
{
    ImGui::GetStyle().ItemInnerSpacing = ImVec2(v.X, v.Y);
}
Vector2 ImGuiCLI::ImGuiStyle::ItemSpacing::get()
{
    auto v = ImGui::GetStyle().ItemSpacing;
    return Vector2(v.x, v.y);
}
void ImGuiCLI::ImGuiStyle::ItemSpacing::set(Vector2 v)
{
    ImGui::GetStyle().ItemSpacing = ImVec2(v.X, v.Y);
}
float ImGuiCLI::ImGuiStyle::IndentSpacing::get() { return ImGui::GetStyle().IndentSpacing; }
void ImGuiCLI::ImGuiStyle::IndentSpacing::set(float v) { ImGui::GetStyle().IndentSpacing = v; }

float ImGuiCLI::ImGuiStyle::ScrollbarRounding::get() { return ImGui::GetStyle().ScrollbarRounding; }
void ImGuiCLI::ImGuiStyle::ScrollbarRounding::set(float v) { ImGui::GetStyle().ScrollbarRounding = v; }
float ImGuiCLI::ImGuiStyle::ScrollbarSize::get() { return ImGui::GetStyle().ScrollbarSize; }
void ImGuiCLI::ImGuiStyle::ScrollbarSize::set(float v) { ImGui::GetStyle().ScrollbarSize = v; }

float ImGuiCLI::ImGuiStyle::PopupRounding::get() { return ImGui::GetStyle().PopupRounding; }
void ImGuiCLI::ImGuiStyle::PopupRounding::set(float v) { ImGui::GetStyle().PopupRounding = v; }
float ImGuiCLI::ImGuiStyle::PopupBorderSize::get() { return ImGui::GetStyle().PopupBorderSize; }
void ImGuiCLI::ImGuiStyle::PopupBorderSize::set(float v) { ImGui::GetStyle().PopupBorderSize = v; }

float ImGuiCLI::ImGuiStyle::ColumnsMinSpacing::get() { return ImGui::GetStyle().ColumnsMinSpacing; }
void ImGuiCLI::ImGuiStyle::ColumnsMinSpacing::set(float v) { ImGui::GetStyle().ColumnsMinSpacing = v; }

Vector2 ImGuiCLI::ImGuiStyle::TouchExtraPadding::get() 
{ 
    auto v = ImGui::GetStyle().TouchExtraPadding; 
    return Vector2(v.x, v.y);
}
void ImGuiCLI::ImGuiStyle::TouchExtraPadding::set(Vector2 v) { ImGui::GetStyle().TouchExtraPadding = ImVec2(v.X, v.Y); }

Color ImGuiCLI::ImGuiStyle::GetColor(ImGuiCol_ idx)
{
    ImVec4 v = ImGui::GetStyle().Colors[(int)idx];
    return Color(v.x, v.y, v.z, v.w);
}
void ImGuiCLI::ImGuiStyle::SetColor(ImGuiCol_ idx, Color value)
{
    auto v = value.ToVector4();
    ImGui::GetStyle().Colors[(int)idx] = ImVec4(v.X, v.Y, v.Z, v.W);
}