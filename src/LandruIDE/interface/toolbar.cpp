

#include "toolbar.h"
#include "imguiRenderFrameEx.h"

using namespace ImGui;

// Toolbar adapted from EtherealEngine

namespace lab {

	bool BeginToolbar(const char* str_id, ImVec2 screen_pos, ImVec2 size)
	{
		ImGui::SetNextWindowPos(screen_pos);
		auto frame_padding = ImGui::GetStyle().FramePadding;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, frame_padding);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		float padding = frame_padding.y * 2;
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings;
		ImGui::SetNextWindowSize(size);
		bool ret = ImGui::Begin(str_id, nullptr, size, -1, flags);
		ImGui::PopStyleVar(3);

		return ret;
	}


	void EndToolbar()
	{
		auto height = ImGui::GetWindowHeight() + ImGui::GetStyle().FramePadding.y*2.0f;
		ImGui::End();
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + height);

	}

	bool ToolbarButton(ImTextureID texture, const char* tooltip, bool selected, bool enabled)
	{
		return ToolbarButtonEx(texture, ImVec2(24, 24), tooltip, selected, enabled);
	}

	bool ToolbarButtonEx(ImTextureID texture, ImVec2 size, const char* tooltip, bool selected, bool enabled)
	{
		ImVec4 bg_color(0, 0, 0, 0);
		// 		if (selected)
		// 		{
		// 			bg_color = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
		// 		}

		auto frame_padding = GetStyle().FramePadding;
		PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
		PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		PushStyleVar(ImGuiStyleVar_WindowPadding, frame_padding);
		PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

		bool ret = false;

		if (!enabled)
			Image(texture, size, ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
		else
		{
			if (ImageButton(texture, size, ImVec2(0, 0), ImVec2(1, 1), -1, bg_color))
			{
				ret = true;
			}
		}
		if (tooltip && IsItemHovered())
		{
			SetTooltip("%s", tooltip);
		}

		ImVec2 rectMin = GetItemRectMin();
		ImVec2 rectMax = GetItemRectMax();
		ImVec2 rectSize = GetItemRectSize();
		const float textHeight = GetTextLineHeight();

		if (selected)
		{
			PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
			RenderFrameEx(rectMin, rectMax, true, 0.0f, 2.0f);
			PopStyleColor();
		}

		PopStyleColor(3);
		PopStyleVar(3);
		return ret;
	}

	bool ToolbarTextButton(const char* text, const char* tooltip, bool selected, bool enabled)
	{
		ImVec4 bg_color(0, 0, 0, 0);
		// 		if (selected)
		// 		{
		// 			bg_color = ImGui::GetStyle().Colors[ImGuiCol_ButtonActive];
		// 		}

		auto frame_padding = GetStyle().FramePadding;
		PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0));
		PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0));
		PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
		PushStyleVar(ImGuiStyleVar_WindowPadding, frame_padding);
		PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

		bool ret = false;

		if (!enabled)
			Button(text);
		else
			ret = Button(text);

		if (tooltip && IsItemHovered())
		{
			SetTooltip("%s", tooltip);
		}

		ImVec2 rectMin = GetItemRectMin();
		ImVec2 rectMax = GetItemRectMax();
		ImVec2 rectSize = GetItemRectSize();
		const float textHeight = GetTextLineHeight();

		if (selected)
		{
			PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 0.6f, 0.0f, 1.0f));
			RenderFrameEx(rectMin, rectMax, true, 0.0f, 2.0f);
			PopStyleColor();
		}

		PopStyleColor(3);
		PopStyleVar(3);
		return ret;
	}


} // lab
