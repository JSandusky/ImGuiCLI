#include "stdafx.h"
#include "imgui_ext.h"
#include "imgui_internal.h"
#include "imgui.h"

extern float SliderBehaviorCalcRatioFromValue(float v, float v_min, float v_max, float power, float linear_zero_pos);

namespace ImGui
{

    // ~80% common code with ImGui::SliderBehavior
    bool RangeSliderBehavior(const ImRect& frame_bb, ImGuiID id, float* v1, float* v2, float v_min, float v_max, float power, int decimal_precision, ImGuiSliderFlags flags)
    {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = GetCurrentWindow();
        const ImGuiStyle& style = g.Style;

        // Draw frame
        RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

        const bool is_non_linear = (power < 1.0f - 0.00001f) || (power > 1.0f + 0.00001f);
        const bool is_horizontal = (flags & ImGuiSliderFlags_Vertical) == 0;

        const float grab_padding = 2.0f;
        const float slider_sz = is_horizontal ? (frame_bb.GetWidth() - grab_padding * 2.0f) : (frame_bb.GetHeight() - grab_padding * 2.0f);
        float grab_sz;
        if (decimal_precision > 0)
            grab_sz = ImMin(style.GrabMinSize, slider_sz);
        else
            grab_sz = ImMin(ImMax(1.0f * (slider_sz / ((v_min < v_max ? v_max - v_min : v_min - v_max) + 1.0f)), style.GrabMinSize), slider_sz);  // Integer sliders, if possible have the grab size represent 1 unit
        const float slider_usable_sz = slider_sz - grab_sz;
        const float slider_usable_pos_min = (is_horizontal ? frame_bb.Min.x : frame_bb.Min.y) + grab_padding + grab_sz*0.5f;
        const float slider_usable_pos_max = (is_horizontal ? frame_bb.Max.x : frame_bb.Max.y) - grab_padding - grab_sz*0.5f;

        // For logarithmic sliders that cross over sign boundary we want the exponential increase to be symmetric around 0.0f
        float linear_zero_pos = 0.0f;   // 0.0->1.0f
        if (v_min * v_max < 0.0f)
        {
            // Different sign
            const float linear_dist_min_to_0 = powf(fabsf(0.0f - v_min), 1.0f / power);
            const float linear_dist_max_to_0 = powf(fabsf(v_max - 0.0f), 1.0f / power);
            linear_zero_pos = linear_dist_min_to_0 / (linear_dist_min_to_0 + linear_dist_max_to_0);
        }
        else
        {
            // Same sign
            linear_zero_pos = v_min < 0.0f ? 1.0f : 0.0f;
        }

        // Process clicking on the slider
        bool value_changed = false;
        if (g.ActiveId == id)
        {
            if (g.IO.MouseDown[0])
            {
                const float mouse_abs_pos = is_horizontal ? g.IO.MousePos.x : g.IO.MousePos.y;
                float clicked_t = (slider_usable_sz > 0.0f) ? ImClamp((mouse_abs_pos - slider_usable_pos_min) / slider_usable_sz, 0.0f, 1.0f) : 0.0f;
                if (!is_horizontal)
                    clicked_t = 1.0f - clicked_t;

                float new_value;
                if (is_non_linear)
                {
                    // Account for logarithmic scale on both sides of the zero
                    if (clicked_t < linear_zero_pos)
                    {
                        // Negative: rescale to the negative range before powering
                        float a = 1.0f - (clicked_t / linear_zero_pos);
                        a = powf(a, power);
                        new_value = ImLerp(ImMin(v_max, 0.0f), v_min, a);
                    }
                    else
                    {
                        // Positive: rescale to the positive range before powering
                        float a;
                        if (fabsf(linear_zero_pos - 1.0f) > 1.e-6f)
                            a = (clicked_t - linear_zero_pos) / (1.0f - linear_zero_pos);
                        else
                            a = clicked_t;
                        a = powf(a, power);
                        new_value = ImLerp(ImMax(v_min, 0.0f), v_max, a);
                    }
                }
                else
                {
                    // Linear slider
                    new_value = ImLerp(v_min, v_max, clicked_t);
                }

                // Round past decimal precision
                new_value = RoundScalar(new_value, decimal_precision);
                if (*v1 != new_value || *v2 != new_value)
                {
                    if (fabsf(*v1 - new_value) < fabsf(*v2 - new_value))
                    {
                        *v1 = new_value;
                    }
                    else
                    {
                        *v2 = new_value;
                    }
                    value_changed = true;
                }
            }
            else
            {
                ClearActiveID();
            }
        }

        // Calculate slider grab positioning
        float grab_t = SliderBehaviorCalcRatioFromValue(*v1, v_min, v_max, power, linear_zero_pos);

        // Draw
        if (!is_horizontal)
            grab_t = 1.0f - grab_t;
        float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
        ImRect grab_bb1;
        if (is_horizontal)
            grab_bb1 = ImRect(ImVec2(grab_pos - grab_sz*0.5f, frame_bb.Min.y + grab_padding), ImVec2(grab_pos + grab_sz*0.5f, frame_bb.Max.y - grab_padding));
        else
            grab_bb1 = ImRect(ImVec2(frame_bb.Min.x + grab_padding, grab_pos - grab_sz*0.5f), ImVec2(frame_bb.Max.x - grab_padding, grab_pos + grab_sz*0.5f));
        window->DrawList->AddRectFilled(grab_bb1.Min, grab_bb1.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);

        // Calculate slider grab positioning
        grab_t = SliderBehaviorCalcRatioFromValue(*v2, v_min, v_max, power, linear_zero_pos);

        // Draw
        if (!is_horizontal)
            grab_t = 1.0f - grab_t;
        grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
        ImRect grab_bb2;
        if (is_horizontal)
            grab_bb2 = ImRect(ImVec2(grab_pos - grab_sz*0.5f, frame_bb.Min.y + grab_padding), ImVec2(grab_pos + grab_sz*0.5f, frame_bb.Max.y - grab_padding));
        else
            grab_bb2 = ImRect(ImVec2(frame_bb.Min.x + grab_padding, grab_pos - grab_sz*0.5f), ImVec2(frame_bb.Max.x - grab_padding, grab_pos + grab_sz*0.5f));
        window->DrawList->AddRectFilled(grab_bb2.Min, grab_bb2.Max, GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);

        ImRect connector(grab_bb1.Min, grab_bb2.Max);
        connector.Min.x += grab_sz;
        connector.Min.y += grab_sz*0.3f;
        connector.Max.x -= grab_sz;
        connector.Max.y -= grab_sz*0.3f;

        window->DrawList->AddRectFilled(connector.Min, connector.Max, GetColorU32(ImGuiCol_SliderGrab), style.GrabRounding);

        return value_changed;
    }

    // ~95% common code with ImGui::SliderFloat
    bool RangeSliderFloat(const char* label, float* v1, float* v2, float v_min, float v_max, const char* display_format, float power)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;
        const ImGuiID id = window->GetID(label);
        const float w = CalcItemWidth();

        const ImVec2 label_size = CalcTextSize(label, NULL, true);
        const ImRect frame_bb(window->DC.CursorPos, 
            ImVec2(window->DC.CursorPos.x + w, window->DC.CursorPos.y + label_size.y + style.FramePadding.y*2.0f));
        const ImRect total_bb(frame_bb.Min, 
            ImVec2(frame_bb.Max.x + label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, frame_bb.Max.y));

        // NB- we don't call ItemSize() yet because we may turn into a text edit box below
        if (!ItemAdd(total_bb, id))
        {
            ItemSize(total_bb, style.FramePadding.y);
            return false;
        }

        const bool hovered = ItemHoverable(frame_bb, id);
        if (hovered)
            SetHoveredID(id);

        if (!display_format)
            display_format = "(%.3f, %.3f)";
        int decimal_precision = ParseFormatPrecision(display_format, 3);

        // Tabbing or CTRL-clicking on Slider turns it into an input box
        bool start_text_input = false;
        const bool tab_focus_requested = FocusableItemRegister(window, g.ActiveId == id);
        if (tab_focus_requested || (hovered && g.IO.MouseClicked[0]))
        {
            SetActiveID(id, window);
            FocusWindow(window);

            if (tab_focus_requested || g.IO.KeyCtrl)
            {
                start_text_input = true;
                g.ScalarAsInputTextId = 0;
            }
        }
        if (start_text_input || (g.ActiveId == id && g.ScalarAsInputTextId == id))
            return InputScalarAsWidgetReplacement(frame_bb, label, ImGuiDataType_Float, v1, id, decimal_precision);

        ItemSize(total_bb, style.FramePadding.y);

        // Actual slider behavior + render grab
        const bool value_changed = RangeSliderBehavior(frame_bb, id, v1, v2, v_min, v_max, power, decimal_precision, 0);

        // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
        char value_buf[64];
        const char* value_buf_end = value_buf + ImFormatString(value_buf, IM_ARRAYSIZE(value_buf), display_format, *v1, *v2);
        RenderTextClipped(frame_bb.Min, frame_bb.Max, value_buf, value_buf_end, NULL, ImVec2(0.5f, 0.5f));

        if (label_size.x > 0.0f)
            RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

        return value_changed;
    }

    bool BitField(const char* label, unsigned* bits, unsigned* hoverIndex)
    {
        unsigned val = *bits;

        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        unsigned oldFlags = window->Flags;
        ImGuiContext* g = ImGui::GetCurrentContext();
        const ImGuiStyle& style = ImGui::GetStyle();
        const ImGuiID id = window->GetID(label);
        const ImVec2 label_size = CalcTextSize(label, 0x0, true);
        const ImVec2 smallLabelSize = ImVec2(label_size.x * 0.5f, label_size.y * 0.5f);

        const float spacingUnit = 2.0f;

        bool anyPressed = false;
        ImVec2 currentPos = window->DC.CursorPos;
        for (unsigned i = 0; i < 32; ++i)
        {
            const void* lbl = (void*)(label + i);
            const ImGuiID localId = window->GetID(lbl);
            if (i == 16)
            {
                currentPos.x = window->DC.CursorPos.x;
                currentPos.y += smallLabelSize.y + style.FramePadding.y * 2 + spacingUnit /*little bit of space*/;
            }
            if (i == 8 || i == 24)
                currentPos.x += smallLabelSize.y;

            const ImRect check_bb(currentPos, { currentPos.x + smallLabelSize.y + style.FramePadding.y * 2, currentPos.y + smallLabelSize.y + style.FramePadding.y * 2 });

            bool hovered, held;
            bool pressed = ButtonBehavior(check_bb, localId, &hovered, &held, ImGuiButtonFlags_PressedOnClick);
            if (pressed && g->IO.KeyCtrl)
                *bits = (*bits == 0) ? -1 : 0;
            else if (pressed)
                *bits ^= (1 << i);

            if (hovered && hoverIndex)
                *hoverIndex = i;

            RenderFrame(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg));
            if (*bits & (1 << i))
            {
                const float check_sz = ImMin(check_bb.GetWidth(), check_bb.GetHeight());
                const float pad = ImMax(spacingUnit, (float)(int)(check_sz / 4.0f));
                window->DrawList->AddRectFilled(
                { check_bb.Min.x + pad, check_bb.Min.y + pad },
                { check_bb.Max.x - pad, check_bb.Max.y - pad }, GetColorU32(ImGuiCol_Text));
            }

            anyPressed |= pressed;
            currentPos.x = check_bb.Max.x + spacingUnit;
        }

        const ImRect matrix_bb(window->DC.CursorPos,
            { window->DC.CursorPos.x + (smallLabelSize.y + style.FramePadding.y * 2) * 16 /*# of checks in a row*/ + smallLabelSize.y /*space between sets of 8*/ + 15 * spacingUnit /*spacing between each check*/,
              window->DC.CursorPos.y + ((smallLabelSize.y + style.FramePadding.y * 2) * 2 /*# of rows*/ + spacingUnit /*spacing between rows*/) });

        ItemSize(matrix_bb, style.FramePadding.y);

        ImRect total_bb = matrix_bb;
        
        if (label_size.x > 0)
            SameLine(0, style.ItemInnerSpacing.x);

        const ImRect text_bb({ window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y }, { window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + style.FramePadding.y + label_size.y });
        if (label_size.x > 0)
        {
            ItemSize(ImVec2(text_bb.GetWidth(), matrix_bb.GetHeight()), style.FramePadding.y);
            total_bb = ImRect(ImMin(matrix_bb.Min, text_bb.Min), ImMax(matrix_bb.Max, text_bb.Max));
        }

        if (!ItemAdd(total_bb, id))
            return false;

        if (label_size.x > 0.0f)
            RenderText(text_bb.GetTL(), label);

        window->Flags = oldFlags;
        return anyPressed;
    }

    bool DragFloatN_Colored(const char* label, float* v, int components, float v_speed, float v_min, float v_max, const char* display_format, float power)
    {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext& g = *GImGui;
        bool value_changed = false;
        BeginGroup();
        PushID(label);
        PushMultiItemsWidths(components);
        for (int i = 0; i < components; i++)
        {
            static const ImU32 colors[] = {
                0xBB0000FF, // red
                0xBB00FF00, // green
                0xBBFF0000, // blue
                0xBBFFFFFF, // white for alpha?
            };

            PushID(i);
            value_changed |= DragFloat("##v", &v[i], v_speed, v_min, v_max, display_format, power);

            const ImVec2 min = GetItemRectMin();
            const ImVec2 max = GetItemRectMax();
            const float spacing = g.Style.ItemInnerSpacing.x;
            const float halfSpacing = spacing / 2;

            // This is the main change
            window->DrawList->AddRectFilled({ min.x, min.y + 1 }, { min.x + 8 , max.y - 1 }, colors[i], 1.0f, ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_TopLeft);
            //window->DrawList->AddLine({ min.x + spacing, max.y - halfSpacing }, { max.x - spacing, max.y - halfSpacing }, colors[i], 4);

            SameLine(0, g.Style.ItemInnerSpacing.x);
            PopID();
            PopItemWidth();
        }
        PopID();

        TextUnformatted(label, FindRenderedTextEnd(label));
        EndGroup();

        return value_changed;
    }

}