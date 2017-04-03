#pragma once

#include "modes.h"
#include "events/event.hpp"
#include <chrono>

namespace lab {

	class GraphNodeFactory;

	class GraphMode : public MinorMode
	{
        class Detail;
        Detail * _detail = nullptr;

	public:
		GraphMode(); // make a sample graph editor
		GraphMode(std::shared_ptr<GraphNodeFactory> gnf);
		virtual ~GraphMode();

		virtual const char * name() const override { return "graph"; }

		virtual void ui(lab::EditState& edit_state,
			lab::CursorManager& cursorManager,
			lab::FontManager& fontManager,
			float width, float height) override;
	};

} // lab
