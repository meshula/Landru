
#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include "interface/imguimath.h"

namespace ImGui {
	void RenderFrameEx(ImVec2 p_min, ImVec2 p_max, bool border, float rounding, float thickness);

} // IMGUI_API
