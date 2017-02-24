
#include <imgui.h>

namespace lab
{
	class CursorManager;
	class FontManager;
	class RenderEngine;

class RenderingView
{
	bool ui_show_gbuffer = false;
	bool ui_more_stats = false;

	void show_statistics(lab::FontManager& fontManager, const unsigned int frameRate);
	void draw_view_content(lab::FontManager& fontManager, const ImVec2& size);

	RenderEngine * _detail = nullptr;

	int width = 0;
	int height = 0;

	ImVec2 previousMousePosition = { 0,0 };
	ImVec2 initialMousePosition = { 0,0 };

	void handle_camera_movement();

	bool left_mouse = false;

public:
	RenderingView();
	~RenderingView();

	void render_ui(lab::CursorManager& cursorManager, lab::FontManager&, ImVec2 area);

	void render_scene();
};




} // lab
