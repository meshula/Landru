
#pragma once

#include "interface/labGraphicsWindow.h"
#include "editState.h"
#include "renderingView.h"
#include "landruConsole.h"
#include <string>

namespace lab
{

	class AppWindow : public GraphicsWindow
	{
		class Detail;
		Detail * _detail = nullptr;

	public:
		AppWindow(const std::string & window_name, int width, int height, 
			std::shared_ptr<lab::CursorManager> cm,
			std::shared_ptr<lab::FontManager>);
		virtual ~AppWindow();

		virtual void ui(EditState&, GraphicsWindowManager & mgr) override;

		void render_scene();

		lab::EditState & editState();
	};

	extern event<void(const std::string&)> evt_set_major_mode;


} //lab
