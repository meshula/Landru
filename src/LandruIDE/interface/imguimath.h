
#pragma once

namespace ImGui {
	ImVec2 operator+ (const ImVec2& a, const ImVec2& b) { return ImVec2(a.x + b.x, a.y + b.y); }
}
