
#pragma once

#include <string>

#include "imguidock.h"

struct ImGuiContext;
struct GLFWwindow;

namespace lab
{
    class ImGuiWindow
    {
        ImGuiContext * _context = nullptr;
        GLFWwindow * _window = nullptr;

    public:
        ImGuiWindow(const std::string & window_name, int width, int height);
        ~ImGuiWindow();

        void frame_begin();
        void frame_end();
		void close();
        bool should_close() const;
        void get_frame_size(int & display_w, int & display_h);
		void set_position(int x, int y);
		void request_focus();

    	ImGuiDock::Dockspace & get_dockspace() { return _dockspace; }

		static std::vector<ImGuiWindow*> & windows();

    private:
    	ImGuiDock::Dockspace _dockspace;

        void _activate_context();
    };

} // lab
