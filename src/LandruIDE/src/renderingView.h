
#include <imgui.h>

namespace lab
{
	class CursorManager;

class RenderingView
{
	bool ui_show_gbuffer = false;
	bool ui_more_stats = false;

	void show_statistics(const unsigned int frameRate);
	void draw_view_content(const ImVec2& size);

public:
	void render_scene(lab::CursorManager& cursorManager, ImVec2 area);
};

} // lab
