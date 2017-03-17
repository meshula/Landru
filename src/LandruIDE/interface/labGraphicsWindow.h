
#pragma once


#include "imguidock.h"

struct ImGuiContext;
struct GLFWwindow;

#include <string>
#include <vector>
#include <memory>

namespace lab
{
	// This is an OpenGL capable window, initialized with ImGui, and
	// an ImGui dockspace.

	class EditState;
	class GraphicsWindowManager;
	class CursorManager;
	class FontManager;

    class GraphicsWindow
    {
	protected:
        ImGuiContext * _context = nullptr;
        GLFWwindow * _window = nullptr;

		std::shared_ptr<FontManager> _font_manager;

		friend class GraphicsWindowManager;

		GraphicsWindow(const std::string & window_name, int width, int height, 
			std::shared_ptr<lab::CursorManager> cm,
			std::shared_ptr<lab::FontManager>);

		void frame_begin();
		void frame_end(lab::EditState & edit_state, GraphicsWindowManager &);

	public:
		virtual ~GraphicsWindow(); // public to avoid needing a deleter friend

		void request_focus();
		void close();
        bool should_close() const;

		void get_frame_size(int & display_w, int & display_h);
		void get_position(int & x, int & y);
		void set_position(int x, int y);

    	ImGuiDock::Dockspace & get_dockspace() { return _dockspace; }

		virtual void ui(lab::EditState&, GraphicsWindowManager & mgr) {}

    protected:
    	ImGuiDock::Dockspace _dockspace;

        void _activate_context();
    };

	class GraphicsWindowManager
	{
		std::vector<std::shared_ptr<GraphicsWindow>> _windows;

	public:

		template <typename WindowType>
		std::weak_ptr<GraphicsWindow> create_window(
			const std::string & window_name,
			int width, int height,
			std::shared_ptr<lab::CursorManager> cm,
			std::shared_ptr<FontManager> fm)
		{
			std::shared_ptr<GraphicsWindow> w(new WindowType(window_name, width, height, cm, fm));
			_windows.push_back(w);
			return w;
		}

		void close_window(std::weak_ptr<GraphicsWindow> w);
		void update_windows(lab::EditState& edit_state);

		std::shared_ptr<GraphicsWindow> find_dragged_window();
	};


} // lab
