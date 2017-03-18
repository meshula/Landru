
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

	class GraphicsRootWindow
	{
		GLFWwindow * _window = nullptr;

	public:
		GraphicsRootWindow();
		~GraphicsRootWindow();

		GLFWwindow * window() const { return _window;  }
	};

    class GraphicsWindow
    {
	protected:
        ImGuiContext * _context = nullptr;
        GLFWwindow * _window = nullptr;

		std::shared_ptr<FontManager> _font_manager;
		std::shared_ptr<GraphicsRootWindow> _root;

		friend class GraphicsWindowManager;
		void frame_begin();
		void frame_end(lab::EditState & edit_state, GraphicsWindowManager &);

	public:
		GraphicsWindow(std::shared_ptr<GraphicsRootWindow>, const std::string & window_name, int width, int height,
			std::shared_ptr<lab::CursorManager> cm,
			std::shared_ptr<lab::FontManager>);

		virtual ~GraphicsWindow(); // public to avoid needing a deleter friend

		void request_focus();
		void close();
        bool should_close() const;

		void get_frame_size(int & display_w, int & display_h);
		void get_position(int & x, int & y);
		void set_position(int x, int y);

		virtual void ui(lab::EditState&, GraphicsWindowManager & mgr) {}

    protected:

        void _activate_context();
    };

	class DockingWindow : public GraphicsWindow
	{
	public:
		DockingWindow(std::shared_ptr<GraphicsRootWindow>,
			const std::string & window_name, int width, int height,
			std::shared_ptr<lab::CursorManager> cm,
			std::shared_ptr<lab::FontManager> fm);

		virtual ~DockingWindow() {}

    	ImGuiDock::Dockspace & get_dockspace() { return _dockspace; }

	protected:
    	ImGuiDock::Dockspace _dockspace;
	};

	class GraphicsWindowManager
	{
		std::vector<std::shared_ptr<GraphicsWindow>> _windows;
		std::shared_ptr<GraphicsRootWindow> _root;

	public:

		GraphicsWindowManager();

		void add_window(std::shared_ptr<GraphicsWindow> w)
		{
			if (w)
				_windows.push_back(w);
		}

		std::shared_ptr<GraphicsRootWindow> graphics_root_window()
		{ 
			if (!_root)
				_root = std::make_shared<GraphicsRootWindow>();
			return _root; 
		}

		void close_window(std::weak_ptr<GraphicsWindow> w);
		void update_windows(lab::EditState& edit_state);

		std::shared_ptr<DockingWindow> find_dragged_window();
	};


} // lab
