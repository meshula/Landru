
#pragma once

#include "editState.h"
#include "interface/labGraphicsWindow.h"

#include <map>
#include <string>

namespace lab
{

	class Mode
	{
		bool _suspended = false;
	public:
		virtual ~Mode() {}

		virtual const char * name() const = 0;

		virtual void update(lab::GraphicsRootWindow&) {}

		virtual void activate() {}
		virtual void suspend(bool s) { _suspended = s; }
		virtual bool suspended() const { return _suspended; }
		virtual void deactivate() {}
	};

	class MajorMode : public Mode
	{
	public:
		virtual ~MajorMode() {}
		virtual void ui(EditState & es, 
			GraphicsWindowManager & mgr, 
			ImGuiDock::Dockspace & dockspace) = 0;
	};

	class MinorMode : public Mode
	{
	public:
		virtual ~MinorMode() {}
		virtual bool is_singleton() const { return true; }

		virtual void ui(lab::EditState& edit_state,
			lab::CursorManager& cursorManager,
			lab::FontManager& fontManager,
			float width, float height) {}
	};

	class ModeManager
	{
		std::map < std::string, std::shared_ptr<MinorMode> > _minorModes;
		std::map < std::string, std::shared_ptr<MajorMode> > _majorModes;

	public:
		void add_mode(std::shared_ptr<MinorMode>);
		void add_mode(std::shared_ptr<MajorMode>);

		void update(lab::GraphicsRootWindow&);

		Mode * find_mode(const std::string &);

		template <typename T>
		T * find(const std::string & m)
		{
			Mode * m = find_mode(m);
			return dynamic_cast<T*>(m);
		}
	};

}
