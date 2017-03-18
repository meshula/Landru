
#pragma once

#include "interface/labGraphicsWindow.h"
#include "editState.h"
#include <string>

namespace lab
{
	class ModeManager;

	class AppWindow : public DockingWindow
	{
		class Detail;
		Detail * _detail = nullptr;

	public:
		AppWindow(
			std::shared_ptr<GraphicsRootWindow>,
			const std::string & window_name, int width, int height,
			lab::ModeManager & mm,
			std::shared_ptr<lab::CursorManager> cm,
			std::shared_ptr<lab::FontManager>);

		virtual ~AppWindow();

		virtual void ui(EditState&, GraphicsWindowManager & mgr) override;

		void render_scene();

		lab::EditState & editState();
	};

	extern event<void(const std::string&)> evt_set_major_mode;


} //lab
