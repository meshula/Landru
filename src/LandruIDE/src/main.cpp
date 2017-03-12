// ImGui - standalone example application for Glfw + OpenGL 3, using programmable pipeline
// If you are new to ImGui, see examples/README.txt and documentation at the top of imgui.cpp.

#include <imgui.h>
#include <imgui_impl_glfw_gl3.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include <iostream>

#include "landruConsole.h"
#include "interface/labGraphicsWindow.h"
#include "interface/imguidock.h"
#include "interface/labCursorManager.h"
#include "interface/labFontManager.h"
#include "editState.h"
#include "renderingView.h"
#include <LabAcme/LabAcme.h>

#include <pxr/base/tf/hashset.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/treeIterator.h>

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

constexpr auto build_name = "able";

void banner(const lab::fs::path & app_path, const lab::fs::path & resource_path)
{
	cout << "------------------------------------------------------------------" << endl;
	cout << " Lab {" << build_name << "}" << endl;
	cout << "   " << app_path << endl;
	cout << "   " << resource_path << endl;
	cout << "------------------------------------------------------------------" << endl;
}

// Gathers information about a layer used as a subLayer, including its
// position in the layerStack hierarchy.
class SubLayerInfo
{
public:
	SubLayerInfo(SdfLayerHandle sublayer, SdfLayerOffset offset, SdfLayerHandle containingLayer, int depth)
		: layer(sublayer), offset(offset), parentLayer(containingLayer), depth(depth) {}

	SdfLayerHandle layer;
	SdfLayerHandle parentLayer;
	SdfLayerOffset offset;
	int depth;

	string GetOffsetString()
	{
		auto o = offset.GetOffset();
		auto s = offset.GetScale();
		if (o == 0)
		{
			if (s == 1)
				return "";
			else
				return "(scale = " + to_string(s) + ")";
		}
		if (s == 1)
			return "(offset = " + to_string(o) + ")";

		return "(offset = " + to_string(o) + "; scale = " + to_string(s) + ")";
	}
};

static void _add_sub_layers(SdfLayerHandle layer, SdfLayerOffset & layerOffset,
	SdfLayerHandle parentLayer, vector<SubLayerInfo> & layers, int depth)
{
	const SdfLayerOffsetVector &offsets = layer->GetSubLayerOffsets();
	layers.push_back(SubLayerInfo(layer, layerOffset, parentLayer, depth));
	const vector<string> &sublayers = layer->GetSubLayerPaths();
	const SdfLayerOffsetVector &sublayerOffsets = layer->GetSubLayerOffsets();
	for (size_t i = 0, numSublayers = sublayers.size(); i<numSublayers; i++)
	{
		SdfLayerOffset offset;
		if (sublayerOffsets.size())
		{
			offset = sublayerOffsets[i];
		}
		auto sublayer = SdfLayer::FindRelativeToLayer(layer, sublayers[i]);
		if (sublayer)
		{
			_add_sub_layers(sublayer, offset, layer, layers, depth + 1);
		}
	}
}
	
	
void outline_state(UsdStageRefPtr stage)
{
	static vector<SubLayerInfo> layers;

	static std::once_flag once;
	auto layersPtr = &layers;
	std::call_once(once, [&stage, layersPtr]()
	{
		auto layer = stage->GetRootLayer();
		SdfLayerOffset layerOffset; // time scale and offset per layer
		SdfLayerHandle parentLayer;
		_add_sub_layers(layer, layerOffset, parentLayer, *layersPtr, 0);
	});

	if (ImGui::TreeNode("root"))
	{
		auto it = layers.begin();
		int pop_depth = 0;

		while (it != layers.end())
		{
			int depth = (*it).depth;
			bool open = ImGui::TreeNode((*it).layer->GetDisplayName().c_str());
			++it;
			if (open)
			{
				++pop_depth;

				if (it == layers.end())
					break;

				if ((*it).depth == depth)
				{
					ImGui::TreePop();
					--pop_depth;
				}
				else if ((*it).depth < depth)
					for (int i = (*it).depth; i < depth; ++i)
					{
						ImGui::TreePop();
						--pop_depth;
					}
			}
			else
			{
				// scan past children if level was closed
				while (it != layers.end() && (*it).depth > depth)
					++it;
			}
		}
		while (pop_depth)
		{
			ImGui::TreePop();
			--pop_depth;
		}

		ImGui::TreePop(); // root node
	}
}


int main(int, char** argv)
{
    // Setup window
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        return 1;

	lab::fs::path app_path = lab::application_executable_path(argv[0]);
	lab::fs::path resource_path = lab::application_resource_path(argv[0]);

	banner(app_path, resource_path);

	shared_ptr<lab::FontManager> fontMgr = make_shared<lab::FontManager>(resource_path);
	lab::GraphicsWindowManager windowMgr;

	const bool highDpi = true;
	int w = highDpi? 2000:1280;
	int h = highDpi? 1500:1024;

	weak_ptr<lab::GraphicsWindow> window = windowMgr.create_window("Landru IDE", w, h, fontMgr);

    // Load Fonts
    // (there is a default font, this is only if you want to change it. see extra_fonts/README.txt for more details)
    ImGuiIO& io = ImGui::GetIO();
	io.DisplayFramebufferScale = { 1.f, 1.f };
	io.FontGlobalScale = 1.f;

	set_style_colors();

	lab::CursorManager cursorMgr;
	lab::EditState editState;


	std::vector<std::unique_ptr<ImGuiDock::Dock>> _docks;
	_docks.emplace_back(std::make_unique<ImGuiDock::Dock>());
	_docks.emplace_back(std::make_unique<ImGuiDock::Dock>());
	_docks.emplace_back(std::make_unique<ImGuiDock::Dock>());
	_docks.emplace_back(std::make_unique<ImGuiDock::Dock>());
	_docks.emplace_back(std::make_unique<ImGuiDock::Dock>());

	auto& console_dock = _docks[0];
	auto& view_dock = _docks[1];
	auto& outliner_dock = _docks[2];
	auto& properties_dock = _docks[3];
	auto& timeline_dock = _docks[4];

	LandruConsole console(fontMgr);
	console_dock->initialize("Console", true, ImVec2(400, 200), [&console](ImVec2 area)
	{
		console.draw_contents();
	});

	lab::RenderingView renderingView;
	view_dock->initialize("View", true, ImVec2(), [&editState, &fontMgr, &cursorMgr, &renderingView](ImVec2 area) {
		renderingView.render_ui(editState, cursorMgr, *fontMgr.get(), area);
	});

	outliner_dock->initialize("Outliner", true, ImVec2(100, 100), [&editState](ImVec2 area)
	{
		outline_state(editState.stage());
	});

	properties_dock->initialize("Properties", true, ImVec2(100, 100), [&editState](ImVec2 area)
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

	timeline_dock->initialize("Timeline", true, ImVec2(100, 100), [](ImVec2 area)
	{
		ImGui::Text("this is the timeline");
	});

	{
		shared_ptr<lab::GraphicsWindow> w = window.lock();
		if (w)
		{
			auto& dockspace = w->get_dockspace();
			dockspace.dock(view_dock.get(), ImGuiDock::DockSlot::None, 300, true);

			dockspace.dock(console_dock.get(), ImGuiDock::DockSlot::Bottom, 300, true);
			dockspace.dock_with(timeline_dock.get(), console_dock.get(), ImGuiDock::DockSlot::Tab, 250, true);

			dockspace.dock_with(outliner_dock.get(), view_dock.get(), ImGuiDock::DockSlot::Left, 300, true);
			dockspace.dock_with(properties_dock.get(), view_dock.get(), ImGuiDock::DockSlot::Right, 300, true);
		}
	}


	windowMgr.update_windows(editState); // prime the pump
	renderingView.render_scene(); // width and height were recorded during UI rendering

	while (true)
	{
		cursorMgr.set_cursor(ImGui::GetMouseCursor());
		windowMgr.update_windows(editState); // prime the pump
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
