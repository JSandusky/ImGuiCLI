#pragma once

#include "imgui.h"

namespace ImGui
{

    IMGUI_API bool RangeSliderFloat(const char* label, float* v1, float* v2, float v_min, float v_max, const char* display_format = "(%.3f, %.3f)", float power = 1.0f);

    IMGUI_API bool BitField(const char* label, unsigned* bits, unsigned* hoverIndex = 0x0);
    IMGUI_API bool DragFloatN_Colored(const char* label, float* v, int components, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* display_format = "%.3f", float power = 1.0f);
}