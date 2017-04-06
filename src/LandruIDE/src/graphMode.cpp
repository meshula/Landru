

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

        void render()
        {
            if (nge.isInited())
            {
                // This adds entries to the "add node" context menu
                // last 2 args can be used to add only a subset of nodes (or to sort their order inside the context menu)
                nge.registerNodeTypes(node_factory->node_type_names(), (int) node_factory->node_type_name_count(),
                                      node_factory->node_factory(), nullptr, -1);

                // restrict the count of output nodes to one
				auto limits = node_factory->node_limits();
				for (auto i : limits)
	                nge.registerNodeTypeMaxAllowedInstances(i.first, i.second);

                nge.show_style_editor = false;
                nge.show_load_save_buttons = false;
                //--------------------------------------------------------------------------------
            }
            nge.render();
        }
    };

	GraphMode::GraphMode()
	{
		auto gnf = std::make_shared<Sample_GraphNodeFactory>();
		_detail = new Detail(gnf);
	}

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
		_detail->render();
	}

} // lab

