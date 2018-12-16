
#include "outliner.h"
#include <imgui.h>
#include "interface/labFontManager.h"
#include "interface/materialIconDefinitions.h"
#include "interface/toolbar.h"
#include "editState.h"
#include <string>

#ifdef _MSC_VER
// suppress warnings until USD is cleaned up
#  pragma warning(push)
#  pragma warning(disable : 4244 4305)
#endif

#include <pxr/base/tf/hashset.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/treeIterator.h>

#ifdef _MSC_VER
#  pragma warning(pop)
#endif

using namespace lab;
using namespace std;

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

class Outliner::Detail
{
public:
	UsdStagePtr stage; // weak pointer
	vector<SubLayerInfo> layers;
};

Outliner::Outliner()
	: detail(new Detail())
{
	evt_stage_created.connect(this, &Outliner::stage_created);
	evt_layer_created.connect(this, &Outliner::layer_created);
}

Outliner::~Outliner()
{
	evt_stage_created.disconnect(this, &Outliner::stage_created);
	evt_layer_created.disconnect(this, &Outliner::layer_created);
	delete detail;
}

void Outliner::stage_created(UsdStageRefPtr new_stage)
{
	detail->stage = new_stage;
	detail->layers.clear();

	if (new_stage)
	{
		auto layer = new_stage->GetRootLayer();
		SdfLayerOffset layerOffset; // time scale and offset per layer
		SdfLayerHandle parentLayer;
		_add_sub_layers(layer, layerOffset, parentLayer, detail->layers, 0);
	}
}

void Outliner::layer_created(const std::string & s)
{
	if (!detail->stage)
		return;

	detail->layers.clear();
	auto layer = detail->stage->GetRootLayer();
	SdfLayerOffset layerOffset; // time scale and offset per layer
	SdfLayerHandle parentLayer;
	_add_sub_layers(layer, layerOffset, parentLayer, detail->layers, 0);
}


void Outliner::ui(EditState & edit_state, FontManager & fm)
{
	SetFont f(fm.mono_font);
	float width = ImGui::GetContentRegionAvailWidth();

	static bool new_layer_dialog = false;
	{
		SetFont icon(fm.icon_font);
		if (lab::ToolbarTextButton(" " ICON_MD_NOTE_ADD " ", "New Layer", false))
		{
			new_layer_dialog = true;
		}
		ImGui::SameLine(0.0f);
		if (lab::ToolbarTextButton(" " ICON_MD_LIBRARY_ADD " ", "New Prim", false))
		{
			// evt_new_prim
		}
		ImGui::SameLine(0.0f);
		if (lab::ToolbarTextButton(" " ICON_MD_FOLDER_OPEN " ", "Reference Layer", false))
		{
			// evt_reference_layer
		}
		ImGui::SameLine(0.0f);
		if (lab::ToolbarTextButton(" " ICON_MD_GPS_FIXED " ", "Set Active Layer", false))
		{
			//evt_set_edit_target
		}
		ImGui::SameLine(0.0f);
		if (lab::ToolbarTextButton(" " ICON_MD_CLEAR " ", "Remove Layers", false))
		{
			// evt_clear_selected_layers
		}
	}

	if (ImGui::TreeNode("stage"))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 2); // Increase spacing.

		auto it = detail->layers.begin();
		int pop_depth = 0;

		static set<string> selected_layers;
		static string target_layer;
		string node_clicked;

		while (it != detail->layers.end())
		{
			// Disable the default open on single-click behavior and pass in Selected flag according to our selection state.
			ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

			string name = (*it).layer->GetDisplayName();
			if (selected_layers.find(name) != selected_layers.end())
				node_flags |= ImGuiTreeNodeFlags_Selected;

			int depth = (*it).depth;
			if (name == target_layer)
				ImGui::Bullet();
			bool open = ImGui::TreeNodeEx((void*) hash<string>{}(name), node_flags, name.c_str());

			if (ImGui::IsItemClicked())
			{
				target_layer = name;
				node_clicked = name;
			}

			++it;
			if (open)
			{
				++pop_depth;

				if (it == detail->layers.end())
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
				while (it != detail->layers.end() && (*it).depth > depth)
					++it;
			}
		}

		if (node_clicked.length())
		{
			// Update selection state. Process outside of tree loop to avoid visual inconsistencies during the clicking-frame.
			if (ImGui::GetIO().KeyCtrl)
			{
				// toggle whether the node is selected
				auto it = selected_layers.find(node_clicked);
				if (it != selected_layers.end())
					selected_layers.erase(it);
				else
					selected_layers.insert(node_clicked);
			}
			else
			{
				selected_layers.clear();
				selected_layers.insert(node_clicked);
			}
		}

		while (pop_depth)
		{
			ImGui::TreePop();
			--pop_depth;
		}

		ImGui::PopStyleVar();

		ImGui::TreePop(); // root node
	}

	if (new_layer_dialog)
	{
		static char layer_name[256] = "dummy";
		bool new_layer = false;
		ImGui::OpenPopup("New Layer");
		if (ImGui::BeginPopupModal("New Layer", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::InputText("Path", layer_name, 256);
			ImGui::Separator();
			if (ImGui::Button("OK", ImVec2(120, 0))) 
			{ 
				ImGui::CloseCurrentPopup(); 
				new_layer_dialog = false;
				new_layer = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) 
			{ 
				ImGui::CloseCurrentPopup(); 
				new_layer_dialog = false;
			}
			ImGui::EndPopup();
		}

		if (!new_layer_dialog && new_layer)
			evt_new_layer(layer_name);
	}
}
