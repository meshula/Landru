
#pragma once

#include "editState.h"
#include "interface/labGraphicsWindow.h"

#include <map>
#include <string>

namespace lab
{

	class MajorMode
	{
	public:
		virtual ~MajorMode() {}
		virtual void ui(EditState & es, GraphicsWindowManager & mgr, ImGuiDock::Dockspace & dockspace) = 0;
		virtual const char * name() const = 0;
	};

	class MinorMode
	{
	public:
		virtual ~MinorMode() {}
		virtual const char * name() const = 0;
		virtual bool is_singleton() const { return true; }

		virtual void ui(lab::EditState& edit_state,
			lab::CursorManager& cursorManager,
			lab::FontManager& fontManager,
			float width, float height) {}

		virtual void update(lab::GraphicsRootWindow&) {}
	};

	class ModeManager
	{
		std::map < std::string, std::shared_ptr<MinorMode> > _minorModes;
		std::map < std::string, std::shared_ptr<MajorMode> > _majorModes;

	public:
		void add_mode(std::shared_ptr<MinorMode>);
		void add_mode(std::shared_ptr<MajorMode>);

		void update(lab::GraphicsRootWindow&);
	};

}
