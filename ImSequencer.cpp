#include "stdafx.h"
#include "ImSequencer.h"
#include "imgui.h"
#include "imgui_internal.h"

#include <vector>

#undef max
#undef min

namespace ImSequencer
{
    float GetTimeCurve()
    {
        return 0.5f + sinf(ImGui::GetTime() * 6.28318530718f * 1.3f) * 0.5f;
    }

    struct ScrollBarInfo
    {
        bool inTrack = false;
        bool inThumb = false;

        bool IsActive() { return inThumb || inTrack; }
        void Deactivate() { inThumb = inTrack = false; }
    };

    struct Context
    {
        bool inTrackHeader = false;
        ScrollBarInfo horizontalScroll;
        ScrollBarInfo verticalScroll;
        int mouseFrameIndex = -1;

        int movingTrack = -1;
        int movingKey = -1;
        int movingPos = -1;
        int movingPart = -1;
        int verticalOffset = 0;
    };
    static Context gContext;


	static bool SequencerAddDelButton(ImDrawList* draw_list, ImVec2 pos, bool add = true)
	{
		ImGuiIO& io = ImGui::GetIO();
		ImRect delRect(pos, ImVec2(pos.x + 16, pos.y + 16));
		bool overDel = delRect.Contains(io.MousePos);
        int delColor = overDel ? 0xFFAAAAAA : 0x50AAAAAA;
		//int delColor = overDel ? 0xFFAAAAAA : 0x50000000;
		float midy = pos.y + 16 / 2 - 0.5f;
		float midx = pos.x + 16 / 2 - 0.5f;
		draw_list->AddRect(delRect.Min, delRect.Max, delColor, 4);
		draw_list->AddLine(ImVec2(delRect.Min.x + 3, midy), ImVec2(delRect.Max.x - 3, midy), delColor, 2);
		if (add)
			draw_list->AddLine(ImVec2(midx, delRect.Min.y + 3), ImVec2(midx, delRect.Max.y - 3), delColor, 2);
		return overDel;
	}

	static int min(int a, int b) { return (a < b) ? a : b; }
	static int max(int a, int b) { return (a > b) ? a : b; }

	bool Sequencer(SequenceInterface *sequence, int *currentFrame, bool *expanded, int *selectedEntry, int* selectedKey, int *firstFrame, int sequenceOptions)
	{
		bool ret = false;
		ImGuiIO& io = ImGui::GetIO();
		int cx = (int)(io.MousePos.x);
		int cy = (int)(io.MousePos.y);
		int framePixelWidth = ImGui::GetFont()->FontSize * 0.8f;
		int legendWidth = 300;

        const float scrollX = ImGui::GetScrollX();
        const float scrollY = ImGui::GetScrollY();

		int delEntry = -1;
		int dupEntry = -1;
		int ItemHeight = ImGui::GetFont()->FontSize;

		bool popupOpened = false;
		ImGui::BeginGroup();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();
		ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
		ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
		static const int scrollBarHeight = 24;
		int firstFrameUsed = firstFrame ? *firstFrame : 0;

		int sequenceCount = sequence->GetTrackCount();
		int controlHeight = (sequenceCount + 1) * ItemHeight;
		int frameCount = sequence->GetFrameCount();
        
		if (expanded && !*expanded)
		{
			ImGui::InvisibleButton("canvas", ImVec2(canvas_size.x - canvas_pos.x, (float)ItemHeight));
			draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), 0xFF3D3837, 0);
			char tmps[512];
            int keyCt = 0;
            for (int i = 0; i < sequenceCount; ++i)
                keyCt += sequence->GetKeyFrameCount(i);
			sprintf_s(tmps, sizeof(tmps), "%d Frames / %d tracks / %d keyframes", frameCount, sequenceCount, keyCt);
			draw_list->AddText(ImVec2(canvas_pos.x + 26, canvas_pos.y), 0xFFFFFFFF, tmps);
		}
		else
		{
			bool hasHorizScrollBar = false;
            bool hasVerticalScrollBar = false;
			int framesPixelWidth = frameCount * framePixelWidth;
			if ((framesPixelWidth + legendWidth) >= canvas_size.x)
			{
                hasHorizScrollBar = true;
				controlHeight += scrollBarHeight;
			}
            if (controlHeight > canvas_size.y)
                hasVerticalScrollBar = true;

            const int visibleTrackCount = hasVerticalScrollBar ? floorf((canvas_size.y - (hasHorizScrollBar ? scrollBarHeight : 0) - ItemHeight) / ItemHeight) : sequenceCount;
            const int visibleFrameCount = (int)floorf((canvas_size.x - legendWidth) / framePixelWidth);
            const float effectiveHeight = (visibleTrackCount + 1) * ItemHeight;
            const bool isScrolling = gContext.verticalScroll.IsActive() || gContext.horizontalScroll.IsActive();

			ImRect backgroundRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + effectiveHeight));
			ImGui::InvisibleButton("canvas", ImVec2(canvas_size.x, effectiveHeight));

			// full background
			draw_list->AddRectFilled(backgroundRect.Min, backgroundRect.Max, 0xFF262222, 0);

			// current frame top
			ImRect topRect(ImVec2(canvas_pos.x + legendWidth, canvas_pos.y + scrollY), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + ItemHeight + scrollY));

            // Move the current time caret line
            if (!isScrolling && sequenceOptions & SEQUENCER_CHANGE_FRAME && currentFrame && *currentFrame >= 0 && (topRect.Contains(io.MousePos) || gContext.inTrackHeader) && io.MouseDown[0] && gContext.movingTrack == -1)
            {
                ImGui::CaptureMouseFromApp();
                *currentFrame = (int)(io.MousePos.x - topRect.Min.x) / framePixelWidth + firstFrameUsed;
                if (*currentFrame < 0)
                    *currentFrame = 0;
                if (*currentFrame >= frameCount)
                    *currentFrame = frameCount - 1;
                if (firstFrame && *currentFrame < *firstFrame)
                    *firstFrame = *currentFrame;
                if (firstFrame && *currentFrame > *firstFrame + visibleFrameCount - 1)
                    *firstFrame = *currentFrame - visibleFrameCount;
                
                // have to do this again??
                if (*currentFrame >= frameCount)
                    *currentFrame = frameCount - 1;

                gContext.inTrackHeader = true;
                // Text cursor resembles the time position
                ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);
            }
            else if (!io.MouseDown[0] && gContext.inTrackHeader)
                gContext.inTrackHeader = false;
			
            // Setup the mouse frame
            gContext.mouseFrameIndex = (int)(io.MousePos.x - (canvas_pos.x + legendWidth)) / framePixelWidth + firstFrameUsed;
            if (gContext.mouseFrameIndex < 0)
                gContext.mouseFrameIndex = 0;
            if (gContext.mouseFrameIndex >= frameCount)
                gContext.mouseFrameIndex = frameCount - 1;

			//header
			draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), 0xFF3D3837, 0);
			if (sequenceOptions&SEQUENCER_ADD)
			{
				if (SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + legendWidth - ItemHeight, canvas_pos.y + 2), true) && io.MouseReleased[0])
					ImGui::OpenPopup("addEntry");

				if (ImGui::BeginPopup("addEntry"))
				{
					for (int i = 0; i < sequence->GetTrackTypeCount(); i++)
						if (ImGui::Selectable(sequence->GetTrackTypeName(i)))
						{
							sequence->Add(i);
							*selectedEntry = sequence->GetTrackCount() - 1;
                            *selectedKey = -1;
						}

					ImGui::EndPopup();
					popupOpened = true;
				}
			}

            float curY = canvas_pos.y + ItemHeight;
			for (int i = gContext.verticalOffset; i < sequenceCount && i < gContext.verticalOffset + visibleTrackCount; i++)
			{
				int type;
				sequence->Get(i, -1, NULL, NULL, &type, NULL);
				ImVec2 tpos(canvas_pos.x + 3, curY + 2);
				draw_list->AddText(tpos, 0xFFFFFFFF, sequence->GetTrackLabel(i));
                curY += ItemHeight;
				if (sequenceOptions&SEQUENCER_DEL)
				{
					bool overDel = SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + legendWidth - ItemHeight + 2 - 10, tpos.y + 2), false);
					if (overDel && io.MouseReleased[0])
						delEntry = i;

					bool overDup = SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + legendWidth - ItemHeight - ItemHeight + 2 - 10, tpos.y + 2), true);
					if (overDup && io.MouseReleased[0])
						dupEntry = i;
				}
			}

			// clipping rect so items bars are not visible in the legend on the left when scrolled
            ImRect outerClip(ImVec2(canvas_pos.x + legendWidth, canvas_pos.y + scrollY), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + controlHeight + scrollY));
			draw_list->PushClipRect(outerClip.Min, outerClip.Max, true);
            ImRect innerClip = outerClip;
            innerClip.Max.x -= hasVerticalScrollBar ? (float)framePixelWidth : 0;
            innerClip.Max.y -= hasHorizScrollBar ? (float)scrollBarHeight : 0;
			
			// slots background
            curY = canvas_pos.y + ItemHeight;
			for (int i = gContext.verticalOffset; i < sequenceCount && i < gContext.verticalOffset + visibleTrackCount; i++)
			{
				unsigned int col = (i & 1) ? 0xFF3A3636 : 0xFF413D3D;

				ImVec2 pos = ImVec2(canvas_pos.x + legendWidth, curY + 1);
                curY += ItemHeight;
				ImVec2 sz = ImVec2(canvas_size.x + canvas_pos.x, pos.y + ItemHeight - 1);
				if (!popupOpened && cy >= pos.y && cy < pos.y + ItemHeight && gContext.movingTrack == -1 && cx>canvas_pos.x && cx < canvas_pos.x + canvas_size.x)
				{
					col += 0x80201008;
					pos.x -= legendWidth;
				}
				draw_list->AddRectFilled(pos, sz, col, 0);
			}

            // Timeline ticks and counts
            draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight), 0xFF3D3837, 0);
			for (int i = 0; i <= frameCount; i++)
			{
				bool baseIndex = ((i % 10) == 0) || (i == frameCount);
				bool halfIndex = (i % 5) == 0;
				int px = (int)canvas_pos.x + i * framePixelWidth + legendWidth - firstFrameUsed * framePixelWidth;
				int tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
				int tiretEnd = baseIndex ? effectiveHeight : ItemHeight;
				draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart), ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

				draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)ItemHeight), ImVec2((float)px, canvas_pos.y + (visibleTrackCount+1) * ItemHeight - 1), 0x30606060, 1);
				if (baseIndex)
				{
					char tmps[512];
                    sprintf_s(tmps, sizeof(tmps), "%d", i);// (i == frameCount) ? i : (i / 10));
					draw_list->AddText(ImVec2((float)px + 3.f, canvas_pos.y), 0xFFBBBBBB, tmps);
				}
			}
			draw_list->AddLine(canvas_pos, ImVec2(canvas_pos.x, canvas_pos.y + canvas_size.y), 0xFF000000, 1);
			draw_list->AddLine(ImVec2(canvas_pos.x, canvas_pos.y + ItemHeight), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + ItemHeight), 0xFF000000, 1);

			// selection
			bool selected = selectedEntry && (*selectedEntry >= 0);
			if (selected)
			{
				draw_list->AddRectFilled(ImVec2(canvas_pos.x, canvas_pos.y + ItemHeight * (*selectedEntry + 1)), ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + ItemHeight * (*selectedEntry + 2)), 0x301080FF, 1.f);
			}

			// slots
            curY = canvas_pos.y + ItemHeight;
            ImGui::PushClipRect(innerClip.Min, innerClip.Max, true);
			for (int trackIndex = gContext.verticalOffset; trackIndex < sequenceCount && trackIndex < gContext.verticalOffset + visibleTrackCount; ++trackIndex)
			{
                TRACK_NATURE nature = sequence->GetTrackNature(trackIndex);
                unsigned keyCt = sequence->GetKeyFrameCount(trackIndex);

                static auto Lerp = [](const ImVec4& lhs, const ImVec4& rhs, float td) {
                    return ImColor(ImVec4(
                        lhs.x + td * (rhs.x - lhs.x),
                        lhs.y + td * (rhs.y - lhs.y),
                        lhs.z + td * (rhs.z - lhs.z),
                        lhs.w + td * (rhs.w - lhs.w)));
                };

#define GLOW_ANIM(A, B) Lerp(ImColor(A), ImColor(B), GetTimeCurve())

                for (int keyIndex = 0; keyIndex < keyCt; ++keyIndex)
                {
                    int *start, *end;
                    unsigned int color;
                    sequence->Get(trackIndex, keyIndex, &start, &end, NULL, &color);

                    ImVec2 pos = ImVec2(canvas_pos.x + legendWidth - firstFrameUsed * framePixelWidth, curY + 1);
                    ImVec2 slotP1(pos.x + *start * framePixelWidth, pos.y + 2);
                    ImVec2 slotP2(pos.x + *end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2);

                    // when a tick always force to 1 keyframe wide
                    if (nature == TRACK_NATURE_TICK)
                        slotP2.x = slotP1.x + framePixelWidth;

                    unsigned int slotColor = color | 0xFF000000;
                    const unsigned int quadColor[] = { 0xFFFFFFFF, 0xFFFFFFFF, (ImU32)(ImColor(slotColor) * 1.5f) | 0xFF000000 };
                    bool drawSelected = selectedEntry && *selectedEntry == trackIndex && selectedKey && *selectedKey == keyIndex;
                    if (gContext.movingKey == keyIndex && gContext.movingTrack == trackIndex)
                        drawSelected = true;
                    if (nature == TRACK_NATURE_TICK)
                    {
                        ImVec2 pts[4] = {
                            ImVec2(slotP1.x, slotP1.y + (slotP2.y - slotP1.y) * 0.5f),
                            ImVec2(slotP1.x + (slotP2.x - slotP1.x) * 0.5f, slotP1.y),
                            ImVec2(slotP2.x, slotP1.y + (slotP2.y - slotP1.y) * 0.5f),
                            ImVec2(slotP1.x + (slotP2.x - slotP1.x) * 0.5f, slotP2.y),
                        };
                        draw_list->AddConvexPolyFilled(pts, 4, drawSelected ? GLOW_ANIM(slotColor, quadColor[2]) : slotColor);
                        draw_list->AddPolyline(pts, 4, drawSelected ? GLOW_ANIM(slotColor, quadColor[0]) : ImColor(slotColor) * 0.5f, true, 1.0f);
                    }
                    else
                        draw_list->AddRectFilled(slotP1, slotP2, drawSelected ? GLOW_ANIM(slotColor, quadColor[2]) : slotColor, 2);

                    ImRect mainRect = ImRect(slotP1, slotP2);
                    ImRect rects[3] = { ImRect(slotP1, ImVec2(slotP1.x + framePixelWidth / 2, slotP2.y))
                        , ImRect(ImVec2(slotP2.x - framePixelWidth / 2, slotP1.y), slotP2)
                        , ImRect(slotP1, slotP2) };

                    if (innerClip.Contains(io.MousePos) && gContext.movingTrack == -1 && (sequenceOptions & SEQUENCER_EDIT_STARTEND) && !gContext.inTrackHeader && !isScrolling)
                    {
                        if (nature == TRACK_NATURE_TICK)
                        {
                            if (mainRect.Contains(io.MousePos))
                            {
                                ImVec2 pts[4] = {
                                    ImVec2(slotP1.x, slotP1.y + (slotP2.y - slotP1.y) * 0.5f),
                                    ImVec2(slotP1.x + (slotP2.x - slotP1.x) * 0.5f, slotP1.y),
                                    ImVec2(slotP2.x, slotP1.y + (slotP2.y - slotP1.y) * 0.5f),
                                    ImVec2(slotP1.x + (slotP2.x - slotP1.x) * 0.5f, slotP2.y),
                                };
                                draw_list->AddConvexPolyFilled(pts, 4, GLOW_ANIM(slotColor, quadColor[2]));
                                draw_list->AddPolyline(pts, 4, GLOW_ANIM(slotColor, quadColor[0]), true, 1.0f);
                                if (io.MouseDown[0])
                                {
                                    gContext.movingTrack = trackIndex;
                                    gContext.movingKey = keyIndex;
                                    gContext.movingPos = cx;
                                    gContext.movingPart = 3;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            for (int j = 2; j >= 0; j--)
                            {
                                ImRect& rc = rects[j];
                                if (!rc.Contains(io.MousePos))
                                    continue;
                                draw_list->AddRectFilled(rc.Min, rc.Max, GLOW_ANIM(slotColor, quadColor[j]), 2);
                            }

                            for (int j = 0; j < 3; j++)
                            {
                                ImRect& rc = rects[j];
                                if (!rc.Contains(io.MousePos))
                                    continue;
                                if (io.MouseDown[0])
                                {
                                    gContext.movingTrack = trackIndex;
                                    gContext.movingKey = keyIndex;
                                    gContext.movingPos = cx;
                                    gContext.movingPart = j + 1;
                                    break;
                                }
                            }
                        }
                    }
                }
                curY += ItemHeight;
			}
            ImGui::PopClipRect();

			// moving
            if (gContext.movingTrack >= sequenceCount)
                gContext.movingTrack = -1;
			if (innerClip.Contains(io.MousePos) && gContext.movingTrack >= 0)
			{
                TRACK_NATURE nature = sequence->GetTrackNature(gContext.movingTrack);
				ImGui::CaptureMouseFromApp();

                // need to grab key starts for reordering keys
                unsigned keyCt = sequence->GetKeyFrameCount(gContext.movingTrack);
                std::vector<int> keyStarts;
                for (int keyIndex = 0; keyIndex < keyCt; ++keyIndex)
                {
                    int *start;
                    sequence->Get(gContext.movingTrack, keyIndex, &start, 0x0, 0x0, 0x0);
                    keyStarts.push_back(*start);
                }

				int diffFrame = (cx - gContext.movingPos) / framePixelWidth;
				if (abs(diffFrame) > 0)
				{
					int *start, *end;
					sequence->Get(gContext.movingTrack, gContext.movingKey, &start, &end, NULL, NULL);

					int & l = *start;
					int & r = *end;
                    if (nature == TRACK_NATURE_TICK)
                    {
                        l += diffFrame;
                        r += diffFrame;
                        l = max(min(l, frameCount), 0);
                        r = l + 1;
                    } 
                    else 
                    {

                        if (gContext.movingPart & 1)
                            l += diffFrame;
                        if (gContext.movingPart & 2)
                            r += diffFrame;
                        if (l < 0)
                        {
                            if (gContext.movingPart & 2)
                                r -= l;
                            l = 0;
                        }
                        if (gContext.movingPart & 1 && l > r)
                            l = r;
                        if (gContext.movingPart & 2 && r < l)
                            r = l;
                    }

                    // swap key orders
                    if (gContext.movingKey > 0)
                    {
                        if (keyStarts[gContext.movingKey - 1] > l)
                        {
                            if (sequence->SwapKeyframes(gContext.movingTrack, gContext.movingKey, gContext.movingKey - 1))
                                gContext.movingKey = gContext.movingKey - 1;
                        }
                    }
                    if (gContext.movingKey < keyCt - 1)
                    {
                        if (keyStarts[gContext.movingKey + 1] < l)
                        {
                            if (sequence->SwapKeyframes(gContext.movingTrack, gContext.movingKey, gContext.movingKey + 1))
                                gContext.movingKey = gContext.movingKey + 1;
                        }
                    }
                    while (l < *firstFrame)
                        *firstFrame -= 1;
                    while (l > *firstFrame + visibleFrameCount || r > *firstFrame + visibleFrameCount)
                        *firstFrame += 1;
                    gContext.movingPos += diffFrame * framePixelWidth;
				}
				if (!io.MouseDown[0])
				{
					// single select
					if (!diffFrame && gContext.movingPart && selectedEntry)
					{
						*selectedEntry = gContext.movingTrack;
                        *selectedKey = gContext.movingKey;
						ret = true;
					}

                    gContext.movingTrack = -1;
				}
                else if (gContext.movingTrack != -1)
                    ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			}

			// cursor
			if (currentFrame && *currentFrame >= 0)
			{
				float cursorOffset = canvas_pos.x + legendWidth + *currentFrame * framePixelWidth + framePixelWidth / 2 - (firstFrameUsed * framePixelWidth);
				draw_list->AddLine(ImVec2(cursorOffset, canvas_pos.y), ImVec2(cursorOffset, canvas_pos.y + effectiveHeight), 0x902A2AFF, 4);
			}
			draw_list->PopClipRect();
			
            // copy paste
			if (sequenceOptions & SEQUENCER_COPYPASTE)
			{
				ImRect rectCopy(ImVec2(canvas_pos.x + 100, canvas_pos.y + 2)
					, ImVec2(canvas_pos.x + 100 + 30, canvas_pos.y + ItemHeight - 2));
				bool inRectCopy = rectCopy.Contains(io.MousePos);
				unsigned int copyColor = inRectCopy ? 0xFF1080FF : 0xFF000000;
				draw_list->AddText(rectCopy.Min, copyColor, "Copy");

				ImRect rectPaste(ImVec2(canvas_pos.x + 140, canvas_pos.y + 2)
					, ImVec2(canvas_pos.x + 140 + 30, canvas_pos.y + ItemHeight - 2));
				bool inRectPaste = rectPaste.Contains(io.MousePos);
				unsigned int pasteColor = inRectPaste ? 0xFF1080FF : 0xFF000000;
				draw_list->AddText(rectPaste.Min, pasteColor, "Paste");

				if (inRectCopy && io.MouseReleased[0])
				{
					sequence->Copy();
				}
				if (inRectPaste && io.MouseReleased[0])
				{
					sequence->Paste();
				}
			}
			
            if (hasHorizScrollBar)
            {
                const float vertTake = hasVerticalScrollBar ? framePixelWidth*2 : framePixelWidth;
                int scrollBarStartHeight = canvas_size.y - scrollBarHeight;
                // ratio = number of frames visible in control / number to total frames
                int visibleFrameCount = (int)floorf((canvas_size.x - legendWidth) / framePixelWidth);
                float barWidthRatio = visibleFrameCount / (float)frameCount;
                float barWidthInPixels = barWidthRatio * (canvas_size.x - legendWidth - vertTake);

                const float xBase = canvas_pos.x + legendWidth;

                float startFrameOffset = ((float)firstFrameUsed / (float)frameCount) * (canvas_size.x - legendWidth);
                ImVec2 scrollBarA(xBase, canvas_pos.y + scrollBarStartHeight);
                ImVec2 scrollBarB(xBase + canvas_size.x - vertTake - legendWidth, canvas_pos.y + canvas_size.y);
                draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF222222, 0);

                ImRect scrollBarRect(scrollBarA, scrollBarB);
                bool inScrollBar = scrollBarRect.Contains(io.MousePos);

                ImVec2 scrollBarC(xBase + startFrameOffset, canvas_pos.y + scrollBarStartHeight + 2);
                ImVec2 scrollBarD(xBase + barWidthInPixels + startFrameOffset, canvas_pos.y + canvas_size.y - 2);
                bool inThumb = ImRect(scrollBarC, scrollBarD).Contains(io.MousePos);

                bool processScrollBar = (inThumb | inScrollBar | gContext.horizontalScroll.IsActive()) && gContext.movingTrack == -1;
                if (processScrollBar && io.MouseDown[0] && firstFrame && !(hasVerticalScrollBar && gContext.verticalScroll.IsActive()) && !gContext.inTrackHeader)
                {
                    if (inThumb || gContext.horizontalScroll.inThumb)
                    {
                        ImGui::CaptureMouseFromApp();
                        auto delta = ImGui::GetMouseDragDelta(0);
                        float ff = *firstFrame;
                        ff += delta.x / framePixelWidth;
                        int newFrame = roundf(ff);
                        if (newFrame != *firstFrame)
                        {
                            ImGui::ResetMouseDragDelta();
                            *firstFrame = newFrame;
                        }
                        gContext.horizontalScroll.inThumb = true;
                    }
                    else
                        *firstFrame = (int)(frameCount * ((io.MousePos.x - canvas_pos.x - legendWidth) / (canvas_size.x - legendWidth)));
                    *firstFrame = max(min(*firstFrame, frameCount - visibleFrameCount), 0);
                    gContext.horizontalScroll.inTrack = true;
                    ImGui::GetCurrentContext()->ActiveId = 1;
                }
                else if (!io.MouseDown[0] && gContext.horizontalScroll.IsActive())
                {
                    gContext.horizontalScroll.Deactivate();
                }
                
                draw_list->AddRectFilled(scrollBarC, scrollBarD, inScrollBar ? 0xFF606060 : 0xFF505050, 2);
            }
            else if (firstFrame)
            {
                *firstFrame = 0;
                gContext.horizontalScroll.Deactivate();
            }
            else
            {
                gContext.horizontalScroll.Deactivate();
            }

            if (hasVerticalScrollBar)
            {
                float scrollBarStartHeight = canvas_pos.y + ItemHeight;
                float scrollBarRight = canvas_pos.x + canvas_size.x;
                float scrollBarLeft = scrollBarRight - framePixelWidth;
                float scrollBarBottom = canvas_pos.y + canvas_size.y - (hasHorizScrollBar ? scrollBarHeight : 0);

                ImRect scrollRect({ scrollBarLeft, scrollBarStartHeight }, { scrollBarRight, scrollBarBottom });
                draw_list->AddRectFilled(scrollRect.Min, scrollRect.Max, 0xFF222222, 0);

                bool inTrack = scrollRect.Contains(io.MousePos);

                float scrollTop = gContext.verticalOffset * ItemHeight + 2;
                float contentSize = effectiveHeight / controlHeight;
                scrollTop = scrollTop * contentSize;
                contentSize = effectiveHeight * contentSize;
                
                ImRect thumbRect({ scrollBarLeft, scrollBarStartHeight + scrollTop }, { scrollBarRight, scrollBarStartHeight + contentSize + scrollTop - 4 });
                bool inThumb = thumbRect.Contains(io.MousePos);

                bool inScrollBar = (inThumb || inTrack || gContext.verticalScroll.IsActive()) && gContext.movingTrack == -1;
                if (inScrollBar && io.MouseDown[0] && !(hasHorizScrollBar && gContext.horizontalScroll.IsActive()) && !gContext.inTrackHeader)
                {
                    if (inThumb || gContext.verticalScroll.inThumb)
                    {
                        ImGui::CaptureMouseFromApp();
                        auto delta = ImGui::GetMouseDragDelta(0);
                        float ff = gContext.verticalOffset;
                        ff += delta.y / framePixelWidth;
                        int newOffset = roundf(ff);
                        if (newOffset != gContext.verticalOffset)
                        {
                            ImGui::ResetMouseDragDelta();
                            gContext.verticalOffset = newOffset;
                        }
                        gContext.verticalScroll.inThumb = true;
                    }
                    else
                        gContext.verticalOffset = (int)(sequenceCount * ((io.MousePos.y - canvas_pos.y - ItemHeight) / (canvas_size.y - ItemHeight)));

                    gContext.verticalScroll.inTrack = true;
                    ImGui::GetCurrentContext()->ActiveId = 1;
                }
                else if (!io.MouseDown[0])
                    gContext.verticalScroll.Deactivate();

                gContext.verticalOffset = max(min(gContext.verticalOffset, sequenceCount - visibleTrackCount), 0);
                draw_list->AddRectFilled(thumbRect.Min, thumbRect.Max, inScrollBar ? 0xFF606060 : 0xFF505050, 2);
            }
            else
            {
                gContext.verticalScroll.Deactivate();
                gContext.verticalOffset = 0;
            }
		}

		ImGui::EndGroup();

		if (expanded)
		{
			bool overExpanded = SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + 2, canvas_pos.y + 2), !*expanded);
			if (overExpanded && io.MouseReleased[0])
				*expanded = !*expanded;
		}

		if (delEntry != -1)
		{
			sequence->Del(delEntry);
            if (selectedEntry && (*selectedEntry == delEntry || *selectedEntry >= sequence->GetTrackCount()))
            {
                *selectedEntry = -1;
                *selectedKey = -1;
            }
		}

		if (dupEntry != -1)
		{
			sequence->Duplicate(dupEntry);
		}
		return ret;
	}
}
