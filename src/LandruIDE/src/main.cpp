// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>

#include "landruConsole.h"
#include "interface/labGraphicsWindow.h"
#include "interface/imguidock.h"
#include "interface/labCursorManager.h"

using namespace std;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;

	lab::GraphicsWindowManager windowMgr;
	weak_ptr<lab::GraphicsWindow> window = windowMgr.create_window("Landru IDE", 1280, 720);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGuiIO& io = ImGui::GetIO();
	io.DisplayFramebufferScale = { 2.f, 2.f };
	io.FontGlobalScale = 2.f;
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyClean.ttf", 13.0f);
    //io.Fonts->AddFontFromFileTTF("../../extra_fonts/ProggyTiny.ttf", 10.0f);
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());


	std::vector<std::unique_ptr<ImGuiDock::Dock>> _docks;
	_docks.emplace_back(std::make_unique<ImGuiDock::Dock>());
	_docks.emplace_back(std::make_unique<ImGuiDock::Dock>());

	LandruConsole console;

	auto& console_dock = _docks[0];
	auto& dummy_dock = _docks[1];

	console_dock->initialize("Console", true, ImVec2(400, 200), [&console](ImVec2 area) 
	{
		console.draw_contents();
	});

	dummy_dock->initialize("Dock1", true, ImVec2(), [](ImVec2 area) {
		ImGui::Text("Hello :)");
	});

	{
		shared_ptr<lab::GraphicsWindow> w = window.lock();
		if (w)
		{
			auto& dockspace = w->get_dockspace();
			dockspace.dock(dummy_dock.get(), ImGuiDock::DockSlot::None, 300, true);
			dockspace.dock(console_dock.get(), ImGuiDock::DockSlot::Bottom, 300, true);
		}
	}

	lab::CursorManager cursorMgr;

	while (true)
	{
		cursorMgr.set_cursor(ImGui::GetMouseCursor());

		auto main_window = window.lock();
		if (!main_window || main_window->should_close())
			break;

		windowMgr.update_windows();
	}

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}
