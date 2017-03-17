
#include "labGraphicsWindow.h"
#include "interface/labCursorManager.h"
#include "src/mainToolbar.h"
#include "interface/ImGuizmo.h"

#include <imgui.h>
#include <imgui_internal.h> // for ImGuiContext
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw_gl3.h>
#include <string>

namespace lab
{

	class FontManager;

	using namespace std;


	GraphicsWindow::GraphicsWindow(const std::string & window_name, int width, int height,
		std::shared_ptr<lab::CursorManager> cm,
		shared_ptr<lab::FontManager> fm)
			: _dockspace(this, cm, fm)
		, _font_manager(fm)
        {
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        #if __APPLE__
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        #endif
            _window = glfwCreateWindow(width, height, window_name.c_str(), NULL, NULL);
            glfwMakeContextCurrent(_window);

        #ifdef _WIN32
            // start GLEW extension handler
            glewExperimental = GL_TRUE;
            glewInit(); // create GLEW after the context has been created

                        // get version info
            const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
            const GLubyte* version = glGetString(GL_VERSION); // version as a string
            printf("Renderer: %s\n", renderer);
            printf("OpenGL version supported %s\n", version);
        #endif

            // Setup ImGui binding
            ImGui_ImplGlfwGL3_Init(_window, true);

            _context = ImGui::CreateContext();
        }

	GraphicsWindow::~GraphicsWindow()
	{
		if (_context)
			ImGui::DestroyContext(_context);
		if (_window)
			glfwDestroyWindow(_window);
	}

	void GraphicsWindow::frame_begin()
	{
		if (!_window)
			return;

		glfwMakeContextCurrent(_window);

		_activate_context();

		int w, h;
		glfwGetFramebufferSize(_window, &w, &h);

		auto& io = ImGui::GetIO();
		// Setup display size (every frame to accommodate for window resizing)
		io.DisplaySize = ImVec2(static_cast<float>(w), static_cast<float>(h));

		ImGui_ImplGlfwGL3_NewFrame(_window);

		ImGuizmo::BeginFrame();

		ImGui::SetNextWindowPos(ImVec2(0, 0));

		ImGui::SetNextWindowSize(ImVec2(static_cast<float>(w), static_cast<float>(h)));
		ImGuiWindowFlags flags =
									ImGuiWindowFlags_NoTitleBar
									| ImGuiWindowFlags_NoResize
									| ImGuiWindowFlags_NoMove
									| ImGuiWindowFlags_NoCollapse
									| ImGuiWindowFlags_NoBringToFrontOnFocus
									| ImGuiWindowFlags_NoFocusOnAppearing
									;

		ImGui::Begin("###workspace", 0, flags);
	}



	void GraphicsWindow::frame_end(lab::EditState & edit_state, GraphicsWindowManager & mgr)
	{
		if (!_window)
			return;

		_activate_context();

		ui(edit_state, mgr);

		ImGui::End(); // end the main window
		ImGui::Render();


		glfwSwapBuffers(_window);
	}

	void GraphicsWindow::close()
	{
		if (!_window)
			return;

		glfwDestroyWindow(_window);
		_window = nullptr;
	}

	bool GraphicsWindow::should_close() const
	{
		return !_window || glfwWindowShouldClose(_window);
	}

	void GraphicsWindow::request_focus()
	{
		if (!_window)
			return;

		glfwFocusWindow(_window);
	}

	void GraphicsWindow::get_frame_size(int & display_w, int & display_h)
	{
		if (!_window)
		{
			display_w = 0;
			display_h = 0;
			return;
		}

		_activate_context();
		glfwGetFramebufferSize(_window, &display_w, &display_h);
	}

	void GraphicsWindow::get_position(int & x, int & y)
	{
		if (!_window)
		{
			x = 0; y = 0;
			return;
		}

		glfwGetWindowPos(_window, &x, &y);
	}

	void GraphicsWindow::set_position(int x, int y)
	{
		if (!_window)
			return;

		glfwSetWindowPos(_window, x, y);
	}


	void GraphicsWindow::_activate_context()
	{
		ImGuiContext* prevContext = ImGui::GetCurrentContext();
		if (prevContext != nullptr && prevContext != _context)
		{
			std::memcpy(&_context->Style, &prevContext->Style, sizeof(ImGuiStyle));
			std::memcpy(&_context->IO.KeyMap, &prevContext->IO.KeyMap, sizeof(prevContext->IO.KeyMap));
			std::memcpy(&_context->MouseCursorData, &prevContext->MouseCursorData, sizeof(_context->MouseCursorData));
			_context->IO.IniFilename = prevContext->IO.IniFilename;
			_context->IO.RenderDrawListsFn = prevContext->IO.RenderDrawListsFn;
			_context->Initialized = prevContext->Initialized;
		}
		ImGui::SetCurrentContext(_context);
	}


	void GraphicsWindowManager::close_window(std::weak_ptr<GraphicsWindow> w)
	{
		auto window = w.lock();
		for (std::vector<std::shared_ptr<GraphicsWindow>>::iterator i = _windows.begin(); i != _windows.end(); ++i)
		{
			if (window.get() == (*i).get())
			{
				_windows.erase(i);
				break;
			}
		}
	}


	shared_ptr<GraphicsWindow> GraphicsWindowManager::find_dragged_window()
	{
		for (auto window : _windows)
		{
			auto& dockspace = window->get_dockspace();
			if (dockspace.node.splits[0] && dockspace.node.splits[0]->active_dock)
			{
				if (dockspace.node.splits[0]->active_dock->dragging)
					return window;
			}
		}

		return nullptr;
	}


	void GraphicsWindowManager::update_windows(lab::EditState& edit_state)
	{
		glfwWaitEventsTimeout(0.1f);

		ImVec4 clear_color = ImColor(1, 0, 0);

		vector < weak_ptr<GraphicsWindow>> windows;
		for (auto w : _windows)
			windows.push_back(w);

		ImGuiIO& io = ImGui::GetIO();

		for (auto wp : windows)
		{
			auto w = wp.lock();
			if (!w)
				continue;

			const float scale = 1.0f;
			io.DisplayFramebufferScale = { scale, scale };

			w->frame_begin();

			const float font_scale = 1.0f;
			ImGui::SetWindowFontScale(font_scale);

			// Rendering
			int display_w, display_h;
			w->get_frame_size(display_w, display_h);

			glViewport(0, 0, display_w, display_h);
			glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);

			w->frame_end(edit_state, *this);
		}
	}



} // lab
