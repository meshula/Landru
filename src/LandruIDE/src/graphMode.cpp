

#include "graphMode.h"
#include "sample_graphNodeFactory.h"
#include "interface/graph/graphEditor.h"

#define IMGUI_DEFINE_PLACEMENT_NEW
#include "imgui_internal.h"

namespace lab
{

    class GraphMode::Detail
    {
    public:
        ImGui::NodeGraphEditor nge;
        std::shared_ptr<GraphNodeFactory> node_factory;

        Detail(std::shared_ptr<GraphNodeFactory> gnf)
        : node_factory(gnf) {}
    };

	GraphMode::GraphMode(const std::string & name, std::shared_ptr<GraphNodeFactory> gnf)
    : _detail(new Detail(gnf))
	, _name(name)
	{
	}

	GraphMode::~GraphMode()
	{
        delete _detail;
	}

    void GraphMode::ui(lab::EditState& edit_state,
        lab::CursorManager& cursorManager,
        lab::FontManager& fontManager,
        float width, float height)
    {
		if (_detail->nge.isInited())
		{
			auto nf = _detail->node_factory;
			auto & nge = _detail->nge;

			// This adds entries to the "add node" context menu
			// last 2 args can be used to add only a subset of nodes (or to sort their order inside the context menu)
			nge.registerNodeTypes(nf->node_type_names(), (int)nf->node_type_name_count(),
			                      nf->node_factory(), nullptr, -1);

			// restrict the count of output nodes to one
			auto limits = nf->node_limits();
			for (auto i : limits)
				nge.registerNodeTypeMaxAllowedInstances(i.first, i.second);

			nge.show_style_editor = false;
			nge.show_load_save_buttons = false;
		}

		auto size = ImGui::GetContentRegionAvail();
		auto pos = ImGui::GetCursorScreenPos();
		ImGui::SetNextWindowPos(pos);
		ImGui::SetNextWindowSize(size);

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_MenuBar;

		if (ImGui::Begin("graph editor", nullptr, size, -1, flags))
		{
			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu(name()))
				{
					if (ImGui::MenuItem("Close")) {}
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			_detail->nge.render();
			ImGui::End();
		}
	}

} // lab

