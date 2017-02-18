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
#include "interface/labFontManager.h"
#include "renderingView.h"

using namespace std;

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

// style from EtherealEngine, BSD license.

struct HSVSetup
{
	float col_main_hue = 145.0f / 255.0f;
	float col_main_sat = 255.0f / 255.0f;
	float col_main_val = 205.0f / 255.0f;

	float col_area_hue = 145.0f / 255.0f;
	float col_area_sat = 0.0f / 255.0f;
	float col_area_val = 65.0f / 255.0f;

	float col_back_hue = 145.0f / 255.0f;
	float col_back_sat = 0.0f / 255.0f;
	float col_back_val = 45.0f / 255.0f;

	float col_text_hue = 0.0f / 255.0f;
	float col_text_sat = 0.0f / 255.0f;
	float col_text_val = 255.0f / 255.0f;
	float frameRounding = 0.0f;
};


void set_style_colors()
{
	HSVSetup setup;

	ImVec4 col_text = ImColor::HSV(setup.col_text_hue, setup.col_text_sat, setup.col_text_val);
	ImVec4 col_main = ImColor::HSV(setup.col_main_hue, setup.col_main_sat, setup.col_main_val);
	ImVec4 col_back = ImColor::HSV(setup.col_back_hue, setup.col_back_sat, setup.col_back_val);
	ImVec4 col_area = ImColor::HSV(setup.col_area_hue, setup.col_area_sat, setup.col_area_val);
	float rounding = setup.frameRounding;

	ImGuiStyle& style = ImGui::GetStyle();
	style.FrameRounding = rounding;
	style.WindowRounding = rounding;
	style.Colors[ImGuiCol_Text] = ImVec4(col_text.x, col_text.y, col_text.z, 1.00f);
	style.Colors[ImGuiCol_TextDisabled] = ImVec4(col_text.x, col_text.y, col_text.z, 0.58f);
	style.Colors[ImGuiCol_WindowBg] = ImVec4(col_back.x, col_back.y, col_back.z, 1.00f);
	style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(col_area.x, col_area.y, col_area.z, 1.00f);
	style.Colors[ImGuiCol_PopupBg] = ImVec4(col_area.x, col_area.y, col_area.z, 1.00f);
	style.Colors[ImGuiCol_Border] = ImVec4(col_text.x, col_text.y, col_text.z, 0.30f);
	style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(col_back.x, col_back.y, col_back.z, 1.00f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.68f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_TitleBg] = ImVec4(col_main.x, col_main.y, col_main.z, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(col_main.x, col_main.y, col_main.z, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.0f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(col_area.x, col_area.y, col_area.z, 1.0f);
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(col_area.x, col_area.y, col_area.z, 1.00f);
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(col_main.x, col_main.y, col_main.z, 0.31f);
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.78f);
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_ComboBg] = ImVec4(col_area.x, col_area.y, col_area.z, 1.00f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(col_text.x, col_text.y, col_text.z, 0.80f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(col_main.x, col_main.y, col_main.z, 0.54f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_Button] = ImVec4(col_main.x, col_main.y, col_main.z, 0.44f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.86f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_Header] = ImVec4(col_main.x, col_main.y, col_main.z, 0.76f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.86f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_Column] = ImVec4(col_text.x, col_text.y, col_text.z, 0.32f);
	style.Colors[ImGuiCol_ColumnHovered] = ImVec4(col_text.x, col_text.y, col_text.z, 0.78f);
	style.Colors[ImGuiCol_ColumnActive] = ImVec4(col_text.x, col_text.y, col_text.z, 1.00f);
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4(col_main.x, col_main.y, col_main.z, 0.20f);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 0.78f);
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_CloseButton] = ImVec4(col_text.x, col_text.y, col_text.z, 0.16f);
	style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(col_text.x, col_text.y, col_text.z, 0.39f);
	style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(col_text.x, col_text.y, col_text.z, 1.00f);
	style.Colors[ImGuiCol_PlotLines] = ImVec4(col_text.x, col_text.y, col_text.z, 0.63f);
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4(col_text.x, col_text.y, col_text.z, 0.63f);
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(col_main.x, col_main.y, col_main.z, 1.00f);
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(col_main.x, col_main.y, col_main.z, 0.43f);
	style.Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.10f, 0.10f, 0.10f, 0.55f);
}


int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;

	shared_ptr<lab::FontManager> fontMgr = make_shared<lab::FontManager>();
	lab::GraphicsWindowManager windowMgr;
	weak_ptr<lab::GraphicsWindow> window = windowMgr.create_window("Landru IDE", 1280, 720, fontMgr);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGuiIO& io = ImGui::GetIO();
	io.DisplayFramebufferScale = { 1.f, 1.f };
	io.FontGlobalScale = 1.f;

	set_style_colors();

	lab::CursorManager cursorMgr;


	std::vector<std::unique_ptr<ImGuiDock::Dock>> _docks;
	_docks.emplace_back(std::make_unique<ImGuiDock::Dock>());
	_docks.emplace_back(std::make_unique<ImGuiDock::Dock>());
	_docks.emplace_back(std::make_unique<ImGuiDock::Dock>());
	_docks.emplace_back(std::make_unique<ImGuiDock::Dock>());

	auto& console_dock = _docks[0];
	auto& view_dock = _docks[1];
	auto& outliner_dock = _docks[2];
	auto& properties_dock = _docks[3];

	LandruConsole console(fontMgr);
	console_dock->initialize("Console", true, ImVec2(400, 200), [&console](ImVec2 area)
	{
		console.draw_contents();
	});

	lab::RenderingView renderingView;
	view_dock->initialize("View", true, ImVec2(), [&fontMgr, &cursorMgr, &renderingView](ImVec2 area) {
		renderingView.render_ui(cursorMgr, *fontMgr.get(), area);
	});

	outliner_dock->initialize("Outliner", true, ImVec2(100, 100), [](ImVec2 area)
	{
		if (ImGui::TreeNode("Scene"))
		{
			if (ImGui::TreeNode("Cameras"))
			{
				ImGui::Text("Interactive");
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("/"))
			{
				ImGui::Text("  ShaderBall");
				ImGui::Text("  Sky");
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
	});

	properties_dock->initialize("Properties", true, ImVec2(100, 100), [](ImVec2 area)
	{
		if (ImGui::TreeNode("ShaderBall"))
		{
			if (ImGui::TreeNode("Transform"))
			{
				ImGui::Text("Translate: 0 0 0");
				ImGui::Text("Rotate: 0 0 0");
				ImGui::Text("Scale: 1 1 1");
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Geometry"))
			{
				ImGui::TreePop();
			}
			if (ImGui::TreeNode("Shading"))
			{
				ImGui::TreePop();
			}
			ImGui::TreePop();
		}
	});

	{
		shared_ptr<lab::GraphicsWindow> w = window.lock();
		if (w)
		{
			auto& dockspace = w->get_dockspace();
			dockspace.dock(view_dock.get(), ImGuiDock::DockSlot::None, 300, true);
			dockspace.dock(console_dock.get(), ImGuiDock::DockSlot::Bottom, 300, true);
			dockspace.dock_with(outliner_dock.get(), view_dock.get(), ImGuiDock::DockSlot::Left, 300, true);
			dockspace.dock_with(properties_dock.get(), view_dock.get(), ImGuiDock::DockSlot::Right, 300, true);
		}
	}

	windowMgr.update_windows(); // prime the pump
	renderingView.render_scene(); // width and height were recorded during UI rendering

	while (true)
	{
		cursorMgr.set_cursor(ImGui::GetMouseCursor());
		windowMgr.update_windows(); // prime the pump
		renderingView.render_scene(); // width and height were recorded during UI rendering

		auto main_window = window.lock();
		if (!main_window || main_window->should_close())
			break;
	}

    // Cleanup
    ImGui_ImplGlfwGL3_Shutdown();
    glfwTerminate();

    return 0;
}
