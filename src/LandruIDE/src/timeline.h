
#pragma once

#include "modes.h"

namespace lab {

	class Timeline : public MinorMode
	{
	public:
		virtual const char * name() const override { return "timeline"; }
		virtual void ui(lab::EditState& edit_state,
			lab::CursorManager& cursorManager,
			lab::FontManager& fontManager,
			float width, float height) override;
	};

} // lab
