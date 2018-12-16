#pragma once

#include "modes.h"
#include "interface/graph/grapheditor.h"


namespace lab {


    class GraphNodeFactory
    {
    public:
        virtual const char ** node_type_names() = 0;
        virtual size_t node_type_name_count() const = 0;

        virtual std::function <ImGui::Node * (int, const ImVec2&)> node_factory() = 0;

		// vector of pairs of node id + limit.
		// for example, if an output node is a singleton, the pair would be (node_id, 1)
		virtual std::vector<std::pair<int, int>> node_limits() { return std::vector<std::pair<int, int>>(); }
    };

	class GraphMode : public MinorMode
	{
        class Detail;
        Detail * _detail = nullptr;

		std::string _name;

	public:
		GraphMode(const std::string & name, std::shared_ptr<GraphNodeFactory> gnf);
		virtual ~GraphMode();

		virtual const char * name() const override { return _name.c_str(); }

		virtual void ui(lab::EditState& edit_state,
			lab::CursorManager& cursorManager,
			lab::FontManager& fontManager,
			float width, float height) override;
	};

} // lab
