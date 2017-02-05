
#include "labImGuiWindow.h"


#include <imgui.h>
#include <imgui_internal.h> // for ImGuiContext
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw_gl3.h>
#include <string>

namespace lab
{

	using namespace std;

	vector<ImGuiWindow*> _windows;

	std::vector<ImGuiWindow*> & ImGuiWindow::windows()
	{
		return _windows;
	}


        ImGuiWindow::ImGuiWindow(const std::string & window_name, int width, int height)
			: _dockspace(this)
        {
			_windows.push_back(this);

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

        ImGuiWindow::~ImGuiWindow()
        {
			for (vector<ImGuiWindow*>::iterator i = _windows.begin(); i != _windows.end(); ++i)
			{
				if (*i == this)
				{
					_windows.erase(i);
					break;
				}
			}

            if (_context)
                ImGui::DestroyContext(_context);
			if (_window)
				glfwDestroyWindow(_window);
        }

        void ImGuiWindow::frame_begin()
        {
			if (!_window)
				return;

            _activate_context();
            glfwPollEvents();
            ImGui_ImplGlfwGL3_NewFrame();
        }

		static const int drag_button = 0;

		struct DragObject { 
			void * data; 
			string name;
			void unselect() {}
			void drop() {}
		};
		static DragObject * drag_object = nullptr;

        void ImGuiWindow::frame_end()
        {
			if (!_window)
				return;
			
			_activate_context();
        	_dockspace.update_and_draw(ImGui::GetContentRegionAvail());

			if (ImGui::IsMouseDragging(drag_button) && drag_object)
			{
				ImGui::SetTooltip(drag_object->name.c_str());

//				if (ImGui::GetMouseCursor() == ImGuiMouseCursor_Arrow)
//					ImGui::SetMouseCursor(ImGuiMouseCursor_NotAllowed);
			}

			if (!ImGui::IsAnyItemActive() && !ImGui::IsAnyItemHovered())
			{
/* for when there is imguizmo 
				if (ImGui::IsMouseDoubleClicked(0) && !imguizmo::is_over() && drag_object)
				{
					drag_object->unselect();
					drag_object->drop();
				}
				*/
			}
			if (ImGui::IsMouseReleased(drag_button))
			{
				drag_object->drop();
			}

			//ImGui::End();
			ImGui::Render();

			
			glfwSwapBuffers(_window);
        }

		void ImGuiWindow::close()
		{
			if (!_window)
				return;

			glfwDestroyWindow(_window);
			_window = nullptr;
		}

        bool ImGuiWindow::should_close() const
        {
            return !_window || glfwWindowShouldClose(_window);
        }

		void ImGuiWindow::request_focus()
		{
			if (!_window)
				return;

			glfwFocusWindow(_window);
		}

        void ImGuiWindow::get_frame_size(int & display_w, int & display_h)
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

		void ImGuiWindow::set_position(int x, int y)
		{
			if (!_window)
				return;

			glfwSetWindowPos(_window, x, y);
		}


        void ImGuiWindow::_activate_context()
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


} // lab
