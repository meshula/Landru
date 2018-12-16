
#pragma once
#include <imgui.h>

namespace lab {
	// Toolbar adapted from EtherealEngine
	bool BeginToolbar(const char* str_id, ImVec2 screen_pos, ImVec2 size);
	void EndToolbar();
	bool ToolbarButton(ImTextureID texture, const char* tooltip, bool selected = false, bool enabled = true);
	bool ToolbarTextButton(const char* text, const char* tooltip, bool selected = false, bool enabled = true);
	bool ToolbarButtonEx(ImTextureID texture, ImVec2 size = ImVec2(24, 24), const char* tooltip = nullptr, bool selected = false, bool enabled = true);
}
