
#include "modes.h"
#include <imgui.h>

namespace lab
{
	class CursorManager;
	class FontManager;
	class RendererMode;
	class EditState;

class RenderingView : public MinorMode
{
	bool ui_more_stats = false;

	void show_statistics(lab::FontManager& fontManager, const unsigned int frameRate);
	void draw_view_content(lab::FontManager& fontManager, const ImVec2& size);

	std::shared_ptr<Mode> _rendererModePtr;
	RendererMode * _rendererMode = nullptr;

	int width = 0;
	int height = 0;

	ImVec2 previousMousePosition = { 0,0 };
	ImVec2 initialMousePosition = { 0,0 };

	void manipulation_gizmos(lab::EditState&);

	bool left_mouse = false;

public:
	RenderingView(ModeManager &);
	~RenderingView();

	virtual void ui(lab::EditState& edit_state,
		lab::CursorManager& cursorManager,
		lab::FontManager& fontManager,
		float width, float height) override;

	virtual void update(lab::GraphicsRootWindow&) override;

	virtual const char * name() const override { return "renderer";  }
};




} // lab
